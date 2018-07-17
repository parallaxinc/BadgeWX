These are the "AutoRun" files that are stored in the BadgeWX's ESP8266 User Filesystem.  
They are Propeller Application images (ie: *.binary files renamed to *.bin).

Upon power-up...
 - The ESP automatically loads the autorun.bin file into the Propeller's RAM.
   - The autorun.bin application displays a "splash screen" on the OLED display then tells the ESP to load the autorunSD.bin to the Propeller next.
 - The ESP loads the autorunSD.bin file into the Propeller's RAM.
   - The autorunSD.bin application checks for an inserted SD card.
     - If SD card exists and includes specific files, the SD's wifi.txt file (access point credentials) is used to configure the ESP module, and the
       SD's autorun.bin file is loaded into the Propeller's RAM.
     - If SD card doesn't exist, or doesn't include its own autorun.bin file, the autorunSD.bin appliction then tells the ESP to load the autorunBL.bin
       to the Propeller next.
 - The ESP loads the autorunBL.bin file into the Propeller's RAM.
   - The autorunBL.bin application provides an interactive menu system allowing the user to manually search for and associate with Wi-Fi access points, 
     run any EEPROM-based Propeller Application, clear the EEPROM, etc.
  
   