:: Postbuild script for Kino. Moves all the custom dll 
:: dependencies from /frameworks to /bin.

echo "---- Kino Postbuild Script START ----"

set roboArgs=/njs /njh /np /fp

:: OpenCV
robocopy "frameworks/opencv/dll/" "bin/" %roboArgs%

:: Darknet
robocopy "addons/ofxDarknet/libs/3rdparty/dll/x64/" "bin/" "pthreadVC2.dll" %roboArgs%

echo "----- Kino Postbuild Script END -----"