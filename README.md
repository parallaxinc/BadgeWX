# BadgeWX
BadgeWX Firmware and Test Code

## Binary Assets
Binary assets are included and managed by Git LFS.  See "_Binary_Assets_Are_GitLFS_Managed-README.txt" for more details.

## Folder Structure
 - Program - Tools used in generating the program files
 - Release - Generated binary release files
   - ATTiny    - ATTiny microcontroller binary image (soft-power button controller)
   - AutoRun   - Copy of the "autorun" BadgeWX boot files from Source/AutoRun/...
   - ESP       - BadgeWX firmware and user filesystem (for built-in ESP8266-based module)
   - Test      - BadgeWX SD-based test firmware
 - Source
   - AutoRun     - Autorun files' source code
   - Examples    - Default source examples (.html, .png), if any, for user filesystem
   - PowerButton - ATTiny soft-power button source code
   - Test        - Test application source code (autorun.bin) and Wi-Fi credentials (wifi.txt) for SD card
   
## Building ESP Release
The ESP Release image consists of ESP firmware plus user filesystem.
  - Build ESP Firmware
    - "Make" the firmware from the [Parallax-ESP repository](https://github.com/parallaxinc/Parallax-ESP)
    - Copy the following files from that build to the Release/ESP subfolder of this repository
      - httpd.ota
      - httpd.user1.bin
      - httpd.user2.bin
  - Build User Filesystem
    - (currently requires a Windows computer and Git Bash)
    - In a Git Bash command line window, navigate to the Program subfolder
    - Run the make_filesystem script:  $ ./make_filesystem.bat
      - This updates the Release/AutoRun folder (if necessary) with .bin files from Source/AutoRun... subfolders and builds the Release/ESP/fsimage.bin file (the user filesystem image) 
   