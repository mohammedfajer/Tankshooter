@echo off

REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

SET SDL_PATH=..\..\thirdparty\SDL2
SET SDL_TTF_PATH=..\..\thirdparty\SDL2_ttf
SET SDL_MIXER_PATH=..\..\thirdparty\SDL_mixer
SET SDL_IMAGE_PATH=..\..\thirdparty\SDL2_image
SET GLAD_PATH=..\..\thirdparty\glad
SET FREETYPE_PATH=..\..\thirdparty\freetype


IF NOT EXIST ..\build mkdir ..\build


REM Copy SDL2 DLLs to output directory
copy "%SDL_PATH%\lib\x64\SDL2.dll" "..\build"
@REM copy "%SDL_TTF_PATH%\lib\x64\SDL2_ttf.dll" "..\build"
copy "%SDL_MIXER_PATH%\lib\x64\SDL2_mixer.dll" "..\build"
copy "%SDL_IMAGE_PATH%\lib\x64\SDL2_image.dll" "..\build"
copy "%FREETYPE_PATH%\objs\x64\Debug\freetype.dll" "..\build"
copy "%FREETYPE_PATH%\objs\x64\Debug\freetype.lib" "..\build"

pushd ..\build

REM Compile the main.c file with the appropriate include and library paths
cl -FC -Zi /EHsc /std:c++20 ..\tankShooter\tankShooter.c  ^
    ..\tankShooter\engine\utils.c ..\tankShooter\engine\image.c ^
    ..\tankShooter\engine\sound.c ..\tankShooter\engine\graphics.c ^
    ..\tankShooter\engine\win32_fileIO.c ..\tankShooter\engine\gl_texture.c ^
    ..\tankShooter\engine\linkedlist.c ..\tankShooter\engine\init.c "%GLAD_PATH%\src\glad.c" ^
    /I"%SDL_PATH%\include" ^
    /I"%SDL_MIXER_PATH%\include" ^
    /I"%SDL_IMAGE_PATH%\include" ^
    /I"%GLAD_PATH%\include" ^
    /I"%FREETYPE_PATH%\include" ^
    /link ^
    /LIBPATH:"%SDL_PATH%\lib\x64" ^
    /LIBPATH:"%SDL_MIXER_PATH%\lib\x64" ^
    /LIBPATH:"%SDL_IMAGE_PATH%\lib\x64" ^
    "%SDL_PATH%\lib\x64\SDL2.lib" "%SDL_PATH%\lib\x64\SDL2main.lib" "%SDL_MIXER_PATH%\lib\x64\SDL2_mixer.lib" "%SDL_IMAGE_PATH%\lib\x64\SDL2_image.lib" "%FREETYPE_PATH%\objs\x64\Debug\freetype.lib" shell32.lib opengl32.lib ^
    /SUBSYSTEM:CONSOLE  
popd