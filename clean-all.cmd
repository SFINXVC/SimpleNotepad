@echo off
echo Cleaning up...
del /q *.exe
del /q *.dll
rmdir /s /q build
rmdir /s /q .cache
echo Success!