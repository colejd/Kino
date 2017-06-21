:: Responsible for downloading files that are too large to distribute with the git repo,
:: as well as fetching the submodules required by this repo.
@echo off

setlocal

:: Pull the submodules that we need to compile
echo Downloading submodules...
echo.

git submodule update --init --recursive
echo.

echo Submodules downloaded.
echo.

:: Pull the really large files used by ofxDarknet
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
