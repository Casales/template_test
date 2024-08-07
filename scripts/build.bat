set applicationName=template_test
set targetPlatform=nrf52dk_nrf52832
set applicationPath=.\build\%targetPlatform%\Release\zephyr\zephyr.hex
set artifactsPath=..\.gitlab\artifacts
set hexPath=%artifactsPath%\%applicationName%.hex
set apiFilePath=.\src\api.c

set originalPath=%cd%
cd %0\..\..\%applicationName%

if not exist %artifactsPath% mkdir %artifactsPath%

IF "%CI_COMMIT_TAG%"=="" set CI_COMMIT_TAG=0.0.0
set VersionNumber=%CI_COMMIT_TAG:.=,%
powershell -Command "(gc %apiFilePath%) -replace '\s*#define\s*SOFTWARE_VERSION\s*{\s*[0-9]*\s*,*\s*[0-9]*\s*,*\s*[0-9]*\s*}', '#define SOFTWARE_VERSION {%VersionNumber%}' | Out-File -encoding ASCII %apiFilePath%"

"C:\Program Files (x86)\Sysprogs\VisualGDB\VisualGDB.exe" /rebuild %applicationName%.vgdbproj /config:Release /platform:%targetPlatform%
if NOT %errorlevel% == 0 set exitCode=1

copy %applicationPath% %hexPath%
if NOT %errorlevel% == 0 set exitCode=1

cd %originalPath%
exit /B %exitCode%
