@echo off
cd /d "%~dp0"
set DIST=dist\NovellEngine

echo === Building release exe ===
call build.bat
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo === Creating distribution ===
if exist dist rmdir /s /q dist
mkdir "%DIST%"
mkdir "%DIST%\assets"
mkdir "%DIST%\assets\Images"
mkdir "%DIST%\assets\Audio"

copy build\NovellEngine.exe "%DIST%\"
copy README.md "%DIST%\"
xcopy /e /i assets\Images "%DIST%\assets\Images" >nul

echo === Creating launcher script ===
echo @echo off > "%DIST%\run.bat"
echo cd /d "%%~dp0" >> "%DIST%\run.bat"
echo start /b NovellEngine.exe >> "%DIST%\run.bat"

echo === Creating ZIP ===
powershell -Command "Compress-Archive -Path '%DIST%\*' -DestinationPath 'dist\NovellEngine-v0.1.3.zip' -Force"
echo === Done: dist\NovellEngine-v0.1.3.zip ===
dir /s /b dist
