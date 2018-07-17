@echo off

echo.
echo ---This script built for Bash on Windows only---

echo.
cd ..
if exist Source/PowerButton/BadgeWX_PowerT10/Release/BadgeWX_PowerT10.hex ( 
  echo Updating Release/ATTiny
  cp --verbose --preserve Source/PowerButton/BadgeWX_PowerT10/Release/BadgeWX_PowerT10.hex Release/ATTiny/
  rm Source/PowerButton/BadgeWX_PowerT10/Release/BadgeWX_PowerT10.hex
) else (
  echo Release/ATTiny is already up-to-date
)
cd Program