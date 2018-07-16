Badge WX Firmware.

Folder Description
------------------
  <root>   - At the root of this folder is the firmware binaries and user file system binary for the BadgeWX plus batch and executables for generation and programming.
             May also may include user files provided on Wi-Fi Module products.
  Flash_FS - User files to be included on the Badge WX (inside the ESP module's flash filesystem).
  SD_Test  - Files for SD card to be used during Badge WX testing.

Programming The Badge WX's ESP Module
-------------------------------------
  1) Open a command-line window and navigate to this <root> folder.
  2) Connect a PropPlug to the appropriate pins on the BadgeWX (Gnd, Res, Tx, Rx).
  3) Ground the PGM pin.
  4) Power on the board.
  5) Execute the following (being sure to replace COM_PORT with the appropriate port name)
       ./esptool -cd none -cb 921600 -cp COM_PORT -bz 2M -bf 80 -bm qio -ca 0x00000 -cf boot_v1.6.bin -ca 0x01000 -cf httpd.user1.bin -ca 0x7e000 -cf blank.bin -ca 0x7f000 -cf blank.bin -ca 0x100000 -cf fsimage.bin -ca 0x3FC000 -cf esp_init_data_default.bin -ca 0x3fe000 -cf blank.bin -ca 0x3fa000 -cf blank.bin

Build the User Filesystem Image (fsimage.bin)
---------------------------------------------
  1) Place desired .html, .png, and .bin files into the Flash_FS subfolder.
  2) Open a command-line window and navigate to this <root> folder.
  3) Execute the following
       ./make_filesystem.bat
  4) The script will build a new fsimage.bin and replace the one in this <root> folder.
  5) Now you can program the Badge WX (above) again