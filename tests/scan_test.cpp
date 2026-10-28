// Offline test for the signature scanner. Maps a game executable from disk the
// same way Windows lays it out in memory and checks that the flags resolve,
// that they match the values verified by disassembly for known builds, and
// that a damaged image is rejected instead of yielding a bogus address.
//
// usage: scan_test.exe <path to tlou-i.exe> [<path to tlou-i-l.exe> ...]
#include "../src/scan.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

static int g_failures = 0;
#define CHECK(cond, ...)                                     \
    do {                                                     \
        if (cond) { std::printf("  PASS  "); }               \
        else { std::printf("  FAIL  "); ++g_failures; }      \
        std::printf(__VA_ARGS__);                            \
        std::printf("\n");                                   \
    } while (0)

static bool MapImage(const char* path, std::vector<uint8_t>& image, size_t& textEnd, uint32_t& stamp) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::vector<uint8_t> file((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(file.data());
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(file.data() + dos->e_lfanew);
    stamp = nt->FileHeader.TimeDateStamp;
    image.assign(nt->OptionalHeader.SizeOfImage, 0);
    std::memcpy(image.data(), file.data(), nt->OptionalHeader.SizeOfHeaders);
    auto* sec = IMAGE_FIRST_SECTION(nt);
    textEnd = 0;
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        const size_t n = (std::min)(sec[i].SizeOfRawData, sec[i].Misc.VirtualSize);
        std::memcpy(image.data() + sec[i].VirtualAddress, file.data() + sec[i].PointerToRawData, n);
        if (!textEnd && (sec[i].Characteristics & IMAGE_SCN_MEM_EXECUTE))
            textEnd = sec[i].VirtualAddress + sec[i].Misc.VirtualSize;
    }
    return true;
}

static void TestParser() {
    std::printf("parser\n");
    auto p = scan::Parse("32 C9 ?? 0d");
    CHECK(p.bytes.size() == 4 && p.bytes[0] == 0x32 && p.bytes[2] == -1 && p.bytes[3] == 0x0D, "parse hex and wildcards");
    const uint8_t buf[] = {0x00, 0x32, 0xC9, 0x55, 0x0D, 0x32, 0xC9, 0x00, 0x0D};
    auto hits = scan::FindAll(buf, sizeof(buf), p);
    CHECK(hits.size() == 2 && hits[0] == 1 && hits[1] == 5, "FindAll finds both wildcard matches");
    CHECK(scan::FindAll(buf, 3, p).empty(), "FindAll does not read past the buffer");
}

struct Known { uint32_t stamp; size_t infinite; size_t noReload; };
// Verified by disassembly of Steam build 18995818 (file version 1.1.4.0).
static const Known kKnown[] = {
    {0x685BEBE1, 0x362241D, 0x3622421},
};

