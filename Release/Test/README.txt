These are the files for testing the BadgeWX in Production.  They should be placed on an SD card that is connected to the tester jig (or inserted directly into the BadgeWX before power-up).


IMPORTANT:
  To optimize repository size, the binary files in this folder are managed by Git LFS.  You MUST have Git LFS installed and initialized before checking out this commit in order
  to retrieve these assets.  https://git-lfs.github.com

  If you don't need the binary assets, you don't need to install Git LFS, just don't change the binary contents of this folder and all should be well for further commits.


FILES:
  autorun.bin - The autorun test code.  Upon boot-up, the BadgeWX automatically loads this file from the SD card and runs it on the Propeller.
  wifi.txt    - This contains the Wi-Fi credentials for use during BadgeWX testing.  Upon boot-up, the BadgeWX automatically loads this file and configures it's Wi-Fi Module accordingly.
