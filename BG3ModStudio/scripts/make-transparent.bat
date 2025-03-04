@echo off

REM This script is used to make the black background of the image transparent.
REM The script uses ImageMagick to process the image. Make sure ImageMagick is installed on your system.

REM Usage: make-transparent.bat <image>
REM <image> is the path to the image file to be processed.

if "%1"=="" goto usage

set "image=%1"
set "magic_path=C:\Program Files\ImageMagick-7.1.1-Q16-HDRI"
set "magic_exe=magick.exe"
set "magic_fullpath=%magic_path%\%magic_exe%"

REM Execute the ImageMagick command
"%magic_fullpath%" convert "%image%" -transparent black -define bmp:format=bgra "%image%-out.bmp" 2>nul

REM Check exit code
if %ERRORLEVEL% NEQ 0 (
    echo Process failed with exit code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
) else (
    echo Process succeeded.
    exit /b 0
)

:usage
echo Usage: make-transparent.bat ^<image^>
echo ^<image^> is the path to the image file to be processed.
exit /b 1
