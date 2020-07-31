@ECHO off

SET HOST_PWD=%~dp0

ECHO ==== Build the Emscripten mobules with ninja
SET CONTAINER_NAME=dockcross_%RANDOM%
docker run -ti --name %CONTAINER_NAME% -v %HOST_PWD%:/work insighttoolkit/itk-js-base ninja -j8 -Cbuild
docker rm -f %CONTAINER_NAME%
del "%HOST_PWD%build\bin\TrialHeaderTest1.js"
copy "%HOST_PWD%build\bin\Test.js" "%HOST_PWD%host\static\Test.js"
ECHO      DONE

PAUSE