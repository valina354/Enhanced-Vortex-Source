@echo off
setlocal

rem ================================
rem ==== MOD PATH CONFIGURATIONS ===

rem == Set the absolute path to your mod's game directory here ==
set GAMEDIR=D:\STEAMG~1\STEAMA~1\SOURCE~2\HALF_L~1

rem == Set the relative or absolute path to Source SDK Base 2013 Singleplayer\bin ==
set SDKBINDIR=C:\PROGRA~2\Steam\STEAMA~1\common\SOURCE~1\bin

rem ==  Set the Path to your mod's root source code ==
rem This should already be correct, accepts relative paths only!
set SOURCEDIR=..\..

rem ==== MOD PATH CONFIGURATIONS END ===
rem ====================================


call buildsdkshaders.bat
call buildincludes.bat stdshader_dx9_20b     -game %GAMEDIR% -source %SOURCEDIR%
call buildincludes.bat stdshader_dx9_30        -game %GAMEDIR% -source %SOURCEDIR% -dx9_30 -force30

@echo Finished building shaders
@pause
