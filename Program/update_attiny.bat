@echo off

echo.
echo ---This script built for Bash on Windows only---

echo.
cd ..
if exist Source/PowerButton/BadgeWX_PowerT10/Release/BadgeWX_PowerT10.hex ( 
  echo Updating Release/Power
  cp --verbose --preserve Source/Power/BadgeWX_PowerT10/Release/BadgeWX_PowerT10.hex Release/Power/
  rm Source/Power/BadgeWX_PowerT10/Release/BadgeWX_PowerT10.hex
) else (
  echo Release/Power is already up-to-date
)
cd Program