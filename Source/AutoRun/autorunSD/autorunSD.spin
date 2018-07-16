{{

----------------------------------

Parallax BadgeWX SD Loader stage
  
Written by: Michael Mulholland
Last Update: 2018 May 17

----------------------------------

' ****************************************************
' FROM THIS SRC, GENERATE A FILE CALLED: autorunSD.bin
' UPLOAD TO BADGEWX WIFI MODULE  /file folder IN MFG
' ****************************************************


' ------------------------------------------------------------------------------------------------------------------------

' Version history

' ------------------------------------------------------------------------------------------------------------------------

' 2018.06.19.
'
'       Change serial code to include break feature
'

' 2018.06.15.
'
'       Added new Parallax-ESP command "SETAP,ssid,password". Use that instead of JOIN



' 2018.05.17.
'
'       More shrinking, including OBJ files
'       Remove testing LED flash at start
'       Rename and tidy for release
'       Extend to 32 char SSID and PASSWORD
'       Remove dependence on CR, LF, EOL chars
'


' 2018.04.16.
'
'       Initial version



' ------------------------------------------------------------------------------------------------------------------------

' TODO: Add rock solid error handling around the SD routines
' -- idea- stick WiFi autorun into another cog which runs at start, configs the wifi connection,
'          waits 30 seconds (or whatever is appropriate)
'          then runs the FRUN command (or 

' ------------------------------------------------------------------------------------------------------------------------
' ------------------------------------------------------------------------------------------------------------------------
' ------------------------------------------------------------------------------------------------------------------------ 



Steps...

1. Display Loading message

2. Blink LEDS (fairly quick, to suggest something happening fast!)

3. FRUN control the WiFi module to load/run autorunBL.bin (This will kill this code, BUT oled will keep the loading image!)

4. If autorunBL.bin not found, nothing left to do ! (Consider OLED msg?)

 
' ------------------------------------------------------------------------------------------------------------------------ 

}}


CON
        _clkmode = xtal1 + pll16x                       'Standard clock mode * crystal frequency = 80 MHz
        _xinfreq = 5_000_000

        LED_COM = 9

        OLED_CS  = 18
        OLED_RST = 19
        OLED_CLK = 20
        OLED_DAT = 21
        OLED_DC  = 22

        WX_DI =  30 ' Duplicate pin defines, from WiFi module perspective, used in WiFi code
        WX_DO =  31 ' Duplicate pin defines, from WiFi module perspective, used in WiFi code

        WX_RES = 16 ' ESP WiFi module Reset Pin
        WX_BOOT = 17 ' ESP Bootloader Lock Pin
         
       
        ' SD pins (REV A2 - Black Badge)
        _dopin = 8
        _clkpin = 7
        _dipin = 6
        _cspin = 5
        _cdpin = -1 ' -1 if unused.
        _wppin = -1 ' -1 if unused.
         
         
         
VAR

        long idx, WiFiCogReady', resetWiFi
        byte ssid[44], pass[44]

        byte buffer[128]
        
        long timeoutMillis
                
        long Stack[512]



OBJ
        
        wificom : "FullDuplexSerial_badgewx_minimal"
        oled    : "sj_oled_badgewx_minimal"
        fat     : "SD-MMC_FATEngine_badgewx_minimal"
        

PUB Start

  'resetWiFi := 0
  WiFiCogReady := 0
  

  ' Call WiFi cog (which also operates as watchdog)
  cognew(WiFiCog, @Stack)

  
  ' Set up display
  oled.init(OLED_CS, OLED_DC, OLED_DAT, OLED_CLK, OLED_RST)
     
  oled.write2x8String(string(" HELLO! "),8,0)   ' (str,len,row)  
  oled.write4x16String(string(" Checking SD... "),16,6,0)
  
  oled.updateDisplay
  
  repeat until WiFiCogReady == 1
    waitcnt(0)

  ' SD card related stuff

  ' Check SD card for WiFi settings, and write to ESP module if exist 
  ' Check SD card for valid firmware, and boot if exists     
  sd_autoload ' blocking

  
  ' Clear timeout so next stage Bootloader loads immediately
  ' Is this signed? ... set to 1 rather than zero for now. Only 1ms delay
  timeoutMillis := 1


  ' 10 millisecond wait, allow FRUN to execute in other cog
  waitcnt(clkfreq/100 + cnt)
  

  ' ---
    
  
  ' Should never reach here, if WiFi module executed the FRUN
  
  
  ' --- BLINK LED 2 as diagnostic info for tech support

  repeat 
   
      OUTA[LED_COM]~~           ' Set to high
      DIRA[LED_COM]~~           ' Set to output
      
      waitcnt(clkfreq/8 + cnt)  ' 1/8 second pause
   
      DIRA[LED_COM]~            ' Set to input




