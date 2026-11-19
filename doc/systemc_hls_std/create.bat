
@echo off

if [%1]==[] goto usage

docker run ^
-v %CD%\books:/home/adf/books ^
-v %CD%\metadata:/home/adf/_data/works ^
-v %CD%\output:/home/adf/_output ^
adf:v0.9.1 /bin/bash -c "npm run eb -- output --format screen-pdf --book %*"
goto :eof

:usage
@echo Usage: %0 ^<book^>
exit /B 1
