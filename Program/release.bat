@echo off

echo.
echo ---This script built for Bash on Windows only---

echo.
call update_attiny.bat
call update_test.bat
call make_filesystem.bat