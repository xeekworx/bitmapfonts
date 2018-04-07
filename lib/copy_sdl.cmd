@ECHO OFF
SETLOCAL
SET use_staticsdl=1

IF EXIST ..\third-party\sdl2\VisualC\Win32\Release ^
IF EXIST ..\third-party\sdl2\VisualC\x64\Release ^
IF EXIST ..\third-party\sdl2\VisualC\Win32\Debug ^
IF EXIST ..\third-party\sdl2\VisualC\x64\Debug ^
IF EXIST "..\third-party\sdl2\VisualC\Win32\Debug [Static]" ^
IF EXIST "..\third-party\sdl2\VisualC\x64\Debug [Static]" ^
IF EXIST "..\third-party\sdl2\VisualC\Win32\Release [Static]" ^
IF EXIST "..\third-party\sdl2\VisualC\x64\Release [Static]" (
 GOTO choice
) ELSE (
 ECHO You must build SDL before running this!
 PAUSE
 GOTO:EOF
)

:choice
set /P c=Do you want to use the static SDL libraries? (Y/N)
IF /I "%c%" EQU "Y" (
 SET use_staticsdl=1
 GOTO start
)
IF /I "%c%" EQU "N" (
 SET use_staticsdl=0
 GOTO start
)
goto :choice

:start
ECHO Creating /lib directory structure ...

if not exist x86\Debug mkdir x86\Debug
if not exist x86\Release mkdir x86\Release
if not exist x64\Debug mkdir x64\Debug
if not exist x64\Release mkdir x64\Release

IF %use_staticsdl%==1 (
 ECHO Copying SDL2 static libraries...
 xcopy "..\third-party\sdl2\VisualC\Win32\Debug [Static]\SDL2.lib" "x86\Debug\" /C /Q /Y
 xcopy "..\third-party\sdl2\VisualC\Win32\Release [Static]\SDL2.lib" "x86\Release\" /C /Q /Y
 xcopy "..\third-party\sdl2\VisualC\x64\Debug [Static]\SDL2.lib" "x64\Debug\" /C /Q /Y
 xcopy "..\third-party\sdl2\VisualC\x64\Release [Static]\SDL2.lib" "x64\Release\" /C /Q /Y
) ELSE (
 ECHO Copying SDL2 shared libraries and files ...
 
IF NOT EXIST ..\third-party\sdl2\VisualC\Win32\Release\SDL2.dll ^
IF NOT EXIST ..\third-party\sdl2\VisualC\x64\Release\SDL2.dll ^
IF NOT EXIST ..\third-party\sdl2\VisualC\Win32\Debug\SDL2.dll ^
IF NOT EXIST ..\third-party\sdl2\VisualC\x64\Debug\SDL2.dll (
 ECHO You need to build the shared libraries, they do not exist.
 PAUSE
 GOTO:EOF
)
 
 xcopy ..\third-party\sdl2\VisualC\Win32\Debug\SDL2.lib x86\Debug\ /C /Q /Y
 xcopy ..\third-party\sdl2\VisualC\Win32\Debug\SDL2.dll x86\Debug\ /C /Q /Y
 xcopy ..\third-party\sdl2\VisualC\Win32\Debug\SDL2.pdb x86\Debug\ /C /Q /Y

 xcopy ..\third-party\sdl2\VisualC\Win32\Release\SDL2.lib x86\Release\ /C /Q /Y
 xcopy ..\third-party\sdl2\VisualC\Win32\Release\SDL2.dll x86\Release\ /C /Q /Y

 xcopy ..\third-party\sdl2\VisualC\x64\Debug\SDL2.lib x64\Debug\ /C /Q /Y
 xcopy ..\third-party\sdl2\VisualC\x64\Debug\SDL2.dll x64\Debug\ /C /Q /Y
 xcopy ..\third-party\sdl2\VisualC\x64\Debug\SDL2.pdb x64\Debug\ /C /Q /Y

 xcopy ..\third-party\sdl2\VisualC\x64\Release\SDL2.lib x64\Release\ /C /Q /Y
 xcopy ..\third-party\sdl2\VisualC\x64\Release\SDL2.dll x64\Release\ /C /Q /Y
)

ECHO Copying SDL2main libraries ...

xcopy ..\third-party\sdl2\VisualC\Win32\Debug\SDL2main.lib x86\Debug\ /C /Q /Y
xcopy ..\third-party\sdl2\VisualC\Win32\Release\SDL2main.lib x86\Release\ /C /Q /Y
xcopy ..\third-party\sdl2\VisualC\x64\Debug\SDL2main.lib x64\Debug\ /C /Q /Y
xcopy ..\third-party\sdl2\VisualC\x64\Release\SDL2main.lib x64\Release\ /C /Q /Y

:end
ECHO Done.