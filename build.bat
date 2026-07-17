@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo Compiling Visual Novel Engine...

set CXX=g++
set CFLAGS=-std=c++17 -I"D:\SOOBSHESTVA\Novell_Engine\include"
set LFLAGS=-static -lmingw32 -lgdi32 -lgdiplus -lcomctl32 -lcomdlg32
set SRC_DIR=D:\SOOBSHESTVA\Novell_Engine
set BUILD_DIR=D:\SOOBSHESTVA\Novell_Engine\build
set OUT=%BUILD_DIR%\NovellEngine.exe

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

%CXX% -c "%SRC_DIR%\src\main.cpp" -o "%BUILD_DIR%\main.o" %CFLAGS% || exit /b 1
%CXX% -c "%SRC_DIR%\src\UI\EditorPanels.cpp" -o "%BUILD_DIR%\EditorPanels.o" %CFLAGS% || exit /b 1
%CXX% -c "%SRC_DIR%\src\Graphics\WinAPIGraphics.cpp" -o "%BUILD_DIR%\WinAPIGraphics.o" %CFLAGS% || exit /b 1
%CXX% -c "%SRC_DIR%\src\Core\ProjectManager.cpp" -o "%BUILD_DIR%\ProjectManager.o" %CFLAGS% || exit /b 1

%CXX% "%BUILD_DIR%\main.o" "%BUILD_DIR%\EditorPanels.o" "%BUILD_DIR%\WinAPIGraphics.o" "%BUILD_DIR%\ProjectManager.o" -o "%OUT%" %CFLAGS% %LFLAGS% || exit /b 1

echo Build successful!
start "" "%OUT%"
