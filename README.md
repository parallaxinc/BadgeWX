# BadgeWX
BadgeWX Firmware and Test Code

## Binary Assets
Binary assets are included and managed by Git LFS.  See "_Binary_Assets_Are_GitLFS_Managed-README.txt" for more details.  Most are in the Releases subfolder and are important since
there are multiple sources and multiple binaries that work together to make a complete product release.

## Folder Structure
 - Program - Scripts/tools used in generating the firmware/binary image files
 - Release - Generated firmware/binary image files
   - Boot  - BadgeWX boot system "autorun" files from generated within Source/Boot/...
   - Core  - BadgeWX firmware and user filesystem (for built-in ESP8266-based Wi-Fi module)
   - Power - ATTiny microcontroller binary image (soft-power button controller)
   - Test  - Special boot + configuration for testing BadgeWX; intended for SD-card use
 - Source
   - Boot     - Autorun files' source code
   - Examples - Default source examples (.html, .png), if any, for user filesystem
   - Power    - ATTiny soft-power button source code
   - Test     - Test application source code (autorun.bin) and Wi-Fi credentials (wifi.txt) for SD card
   
## Setup A New Development Machine
The following needs to be installed on the development computer before proceeding with development.  Most of this software supports Windows only, so a Windows computer will be assumed.
  - [Propeller Tool](https://www.parallax.com/downloads/propeller-tool-software-windows-spin-assembly)
  - [Simple IDE](https://www.parallax.com/downloads/simpleide-software-windows-propeller-c)
  - [Atmel Studio](http://www.microchip.com/mplab/avr-support/atmel-studio-7)
  - [Python v3.6.5](https://www.python.org/downloads/release/python-365/)
  - Esptool - After Python install, open command prompt and enter: python -m pip install esptool
    - If needed, here are [more details](https://github.com/espressif/esptool) about Esptool for Python.
   
## Building/Updating the Release
The Release consists of many Build/Release parts; perform any or all of the build steps and/or release steps (below) as necessary.

### Build
#### Boot firmware
  - autorun.bin - Compile Source/Boot/autorun/autorun.spin with Propeller Tool and save as autorun.bin (in same folder)
  - autorunBL.bin - Compile Source/Boot/autorun/BadgeWX-Bootloader.side (c project) with SimpleIDE
    - Ensure it saved a .binary of the image (should be in cmm subfolder) and rename it to autorunBL.bin
  - autorunSD.bin - Compile Source/Boot/autorun/autorunSD.spin with Propeller Tool and save as autorunSD.bin (in same folder)
  - Examples - If any examples are to be included in the user filesystem (.html, .gif, .jpg, .png), place them in the Source/Examples folder
  - Follow Release instructions (below)

#### Core Firmware   
  - "Make" the Wi-Fi firmware from the [Parallax-ESP repository](https://github.com/parallaxinc/Parallax-ESP)
```
  $ cd <Parallax-ESP_folder>
  $ make clean
  $ make EXTRA_CFLAGS=-DWIFI_BADGE
```
  - Follow Release instructions (below)

#### Power (ATTiny) firmware
  - Compile the Source/Power/BadgeWX_PowerT10 project with AtmelStudio
    - This will generate a .hex file in its Release subfolder
  - Follow Release instructions (below)
        
#### Test firmware
  - Compile Source/Test/BadgeWX_Autorun_TESTCODE.spin with Propeller Tool and save as autorun.bin (in same folder)
  - Follow Release instructions (below)

### Release
In a Git Bash command line window on a Windows computer, navigate to this repository's Program subfolder and perform one or more of the following...

#### Full Release (recommended for most cases; Core being the exception)
After building any one or more of the above components...
  - Run the release script:  ```$ ./release.bat```
    - This updates the Release/Boot, Core, Power, and Test folders (as necessary) with .hex and .bin files from Source and builds the Release/Core/fsimage.bin file (the user filesystem image)

#### Boot Firmware Only
  - Optionally, you may run only the release_filesystem script:  ```$ ./release_filesystem.bat```
      - This updates the Release/Boot folder (if necessary) with .bin files from Source/Boot... subfolders and builds the Release/Core/fsimage.bin file (the user filesystem image)

#### Core Firmware
  - Perform Full Release, or individual release steps, as described, then...
  - Manually copy the following files from the Propeller-ESP repository build processes' resulting "build" folder into the Release/Core subfolder of this repository
    - httpd.ota
    - httpd.user1.bin
    - httpd.user2.bin

#### Power Firmware Only
  - Optionally, you may run only the release_power script:  ```$ ./release_power.bat```
    - This updates the Release/Power folder (if necessary) with the .hex file

#### Test Firmware Only
  - Optionally, you may run only the release_test script:  ```$ ./release_test.bat```
    - This updates the Release/Test folder (if necessary) with the autorun.bin file
   
## Program the BadgeWX
For each badge, the Power button firmware needs to be programmed in first, followed by the Core firmware.  Normally, the test jig will be used to do this automatically; however, if
manual programming must be done, here are some tips.

#### Power Firmware
  1) Connect a PropPlug to the appropriate pins on the BadgeWX or test jig (Gnd, Res, Tx, Rx)
  2) Power on the board/jig
  3) Start Atmel Studio
  4) Choose the "Device Programming" menu item
  5) Set "Tool" to Atmel-ICE
  6) Set "Device" to ATtiny10
  7) Set "Interface" to TPI, click Apply
  8) Click Read
     The "Target Voltage" should read around 4.5 V
  9) Select Memories
  10) Set "Flash (1 KB)" field to path of BadgeWX_PowerT10.hex file
  11) Checkmark "Erase device before programming"
  12) Checkmark "Verify Flash after programming"
  13) Click the Program button

