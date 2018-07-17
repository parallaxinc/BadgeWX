# BadgeWX
BadgeWX Firmware and Test Code

## Binary Assets
Binary assets are included and managed by Git LFS.  See "_Binary_Assets_Are_GitLFS_Managed-README.txt" for more details.  Most are in the Releases subfolder and are important since
there are multiple sources and multiple binaries that work together to complete the product.

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
   
## Updating the Release
The Release consists of many Build/Release parts; perform any of the build and/or release steps (below) as necessary
### Build the Power (ATTiny) firmware
  - Compile the Source/Power/BadgeWX_PowerT10 project with AtmelStudio
    - This will generate a .hex file in its release subfolder
  - If only updating this component...
    - In a Git Bash command line window, navigate to the Program subfolder
    - Run the update_attiny script:  $ ./update_attiny.bat
      - This updates the Release/Power folder (if necessary) with the .hex file
### Build the Boot firmware
  - autorun.bin - Compile Source/Boot/autorun/autorun.spin with Propeller Tool and save as autorun.bin (in same folder)
  - autorunBL.bin - Compile Source/Boot/autorun/BadgeWX-Bootloader.side (c project) with SimpleIDE
    - This will save the image as autorunBL.bin (in same cmm subfolder)
  - autorunSD.bin - Compile Source/Boot/autorun/autorunSD.spin with Propeller Tool and save as autorunSD.bin (in same folder)
  - If only updating this component...
    - In a Git Bash command line window, navigate to the Program subfolder
    - Run the make_filesystem script:  $ ./make_filesystem.bat
      - This updates the Release/AutoRun folder (if necessary) with .bin files from Source/Boot... subfolders and builds the Release/Core/fsimage.bin file (the user filesystem image)
### Build the Test firmware
  - Compile Source/Test/BadgeWX_Autorun_TESTCODE.spin with Propeller Tool and save as autorun.bin (in same folder)
  - If only updating this component...
    - In a Git Bash command line window, navigate to the Program subfolder
    - Run the update_test script:  $ ./update_test.bat
      - This updates the Release/Test folder (if necessary) with the autorun.bin file
### Build the Core Firmware   
  - "Make" the firmware from the [Parallax-ESP repository](https://github.com/parallaxinc/Parallax-ESP)
  - Copy the following files from that build to the Release/Core subfolder of this repository
    - httpd.ota
    - httpd.user1.bin
    - httpd.user2.bin
### Release all files
After building any one or more of the above...    
  - In a Git Bash command line window, navigate to the Program subfolder
  - Run the release script:  $ ./release.bat
    - This updates the Release/Power, Boot, Core, and Test folders (as necessary) with .hex and .bin files from Source and builds the Release/Core/fsimage.bin file (the user filesystem image) 
   