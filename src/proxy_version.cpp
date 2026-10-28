// version.dll proxy. The game imports VERSION.dll, and Windows looks in the
// game folder before System32, so dropping this DLL there loads the mod with
// no third-party loader. Every export forwards to the real System32 DLL.
#include <windows.h>

namespace {

HMODULE g_real = nullptr;
INIT_ONCE g_once = INIT_ONCE_STATIC_INIT;

BOOL CALLBACK LoadReal(PINIT_ONCE, PVOID, PVOID*) {
    wchar_t path[MAX_PATH];
    const UINT n = GetSystemDirectoryW(path, MAX_PATH);
    if (n == 0 || n > MAX_PATH - 14) return TRUE;
    lstrcatW(path, L"\\version.dll");
    g_real = LoadLibraryW(path);
    return TRUE;
}

FARPROC Real(const char* name) {
    InitOnceExecuteOnce(&g_once, LoadReal, nullptr, nullptr);
    return g_real ? GetProcAddress(g_real, name) : nullptr;
}

template <typename Fn>
Fn Get(const char* name) {
    static_assert(sizeof(Fn) == sizeof(FARPROC));
    return reinterpret_cast<Fn>(Real(name));
}

} // namespace

#define FWD(ret, fail, name, params, args)                                  \
    extern "C" ret WINAPI Proxy_##name params {                              \
        using Fn = ret(WINAPI*) params;                                      \
        static Fn fn = Get<Fn>(#name);                                       \
        if (!fn) { SetLastError(ERROR_PROC_NOT_FOUND); return fail; }        \
        return fn args;                                                      \
    }

FWD(BOOL, FALSE, GetFileVersionInfoA, (LPCSTR a, DWORD b, DWORD c, LPVOID d), (a, b, c, d))
FWD(BOOL, FALSE, GetFileVersionInfoW, (LPCWSTR a, DWORD b, DWORD c, LPVOID d), (a, b, c, d))
FWD(BOOL, FALSE, GetFileVersionInfoExA, (DWORD f, LPCSTR a, DWORD b, DWORD c, LPVOID d), (f, a, b, c, d))
FWD(BOOL, FALSE, GetFileVersionInfoExW, (DWORD f, LPCWSTR a, DWORD b, DWORD c, LPVOID d), (f, a, b, c, d))
FWD(DWORD, 0, GetFileVersionInfoSizeA, (LPCSTR a, LPDWORD b), (a, b))
FWD(DWORD, 0, GetFileVersionInfoSizeW, (LPCWSTR a, LPDWORD b), (a, b))
FWD(DWORD, 0, GetFileVersionInfoSizeExA, (DWORD f, LPCSTR a, LPDWORD b), (f, a, b))
FWD(DWORD, 0, GetFileVersionInfoSizeExW, (DWORD f, LPCWSTR a, LPDWORD b), (f, a, b))
// Undocumented export; forwarded with pointer-sized args so nothing is truncated.
FWD(ULONG_PTR, 0, GetFileVersionInfoByHandle, (ULONG_PTR a, ULONG_PTR b, ULONG_PTR c, ULONG_PTR d), (a, b, c, d))
FWD(DWORD, 0, VerFindFileA, (DWORD a, LPCSTR b, LPCSTR c, LPCSTR d, LPSTR e, PUINT f, LPSTR g, PUINT h), (a, b, c, d, e, f, g, h))
FWD(DWORD, 0, VerFindFileW, (DWORD a, LPCWSTR b, LPCWSTR c, LPCWSTR d, LPWSTR e, PUINT f, LPWSTR g, PUINT h), (a, b, c, d, e, f, g, h))
FWD(DWORD, 0, VerInstallFileA, (DWORD a, LPCSTR b, LPCSTR c, LPCSTR d, LPCSTR e, LPCSTR f, LPSTR g, PUINT h), (a, b, c, d, e, f, g, h))
FWD(DWORD, 0, VerInstallFileW, (DWORD a, LPCWSTR b, LPCWSTR c, LPCWSTR d, LPCWSTR e, LPCWSTR f, LPWSTR g, PUINT h), (a, b, c, d, e, f, g, h))
FWD(DWORD, 0, VerLanguageNameA, (DWORD a, LPSTR b, DWORD c), (a, b, c))
FWD(DWORD, 0, VerLanguageNameW, (DWORD a, LPWSTR b, DWORD c), (a, b, c))
FWD(BOOL, FALSE, VerQueryValueA, (LPCVOID a, LPCSTR b, LPVOID* c, PUINT d), (a, b, c, d))
FWD(BOOL, FALSE, VerQueryValueW, (LPCVOID a, LPCWSTR b, LPVOID* c, PUINT d), (a, b, c, d))
