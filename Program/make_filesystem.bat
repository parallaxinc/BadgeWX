@echo off

echo.
echo ---This script built for Bash on Windows only---

echo.
echo [Step 1 of 5]: Clean Release/tempfiles (if exists)
cd ..
if exist "Release/tempfiles/*.*" (
  rm -f Release/tempfiles/*.*
)  


echo.
echo [Step 2 of 5]: Gather files into Release
rem Update Boot files if necessary
if exist Source/Boot/autorun/autorun.bin ( 
  cp --verbose --preserve Source/Boot/autorun/autorun.bin Release/Boot/
  rm Source/Boot/autorun/autorun.bin
)
if exist Source/Boot/autorunBL/cmm/autorunBL.bin ( 
  cp --verbose --preserve Source/Boot/autorunBL/cmm/autorunBL.bin Release/Boot/
  rm Source/Boot/autorunBL/cmm/autorunBL.bin
)
if exist Source/Boot/autorunSD/autorunSD.bin ( 
  cp --verbose --preserve Source/Boot/autorunSD/autorunSD.bin Release/Boot/
  rm Source/Boot/autorunSD/autorunSD.bin
)
rem Copy files to temporary space
if not exist Release/tempfiles (
  mkdir "Release/tempfiles/"
)  
if exist Source/Examples/*.html (
  cp --verbose --preserve Source/Examples/*.html Release/tempfiles/
) 
if exist Source/Examples/*.png ( 
  cp --verbose --preserve Source/Examples/*.png Release/tempfiles/
)  
cp --verbose --preserve Release/Boot/autorun.bin Release/tempfiles/
cp --verbose --preserve Release/Boot/autorunBL.bin Release/tempfiles/
cp --verbose --preserve Release/Boot/autorunSD.bin Release/tempfiles/


echo.
echo [Step 3 of 5]: Generating file listing: Release/tempfiles.txt.
cd Release/tempfiles
dir /b *.html *.png *.bin >../tempfiles.txt


echo.
echo [Step 4 of 5]: Building Release/Core/fsimage.bin
echo.
echo Included files:
"../../Program/mkroffsimage" <../tempfiles.txt >../Core/fsimage.bin
cd ../../


echo.
echo [Step 5 of 5]: Clean up
if exist Release/tempfiles/*.* (
  rm -R Release/tempfiles/*.*
)  
if exist Release/tempfiles (
  rmdir "Release/tempfiles/"
)  
if exist Release/tempfiles.txt (
  rm Release/tempfiles.txt
)  

echo.
echo Done!