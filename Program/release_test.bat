@echo off

echo.
cd ..
if exist Source/Test/autorun.bin ( 
  echo Updating Release/Test
  cp --verbose --preserve Source/Test/autorun.bin Release/Test/
  rm Source/Test/autorun.bin
) else (
  echo Release/Test is already up-to-date
)
cd Program