@ECHO off

SET HOST_PWD=%~dp0

IF EXIST ".\build\" rmdir ".\build\" /S /Q
mkdir ".\build\"

ECHO ==== Ensure we have the 'dockcross' Docker build environment driver script
docker run --rm insighttoolkit/itk-js-base:latest > .\build\dockcross.sh
ECHO      DONE

ECHO ==== Perform initial CMake configuration
SET CONTAINER_NAME=dockcross_%RANDOM%
docker run -ti --name %CONTAINER_NAME% -v %HOST_PWD%:/work insighttoolkit/itk-js-base cmake -DRapidJSON_INCLUDE_DIR=/rapidjson/include -DCMAKE_BUILD_TYPE=Release -Bbuild -H. -GNinja -DITK_DIR=/ITK-build
docker rm -f %CONTAINER_NAME%
ECHO      DONE

PAUSE