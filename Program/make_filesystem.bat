@echo off

echo.
echo [Step 1 of 3]: Searching flashfw/ subfolder.
cd Flash_FS

echo.
echo [Step 2 of 3]: Generating temporary file listing: tempfiles.txt.
dir /b *.html *.png *.bin >../tempfiles.txt


echo.
echo [Step 3 of 3]: Building fsimage.bin.
echo.
echo Included files:
"../mkroffsimage" <../tempfiles.txt >../fsimage.bin


cd ..
echo.
echo Done!