' ----------------------------------------------------------------------------------------------------------------
  


PRI WiFiCog


    ' Set wiFi module to command mode, by holding DI low longer than 30/baud seconds
    DIRA[WX_DI]~~               ' Set to output
    OUTA[WX_DI]~                ' Set to low
        
    waitcnt(clkfreq/800 + cnt)  ' pause

    DIRA[WX_DI]~                ' Set to input
        
     
    wificom.start(WX_DO,WX_DI,0,115200) ' rxpin, txpin, mode, baudrate
    'wificom.break

    

    ' TODO: How to handle situation when ESP really doesn't respond? Flash LED a certain way?
    ' hmm.. LED1 should be stuck on in this case, to indicate autorunSD has failed- which must be an ESP issue.

    wificom.str(string($FE,"CHECK:module-name",13,10))
                
    WiFiCogReady := 1      
  
    'DIRA[LED_COM]~~               ' Set to output
    'OUTA[LED_COM]~

    'wificom.str(string("CHECK:module-name"))

      
    ' Wait for timeout seconds...
    ' - Cog0 will keep updating this timer whilst it is operating correctly,
    '   so the timer will expire if something goes wrong!
    timeoutMillis := 2000

    
    repeat until timeoutMillis < 1

        waitcnt(clkfreq/1000 + cnt)
        --timeoutMillis
        

    {if (resetWifi == 1)

          ' TODO: Surely the underlying ESP API has a soft-restart command, so it gracefully cleans then reboots ??
          '        - consider implementing in future Parallax-ESP if this rug-pulling method proves unreliable

          
          
          wificom.stop ' To release DI for a moment...

          

          ' do the reset (twice!)
          ' first time, reset/restart action will report as: rst cause:2, boot mode:(3,7)
          ' second time, it will work when reported as: rst cause:2, boot mode:(3,6)
          ' I tried calling this repeat 5 times, and after the 1st "failed" 3,7 reset, the
          ' further 4 resets are all 3,6. So this could be called multiple times- seems only the
          ' first reset fails.

          ' ESP internal error:
          ' https://www.pieterverhees.nl/sparklesagarbage/esp8266/130-difference-between-esp-reset-and-esp-restart
          ' https://github.com/esp8266/Arduino/issues/1017

          
          repeat 2
          
                                OUTA[WX_RES]~            ' Set to low
                                DIRA[WX_RES]~~           ' Set to output
                                waitcnt(CNT + CLKFREQ/800) 
                                DIRA[WX_RES]~            ' Set to input
                                 
                                ' disable bootloader lock- IMPORTANT!
                                OUTA[WX_BOOT]~~          ' Set to high
                                DIRA[WX_BOOT]~~          ' Set to output
                                 
                                ' wait for reset to complete, and the Parallax-ESP firmware to initialise            
                                waitcnt(CNT + CLKFREQ / 3)  ' ~333 ms - ESP module must reboot whilst lock held
                                DIRA[WX_BOOT]~            ' Set to input - release the lock 

    
          ' Set wiFi module to command mode, by holding DI low longer than 30/baud seconds
          DIRA[WX_DI]~~               ' Set to output
          OUTA[WX_DI]~                ' Set to low
          waitcnt(clkfreq/800 + cnt)  ' pause
          'DIRA[WX_DI]~                ' Set to input

          'OUTA[WX_DI]~~                ' Set to high
          DIRA[WX_DI]~                ' Set to input
          
          'wificom.break
             
          ' Re-start comms 
          wificom.start(WX_DO,WX_DI,0,115200) ' rxpin, txpin, mode, baudrate
          'wificom.setBadgeCmdMode
          }                      

    
    ' Update oled message for next stage
    oled.write4x16String(string("Searching WiFi.."),16,6,0)
    oled.updateDisplay

    
    ' Talk to ESP- make sure it's alive after the reset
    ' else this data may be useful for future application
    
    wificom.rxflush ' Clear the buffer (it will be full of junk since the reboot - the pause might be what's needed, rather than the actual flush :)

    wificom.str(string($FE,"CHECK:wifi-mode",13,10))
    'wificom.str(string($FE,"CHECK:wifi-ssid",13,10))
    'wificom.str(string($FE,"CHECK:station-ipaddr",13,10))
        
    'wificom.rxflush

    waitcnt(clkfreq + cnt)
    
    ' --- FRUN load the next autorun stage
    wificom.str(string($FE,"FRUN:autorunBL.bin",13,10))

    ' Allow about 1.9mS + fudge factor, to transmit the msg before stopping the cogs
    
    'waitcnt(clkfreq/400 + cnt) ' ~2.5mS
    
    ' Call reboot- if ESP fails for whatever reason, at least the user firmware might execute....
    'reboot


    ' HALT!
    
    repeat
      waitcnt(0)  
    