#### Core Firmware
  1) Open a command-line window and navigate to this folder
  2) Connect a PropPlug to the appropriate pins on the BadgeWX or test jig (Gnd, Res, Tx, Rx)
  3) Ground the PGM pin
  4) Power on the board/jig
  5) Perform the Erase procedure (replacing <com_port> with appropriate port name)
```
     $ python -m esptool --baud 921600 --port <com_port> --before no_reset --after no_reset erase_flash
```
```     
     EXAMPLE OUTPUT:
       esptool.py v2.4.1
       Serial port com7
       Connecting....
       Detecting chip type... ESP8266
       Chip is ESP8266EX
       Features: WiFi
       MAC: b4:e6:2d:2d:50:f1
       Uploading stub...
       Running stub...
       Stub running...
       Changing baud rate to 921600
       Changed.
       Erasing flash (this may take a while)...
       Chip erase completed successfully in 3.6s
       Staying in bootloader.
```             
  6) Perform the Programming procedure (replacing <com_port> with appropriate port name)
```
     $ python -m esptool --baud 921600 --port <com_port> --before no_reset --after no_reset write_flash 0x00000 boot_v1.6.bin 0x01000 httpd.user1.bin 0x100000 fsimage.bin 0x3FC000 esp_init_data_default.bin
```  
```       
     EXAMPLE OUTPUT:
       esptool.py v2.4.1
       Serial port com7
       Connecting....
       Detecting chip type... ESP8266
       Chip is ESP8266EX
       Features: WiFi
       MAC: b4:e6:2d:2d:50:f1
       Uploading stub...
       Running stub...
       Stub running...
       Changing baud rate to 921600
       Changed.
       Configuring flash size...
       Auto-detected Flash size: 4MB
       Flash params set to 0x004f
       Compressed 3856 bytes to 2763...
       Wrote 3856 bytes (2763 compressed) at 0x00000000 in 0.1 seconds (effective 385.6 kbit/s)...
       Hash of data verified.
       Compressed 318964 bytes to 240304...
       Wrote 318964 bytes (240304 compressed) at 0x00001000 in 3.4 seconds (effective 741.9 kbit/s)...
       Hash of data verified.
       Compressed 120148 bytes to 66192...
       Wrote 120148 bytes (66192 compressed) at 0x00100000 in 1.1 seconds (effective 883.4 kbit/s)...
       Hash of data verified.
       Compressed 128 bytes to 75...
       Wrote 128 bytes (75 compressed) at 0x003fc000 in 0.0 seconds (effective 21.3 kbit/s)...
       Hash of data verified.
       
       Leaving...
       Staying in bootloader.  
```     
  OLD PROGRAMMING METHOD)
     The following is the appropriate erase+program statement for the old esptool Windows executable (no longer used):
```
     $ ./esptool -cd none -cb 921600 -cp <com_port> -bz 4M -bf 80 -bm qio -ca 0x00000 -cf boot_v1.6.bin -ca 0x01000 -cf httpd.user1.bin -ca 0x7e000 -cf blank.bin -ca 0x7f000 -cf blank.bin -ca 0x100000 -cf fsimage.bin -ca 0x3FA000 -cf blank.bin -ca 0x3FC000 -cf esp_init_data_default.bin -ca 0x3FE000 -cf blank.bin
```
   
   
   