{{

----------------------------------

Parallax BadgeWX Loading Message
  - keep small to load fast!

Written by: Michael Mulholland
Last Update: 2019 Jan 27

----------------------------------

' **************************************************
' FROM THIS SRC, GENERATE A FILE CALLED: autorun.bin
' UPLOAD TO BADGEWX WIFI MODULE  /file folder IN MFG
' **************************************************


' ------------------------------------------------------------------------------------------------------------------------

' Version history

' ------------------------------------------------------------------------------------------------------------------------

' 2019.01.27.
'
'       Move Set,STA here, so it's done well before autorunSD calls JOIN
'       Couple smaller updates


' 2019.01.20.
'
'       Set RGB LEDs to OFF immediately on startup
'

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

        'WX_RES = 16 ' ESP WiFi module Reset Pin
        'WX_BOOT = 17 ' ESP Bootloader Lock Pin

        STRIP_PIN = 10
        STRIP_LEN = 4
       
        


OBJ
        
        wificom : "FullDuplexSerial_badgewx_minimal"
        oled    : "sj_oled_badgewx_minimal"
        strip   : "jm_rgbx_pixel_minimal"


VAR

        long  pixels1[STRIP_LEN]

        
  
PUB Start


  ' Set up and clear RGB LEDs
  longfill(@pixels1, $00_00_05_00, STRIP_LEN) ' Blue 05 for nice calm Blue color (testing - change to off for Production)
  'longfill(@pixels1, $00_00_00_00, STRIP_LEN) ' All zero = All LED's off
    
  'strip.startx(@pixels1, STRIP_LEN, STRIP_PIN, 1_0, true, 24, 300,900,900,350) ' Timing for RevA 1818 build
  'strip.startx(@pixels1, STRIP_LEN, STRIP_PIN, 1_0, true, 24, 400,850,800,450) ' Timing for other builds

  
  ' Recalculated timing based on minimum values published in WorldSemi datasheet, + 50
  ' Concept:
  ' 1. maximum should not be a concern for Propeller to time within,
  ' 2. and we just need to pass minimum a little bit for tolerance.
  
  strip.startx(@pixels1, STRIP_LEN, STRIP_PIN, 1_0, true, 24, 250,700,600,500) ' Timing for RevA 1818, RevB 1823

  


  ' Set up display
  
  oled.init(OLED_CS, OLED_DC, OLED_DAT, OLED_CLK, OLED_RST) ', oled#SSD1306_SWITCHCAPVCC, oled#TYPE_128X64)

  oled.write2x8String(string(" HELLO! "),8,0)
  oled.write4x16String(string("Loading..."),10,6,4)
  
  oled.updateDisplay


  {
  ' Reset WiFi module
  ' Note: This leaves the Badge unlocked. The next autorun phase will re-lock!
  
  OUTA[WX_BOOT]~~          ' Set to high (disable bootloader lock- IMPORTANT!)
  DIRA[WX_BOOT]~~          ' Set to output

  OUTA[WX_RES]~            ' Set to low (reset WiFi module)
  DIRA[WX_RES]~~           ' Set to output

  waitcnt(clkfreq/5000 + cnt)  ' ESP module requires 100uS minimum pulse

  DIRA[WX_RES]~            ' Clear to input (remove reset!)

  
  ' WiFi module takes minimum 160ms to start. Wait for 500ms here.
  waitcnt(clkfreq/2 + cnt)
  }

  

  
  ' --- FRUN load the next autorun stage


  
  
  ' Set wiFi module to command mode, by holding DI low longer than 30/baud seconds
  DIRA[WX_DI]~~                 ' Set to output
  OUTA[WX_DI]~                  ' Set to low
  waitcnt(clkfreq/800 + cnt)    ' 125us second pause
  DIRA[WX_DI]~                  ' Set to input

  
  wificom.start(WX_DO,WX_DI,0,115200) ' rxpin, txpin, mode, baudrate

  'wificom.rxflush   ' Clear RX buffer

  wificom.str(string($FE,"CHECK:module-name",13,10))
  waitcnt(clkfreq / 1000 + cnt)
  wificom.str(string($FE,"CHECK:wifi-mode",13,10))
  waitcnt(clkfreq / 1000 + cnt)

  'wificom.str(string($FE,"SET:wifi-mode,AP",13,10)) ' Will disconnect and cancel existing connection states
  'waitcnt(clkfreq/2 + cnt)
  'wificom.str(string($FE,"CHECK:wifi-mode",13,10))
  'waitcnt(clkfreq / 1000 + cnt)
  wificom.str(string($FE,"SET:wifi-mode,STA",13,10))
  waitcnt(clkfreq/4 + cnt)
  
  'wificom.str(string($FE,"CHECK:wifi-mode",13,10))
  'waitcnt(clkfreq / 1000 + cnt)
  
  wificom.str(string($FE,"FRUN:autorunSD.bin",13,10))
  
  'wificom.stop
  
  ' DO NOT RX or TX to WIFICOM after issuing FRUN command !
  

  '''wificom.rx ' $FE
  '''wificom.rx ' =

  
  ' If not successful,  try running the bootloader
  ' - Possible that the SD file does not exist if the user doesn't want that feature

  '''if (wificom.rx <> "S")

      '''wificom.str(string($FE,"FRUN:autorunBL.bin",13,10))
    

  ' ---

  ' 1 second wait, allow FRUN to execute
  waitcnt(clkfreq + cnt)
  
  
  ' Should never reach here, if WiFi module executed the FRUN
  
                                                           
  ' --- BLINK LED 1 as diagnostic info for tech support
  
  repeat 
     
      OUTA[LED_COM]~            ' Set to low
      DIRA[LED_COM]~~           ' Set to output
        
      waitcnt(clkfreq/8 + cnt)  ' 1/8 second pause

      DIRA[LED_COM]~            ' Set to input    
  

' ----------------------------------------------------------------------------------------------------------------
   