' ----------------------------------------------------------------------------------------------------------------
 
      
PRI sd_autoload | charcount, bufidx


  ' Check if card exists before starting driver!
  DIRA[_cspin]~~
  OUTA[_cspin]~
  
  waitcnt(clkfreq/100 + cnt)

  DIRA[_cspin]~

  if (INA[_cspin] == 0) ' No SD card inserted

      return


  ' // ---

  
  ' SD card detected

  
  ' // ---


  if !fat.fatEngineStart( _dopin, _clkpin, _dipin, _cspin, -1, -1, -1, -1, -1)
      return
      
   
  if(fat.partitionMounted)
      \fat.unmountPartition


  fat.mountPartition(0)
  
  
  if(!fat.partitionMounted)
      return
    


  ' Reset timeout
  timeoutMillis := 2000
  
  
  ' // Check for WiFi settings file


  ' /////
  
  ' LOGIC
  '
  ' valid char in the range 32 to 126
  '
  ' 1. read chars until first valid char (abort if EOF)
  
  ' 2. SSID = read chars until first non-valid char; could be CR, LF, whatever (abort if EOF, or more than 32 chars)

  ' 3. read chars until next valid char (abort if EOF)

  ' 4. PASS = read chars until first non-valid char; could be CR, LF, whatever (abort if EOF, or more than 32 chars)

  ' /////


  fat.openFile(string("wifi.txt"), "R")

  charcount := fat.readData(@buffer, 128)

  fat.closeFile


  ' Errors?
  if (charcount < 1)
      return


      
  ' Reset timeout
  timeoutMillis := 2000


  
  ' parse buffer
  bufidx := 0

  
  ' ignore any control chars at start of file (CR, LF, etc...)    
  repeat while ( ((buffer[bufidx] < 32) or (buffer[bufidx] > 126)) and (bufidx < charcount) )
      bufidx++

  

  ' grab SSID up until next control char, or EOF
  idx := 0
  repeat while ( (buffer[bufidx] > 31) and (buffer[bufidx] < 127) and (bufidx < charcount) )
      ssid[idx++] := buffer[bufidx++]


  
  ' ignore any control chars at start of file (CR, LF, etc...)    
  repeat while ( ((buffer[bufidx] < 32) or (buffer[bufidx] > 126)) and (bufidx < charcount) )
      bufidx++


  
  ' grab PASS up until next control char, or EOF
  idx := 0
  repeat while ( (buffer[bufidx] > 31) and (buffer[bufidx] < 127) and (bufidx < charcount) )
      pass[idx++] := buffer[bufidx++]



  ' disable bootloader lock- IMPORTANT!
  OUTA[WX_BOOT]~~          ' Set to high
  DIRA[WX_BOOT]~~          ' Set to output
  
  
  
  ' Send JOIN network command to WiFi module

  wificom.rxflush   ' Clear RX buffer

  wificom.str(string($FE,"SET:wifi-mode,STA+AP",13,10))
  waitcnt(clkfreq/2 + cnt)

  wificom.str(string($FE,"SET:wifi-mode,STA",13,10))
  'wificom.str(string($FE,"SET:wifi-mode,STA",13,10))
  
  
  wificom.str(string($FE,"JOIN:")) ' wificom.str(string($FE,"APSET:")) ' 
  wificom.str(@ssid)
  wificom.str(string(","))
  'wificom.str(string("BADDPASS")) ' TEST
  wificom.str(@pass)
  wificom.str(string(13,10))

  'wificom.str(string($FE,"SET:wifi-mode,STA+AP",13,10))
  'wificom.str(string($FE,"SET:wifi-mode,STA+AP",13,10))

            
  'resetWiFi := 1 ' ESP module must be reset after setting AP details (quick solution to an ESP firmware issue)




  ' Hold BOOT pin high, restart ESP module, Release BOOT pin after 300mSecs
  ' -- note, BOOT lock was already disabled before APSET command. No need to repeat here,
  ' -- also, release will happen when Propeller moves to next autorun stage, or boots SD firmware

  'wificom.str(string($FE,"RESTART:0",13,10))




  

  'waitcnt(clkfreq/4 + cnt)
  'wificom.stop

  
  ' Reset timeout
  'timeoutMillis := 4000

  
  ' Reset the ESP module
  {
  ' disable bootloader lock- IMPORTANT!
  OUTA[WX_BOOT]~~          ' Set to high
  DIRA[WX_BOOT]~~          ' Set to output

  OUTA[WX_RES]~            ' Set to low
  DIRA[WX_RES]~~           ' Set to output

  waitcnt(CNT + CLKFREQ/1000) ' 1 ms
  DIRA[WX_RES]~            ' Set to input 

  
  waitcnt(CNT + CLKFREQ/2)  ' ~500 ms - ESP module must reboot whilst lock held
  DIRA[WX_BOOT]~            ' Set to input - release the lock


  
  ' Set wiFi module back to command mode, by holding DI low longer than 30/baud seconds
  DIRA[WX_DI]~~               ' Set to output
  OUTA[WX_DI]~                ' Set to low
        
  waitcnt(clkfreq/800 + cnt)  ' 125us second pause

  DIRA[WX_DI]~                ' Set to input


  wificom.start(WX_DO,WX_DI,0,115200) ' rxpin, txpin, mode, baudrate

  }
    
  oled.write4x16String(string("  Reading SD... "),16,6,0)
  oled.updateDisplay

  
  ' Reset timeout - allow enough time for file to load and execute
  ' NOTE: This timeout extended to 2 secs, as we want a little time after APSET is written to ESP module, before calling reset.
  ' - give time for ESP to write the ssid,password to flash memory.
  
  timeoutMillis := 2000


  ' Check if file exists
  ' TODO: I believe the openFile will "hang" the code, so the 1 second timeout (or less) should be sufficient detection of no-file-detected
  fat.openFile(string("autorun.bin"), "R")
  'charcount := fat.readData(@buffer, 8)
  fat.closeFile
  
  ' Errors?
  'if (charcount < 1)
  '    return
  

  timeoutMillis := 8000
  
  ' // Boot from autorun.bin, if the file exists
  ' TODO: Does this return FAST if file not found?
  fat.bootPartition(string("autorun.bin"))


  ' Won't get here if the boot worked!
  ' Otherwise, clean up and return


  fat.unmountPartition

  
  ' Reset timeout
  timeoutMillis := 2000

  
  ' Stop SD Driver
  waitcnt(clkfreq + cnt)
  fat.fatEngineStop ' Give the block driver a second to finish up.


  return

  
' ----------------------------------------------------------------------------------------------------------------

' ----------------------------------------------------------------------------------------------------------------