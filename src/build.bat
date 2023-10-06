@echo off

if not exist ..\build mkdir ..\build

set CFLAGS=/nologo /Od /Zi /EHsc
set LIBS=User32.lib Xinput.lib Xaudio2.lib d3d11.lib d3dcompiler.lib
set SOURCES=..\src\main.cpp ..\imgui\backends\imgui_impl_dx11.cpp ..\imgui\backends\imgui_impl_win32.cpp ..\imgui\imgui*.cpp 
set INC_DIR=/I..\imgui /I..\imgui\backends /I..\stb_image
set LNK_DIR=

pushd ..\build

    REM cl %CFLAGS% %INC_DIR% %SOURCES% /Fe.\TMMapEditor /link %LNK_DIR% %LIBS% /SUBSYSTEM:CONSOLE
    cl %CFLAGS% %INC_DIR% %SOURCES% /Fe.\TMMapEditor /link %LNK_DIR% %LIBS% /SUBSYSTEM:WINDOWS

popd
