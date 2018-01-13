@echo off

setlocal

set vswhere="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`%vswhere% -latest -property installationPath`) do (
  set InstallDir=%%i
)

set EXENAME=RodentIII
set PROF=%1

call "%InstallDir%\Common7\Tools\VsDevCmd.bat" -arch=x86

set POPCNTDEF=
set ENAME="%EXENAME%_x32_POPCNT.exe"
call :build

set POPCNTDEF=/D NO_MM_POPCNT
set ENAME="%EXENAME%_x32_noPOPCNT.exe"
call :build

call "%InstallDir%\Common7\Tools\VsDevCmd.bat" -arch=amd64

set POPCNTDEF=
set ENAME="%EXENAME%_x64_POPCNT.exe"
call :build

set POPCNTDEF=/D NO_MM_POPCNT
set ENAME="%EXENAME%_x64_noPOPCNT.exe"

:build

if "%PROF%"=="pgo" goto :doprofiling

call :buildhlp
del /q *.obj

goto :eof

:doprofiling

set ENAME=%ENAME:.exe=_pgo.exe%

call :buildhlp /GENPROFILE
del /q *.obj

echo(
echo Profiling...
echo bench | %ENAME% > nul

call :buildhlp /USEPROFILE
del /q *.obj
del /q *.pgd
del /q *.pgc

goto :eof

:buildhlp
echo(
echo -^> Building %ENAME% %1...
echo(

cl /O2 /GL /Gw /GS- /GR- /wd4577 /wd4530 /analyze- /MP /MT /Zc:inline /fp:fast /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_CRT_SECURE_NO_WARNINGS" /D "USE_THREADS" /D "USEGEN" %POPCNTDEF% src/*.cpp /link /SAFESEH:NO /LTCG /OPT:REF /OPT:ICF /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE /OUT:%ENAME% %1 | findstr /v /i ".cpp .obj"
