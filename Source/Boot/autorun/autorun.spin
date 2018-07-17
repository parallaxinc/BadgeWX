{{

----------------------------------

Parallax BadgeWX Loading Message
  - keep small to load fast!

Written by: Michael Mulholland
Last Update: 2018 May 17

----------------------------------

' **************************************************
' FROM THIS SRC, GENERATE A FILE CALLED: autorun.bin
' UPLOAD TO BADGEWX WIFI MODULE  /file folder IN MFG
' **************************************************


' ------------------------------------------------------------------------------------------------------------------------

' Version history

' ------------------------------------------------------------------------------------------------------------------------


' 2018.05.17.
'
'       More shrinking, including OBJ files
'       Remove testing LED flash at start
'       Rename and tidy for release


' 2018.04.16.
'
'       Shrink size / prepare for release


' 2018.03. 28.
'
'       Initial version

' ------------------------------------------------------------------------------------------------------------------------

' 

' ------------------------------------------------------------------------------------------------------------------------
' ------------------------------------------------------------------------------------------------------------------------
' ------------------------------------------------------------------------------------------------------------------------ 



Steps...

1. Display Loading message

2. Blink LEDS (fairly quick, to suggest something happening fast!)

3. FRUN control the WiFi module to load/run autorunSD.bin (This will kill this code, BUT oled will keep the loading image!)

4. If autorunSD.bin not found, try autorunBL.bin instead

 
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

        WX_DI =  30 ' Duplcate pin defines, from WiFi module perspective, used in WiFi code
        WX_DO =  31 ' Duplcate pin defines, from WiFi module perspective, used in WiFi code

       
        


OBJ
        
        wificom : "FullDuplexSerial_badgewx_minimal"
        oled    : "sj_oled_badgewx_minimal"
        


PUB Start


  ' Set up display
  
  oled.init(OLED_CS, OLED_DC, OLED_DAT, OLED_CLK, OLED_RST) ', oled#SSD1306_SWITCHCAPVCC, oled#TYPE_128X64)

  oled.write2x8String(string(" HELLO! "),8,0)
  oled.write4x16String(string("Loading..."),10,6,4)
  
  oled.updateDisplay



  ' --- FRUN load the next autorun stage

  
  ' Set wiFi module to command mode, by holding DI low longer than 30/baud seconds
  DIRA[WX_DI]~~                 ' Set to output
  OUTA[WX_DI]~                  ' Set to low
  waitcnt(clkfreq/800 + cnt)    ' 125us second pause
  DIRA[WX_DI]~                  ' Set to input

  
  wificom.start(WX_DO,WX_DI,0,115200) ' rxpin, txpin, mode, baudrate

  wificom.rxflush   ' Clear RX buffer
  
  wificom.str(string($FE,"FRUN:autorunSD.bin",13,10))

  wificom.rx ' $FE
  wificom.rx ' =

  
  ' If not successful,  try running the bootloader
  ' - Possible that the SD file does not exist if the user doesn't want that feature

  if (wificom.rx <> "S")

      wificom.str(string($FE,"FRUN:autorunBL.bin",13,10))
    

  ' ---
  
  
  ' Should never reach here, if WiFi module executed the FRUN
  
  
  ' --- BLINK LED 1 as diagnostic info for tech support
  
  repeat 
     
      OUTA[LED_COM]~            ' Set to low
      DIRA[LED_COM]~~           ' Set to output
        
      waitcnt(clkfreq/8 + cnt)  ' 1/8 second pause

      DIRA[LED_COM]~            ' Set to input    
  

' ----------------------------------------------------------------------------------------------------------------
   