@echo off
rem Builds release\TLOUInfiniteAmmo-v<version>.zip (Fluffy or manual install) from
rem build\version.dll, dist\TLOUInfiniteAmmo.ini and the files in package\fluffy.
rem Run build.bat first.
setlocal
set VERSION=1.0.0
cd /d "%~dp0"
if not exist build\version.dll (
  echo build\version.dll not found. Run build.bat first.
  exit /b 1
)
copy /y build\version.dll package\fluffy\ >nul || exit /b 1
copy /y dist\TLOUInfiniteAmmo.ini package\fluffy\ >nul || exit /b 1
if not exist release mkdir release
set ZIP=release\TLOUInfiniteAmmo-v%VERSION%.zip
if exist "%ZIP%" del "%ZIP%"
powershell -NoProfile -Command "Compress-Archive -Path package\fluffy\modinfo.ini,package\fluffy\screenshot.png,package\fluffy\version.dll,package\fluffy\TLOUInfiniteAmmo.ini,package\fluffy\TLOUInfiniteAmmo.log,package\fluffy\TLOUInfiniteAmmo-README.txt -DestinationPath '%ZIP%'" || exit /b 1
echo Built %ZIP%
