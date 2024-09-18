@ECHO OFF
PUSHD "%~dp0"
xcopy "src\rptoon.h" "bin\include\d3d9\" /z /i /y
POPD