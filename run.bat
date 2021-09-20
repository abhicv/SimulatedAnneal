@echo off
set exec=app.exe
pushd bin
if exist %exec% %exec%
popd bin

