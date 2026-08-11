@echo off
setlocal

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"

rem The project presets are intentionally hidden base presets.  Invoke the
rem visible local presets, which inherit the shared Windows x64 settings.
if /i "%CONFIG%"=="Debug" set "CONFIGURE_PRESET=windows-x64-local"
if /i "%CONFIG%"=="Debug" set "BUILD_PRESET=debug-local"
if /i "%CONFIG%"=="Editor" set "CONFIGURE_PRESET=windows-x64-local"
if /i "%CONFIG%"=="Editor" set "BUILD_PRESET=editor-local"
if /i "%CONFIG%"=="Release" set "CONFIGURE_PRESET=windows-x64-local"
if /i "%CONFIG%"=="Release" set "BUILD_PRESET=release-local"
if not defined BUILD_PRESET (
  echo [error] Unsupported configuration: %CONFIG%
  exit /b 1
)

set "CMAKE_COMMAND=cmake"
where cmake >nul 2>nul
if errorlevel 1 goto find_visual_studio_cmake
goto configure

:find_visual_studio_cmake
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
  for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
    set "VS_INSTALL_DIR=%%i"
  )
)
if defined VS_INSTALL_DIR (
  set "CMAKE_COMMAND=%VS_INSTALL_DIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)

if not exist "%CMAKE_COMMAND%" (
  echo [error] CMake was not found. Add cmake to PATH or install Visual Studio CMake tools.
  exit /b 1
)

:configure
"%CMAKE_COMMAND%" --preset %CONFIGURE_PRESET%
if errorlevel 1 exit /b %errorlevel%

"%CMAKE_COMMAND%" --build --preset %BUILD_PRESET%
if errorlevel 1 exit /b %errorlevel%

endlocal
