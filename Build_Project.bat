@ECHO off

SET HOST_PWD=%~dp0

ECHO ==== Build the Emscripten mobules with ninja
SET CONTAINER_NAME=dockcross_%RANDOM%
docker run -ti --name %CONTAINER_NAME% -v %HOST_PWD%:/work insighttoolkit/itk-js-base ninja -j8 -Cbuild
docker rm -f %CONTAINER_NAME%
ECHO      DONE

PAUSE