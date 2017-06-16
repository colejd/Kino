:: Responsible for downloading and installing the addons needed 
:: for the program to compile.

@echo off

setlocal

echo Checking out addons...
set currentDir=%CD%
cd ../../../addons/
echo.

git clone --branch mrzl/windows --single-branch https://github.com/seieibob/ofxDarknet.git --depth 1 ./ofxDarknet-mrzl-windows
git clone https://github.com/armadillu/ofxFontStash.git --depth 1 ./ofxFontStash-master
git clone https://github.com/jvcleave/ofxImGui.git --depth 1 ./ofxImGui-master
git clone https://github.com/armadillu/ofxTimeMeasurements.git --depth 1 ./ofxTimeMeasurements-master
echo.

cd %currentDir%
echo Checkout completed!
echo.


:: Copy all the config files for darknet.
:: echo Copying files...
:: echo Copying completed!
:: echo.

echo This script will download a few assets to the addons folder, as well
echo as several data files needed for the program to run. These files are
echo fairly large! You'll need about 1.5 GB of free space.
echo.

SET /P AREYOUSURE="Continue? (Y/n) "
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END
echo.

call :DOWNLOAD "http://pjreddie.com/media/files/tiny-yolo.weights" "bin/data/data/darknet/tiny-yolo.weights"
call :DOWNLOAD "http://pjreddie.com/media/files/tiny-yolo-voc.weights" "bin/data/data/darknet/tiny-yolo-voc.weights"
echo.

echo Downloads completed!
echo.

pause&goto:eof

:END
endlocal
echo.&pause&goto:eof


:DOWNLOAD
if not exist %~2 (
	REM Download the file
	echo Downloading %~1...
	powershell -Command "Invoke-WebRequest %~1 -OutFile %~2"
	echo Download complete.
) else (
	echo %~1% already exists.
)
EXIT /B 0