static void TestImage(const char* path) {
    std::printf("image %s\n", path);
    std::vector<uint8_t> image;
    size_t textEnd = 0;
    uint32_t stamp = 0;
    if (!MapImage(path, image, textEnd, stamp)) {
        CHECK(false, "could not read file");
        return;
    }
    auto r = scan::LocateFlags(image.data(), textEnd, image.size());
    CHECK(r.ok, "flags located%s%s", r.ok ? "" : ": ", r.error.c_str());
    if (!r.ok) return;
    std::printf("        InfiniteAmmo RVA 0x%zX, NoReload RVA 0x%zX, timestamp 0x%08X\n", r.infiniteAmmo, r.noReload, stamp);
    CHECK(r.infiniteAmmo > textEnd && r.noReload > textEnd, "flags resolve outside the code section");
    for (const auto& k : kKnown) {
        if (k.stamp == stamp)
            CHECK(r.infiniteAmmo == k.infinite && r.noReload == k.noReload, "matches disassembly for this build");
    }
    CHECK(r.bottomless == r.infiniteAmmo + 1, "bottomless-reserve flag sits right after Infinite Ammo");
    CHECK(r.modifierOffset == 0x4968 && r.optionsPtr > textEnd, "options pointer resolved, modifier offset 0x%X", r.modifierOffset);

    // HUD rewrite: every site found, and each rewritten instruction must point
    // rip-relative at the Infinite Ammo flag with the original operand kept.
    std::string hudErr;
    auto sites = scan::LocateHudSites(image.data(), textEnd, hudErr);
    CHECK(sites.size() == 5, "all 5 HUD sites found%s%s", hudErr.empty() ? "" : ": ", hudErr.c_str());
    bool patchesOk = !sites.empty();
    for (const auto& s : sites) {
        auto p = scan::BuildHudPatch(s, r.infiniteAmmo);
        int32_t disp;
        std::memcpy(&disp, p.data() + 2, 4);
        const bool targetOk = p.size() == s.length && s.offset + s.length + disp == r.infiniteAmmo;
        const bool formOk = s.length == 7 ? (s.original[0] == 0x80 && p[0] == 0x80 && p[1] == 0x3D && p[6] == s.original[6])
                                          : (s.original[0] == 0x38 && p[0] == 0x38 && (p[1] & 0xC7) == 0x05 &&
                                             (p[1] & 0x38) == (s.original[1] & 0x38));
        const bool origOk = s.original[2] == 0x68 && s.original[3] == 0x49; // disp32 0x4968
        if (!(targetOk && formOk && origOk)) patchesOk = false;
    }
    CHECK(patchesOk, "HUD patches are same-length and target the Infinite Ammo flag");

    // Compares that the mod redirects to its private bytes; counts must match
    // what was verified by disassembly or the mod refuses to patch.
    const size_t inf = scan::FindByteCompares(image.data(), textEnd, r.infiniteAmmo).size();
    const size_t bot = scan::FindByteCompares(image.data(), textEnd, r.bottomless).size();
    CHECK(inf == 2 && bot == 5, "flag reads to redirect: InfiniteAmmo %zu/2, Bottomless %zu/5", inf, bot);

    // No Reload detour: the fire routine's magazine store, and a stub whose
    // relative operands land where they should when placed near the image.
    const auto c = scan::LocateClipStore(image.data(), textEnd);
    CHECK(c.ok, "magazine store found%s%s", c.ok ? "" : ": ", c.error.c_str());
    if (c.ok) {
        CHECK(c.isInfinite == r.infiniteCheck, "fire routine calls the same infinite-ammo check");
        CHECK(image[c.store] == 0x66 && image[c.store + 1] == 0x89 && image[c.store + 3] == 0x20 && image[c.store + 4] == 0x0C,
              "store is mov word ptr [rbx+0C20h], cx at RVA 0x%zX", c.store);
        const uint64_t base = 0x140000000ull, stub = base - 0x10000, flag = stub - 0x1000 + 2;
        const auto code = scan::BuildNoReloadStub(stub, base + c.store, flag, base + c.getOwner, base + c.asPlayer);
        const auto jump = scan::BuildDetourJump(base + c.store, stub);
        int32_t d;
        std::memcpy(&d, code.data() + 2, 4);
        uint64_t back, owner, player;
        std::memcpy(&back, code.data() + 76, 8);
        std::memcpy(&owner, code.data() + 84, 8);
        std::memcpy(&player, code.data() + 92, 8);
        int32_t j;
        std::memcpy(&j, jump.data() + 1, 4);
        CHECK(code.size() == 100 && stub + 7 + d == flag && back == base + c.store + 7 && owner == base + c.getOwner &&
                  player == base + c.asPlayer && std::memcmp(code.data() + 63, image.data() + c.store, 7) == 0,
              "stub checks the flag, re-runs the original store and jumps back after it");
        CHECK(jump.size() == 7 && jump[0] == 0xE9 && base + c.store + 5 + j == stub, "detour jump is 7 bytes and lands on the stub");
    }

    // Damage the cross-check instruction: the layout check must reject it.
    auto b = scan::FindAll(image.data(), textEnd, scan::Parse("40 38 3D ?? ?? ?? ?? 40 8A CF 74 02 B1 01 40 38 3D ?? ?? ?? ?? 75 0C 40 38 3D"));
    if (b.size() == 1) {
        auto copy = image;
        copy[b[0] + 26] ^= 0x40; // alter third disp32
        auto bad = scan::LocateFlags(copy.data(), textEnd, copy.size());
        CHECK(!bad.ok, "rejects image whose flag layout no longer matches (%s)", bad.error.c_str());
    }
    // Remove the main signature: must fail cleanly.
    {
        auto copy = image;
        auto a = scan::FindAll(copy.data(), textEnd, scan::Parse("32 C9 38 0D ?? ?? ?? ?? 75 0F 48 8B 05"));
        for (size_t off : a) copy[off] = 0xCC;
        auto bad = scan::LocateFlags(copy.data(), textEnd, copy.size());
        CHECK(!bad.ok, "fails safely when signature is missing (%s)", bad.error.c_str());
    }
}

int main(int argc, char** argv) {
    TestParser();
    for (int i = 1; i < argc; ++i) TestImage(argv[i]);
    std::printf("\n%s (%d failure%s)\n", g_failures ? "FAILED" : "ALL PASSED", g_failures, g_failures == 1 ? "" : "s");
    return g_failures ? 1 : 0;
}
