''********************************************
'' *** ***
'' WARNING!
''
'' This version has been stripped down to
'' minimal functionallity for BadgeWX
'' to help speed up load time
''
''********************************************


{{      
************************************************
* OLED_AsmFast (well, faster)                  *
* Thomas P. Sullivan, Seairth Jacobs           *
* Copyright (c) 2015                           *
* Some original comments left/modified         *
* See end of file for terms of use.            *
************************************************
Revision History:
  V1.0   - Original program 12-2-2012
  V1.1   - Changes to comments and modification 
           of a few commands.
  V1.2   - (2012-16-12) Added support for the 128x64 display
  V1.3   - (2015-09-10 sjacobs) Use DAT instead of VAR

This is a Propeller driver object for the Adafruit
SSDD1306 OLED Display. It has functions to draw
individual pixels, lines, and rectangles. It also
has character functions to print 16x32 characters
derived from the Propeller's internal fonts.


     ┌─────────────────────────┐    
     │         SSD1306         │    
     │         Adafruit        │    
     │          128x32         │    
     │       OLED Display      │    
     │                         │    
     │   RST   CLK   VIN   GND │    
     │ CS   D/C   DATA  3.3    │    
     └─┬──┬──┬──┬──┬──┬──┬──┬──┘    
       │  │  │  │  │  │  │  │


This file is based on the following code sources:
************************************************
* Propeller SPI Engine                    v1.2 *
* Author: Beau Schwabe                         *
* Copyright (c) 2009 Parallax                  *
* See end of file for terms of use.            *
************************************************

...and this code:

*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in
any redistribution
*********************************************************************
Note: The splash screen is way down in the DAT section of this file.
         
}}
CON

  'BLACK = 0
  'WHITE = 1

  'TYPE_128X32                   = 32
  'TYPE_128X64                   = 64
  LCD_BUFFER_SIZE_BOTH_TYPES    = 1024

  'SSD1306_LCDWIDTH              = 128
  'SSD1306_LCDHEIGHT32           = 32
  'SSD1306_LCDHEIGHT64           = 64
  SSD1306_LCDCHARMAX            = 8

  SSD1306_SETCONTRAST           = $81
  SSD1306_DISPLAYALLON_RESUME   = $A4
  SSD1306_DISPLAYALLON          = $A5
  SSD1306_NORMALDISPLAY         = $A6
  SSD1306_INVERTDISPLAY         = $A7
  SSD1306_DISPLAYOFF            = $AE
  SSD1306_DISPLAYON             = $AF
  SSD1306_SETDISPLAYOFFSET      = $D3
  SSD1306_SETCOMPINS            = $DA
  SSD1306_SETVCOMDETECT         = $DB
  SSD1306_SETDISPLAYCLOCKDIV    = $D5
  SSD1306_SETPRECHARGE          = $D9
  SSD1306_SETMULTIPLEX          = $A8
  SSD1306_SETLOWCOLUMN          = $00
  SSD1306_SETHIGHCOLUMN         = $10
  SSD1306_SETSTARTLINE          = $40
  SSD1306_MEMORYMODE            = $20
  SSD1306_COMSCANINC            = $C0
  SSD1306_COMSCANDEC            = $C8
  SSD1306_SEGREMAP              = $A0
  SSD1306_CHARGEPUMP            = $8D
  'SSD1306_EXTERNALVCC           = $1
  'SSD1306_SWITCHCAPVCC          = $2

  'Scrolling #defines
  SSD1306_ACTIVATE_SCROLL       = $2F
  SSD1306_DEACTIVATE_SCROLL     = $2E
  SSD1306_SET_VERT_SCROLL_AREA  = $A3
  SSD1306_RIGHT_HORIZ_SCROLL    = $26
  SSD1306_LEFT_HORIZ_SCROLL     = $27
  SSD1306_VERTRIGHTHORIZSCROLL  = $29
  SSD1306_VERTLEFTHORIZSCROLL   = $2A


'------------------------------------------------------------------------------------------------------------------------------
DAT
  {{
     Use of DAT means that all instances of this object share the same
     hub memory. Init() should only be called in the top-level object.
  }}     
  cog                   long 0
  command               long 0
  CS                    long 0
  DC                    long 0
  DATA                  long 0
  CLK                   long 0
  RST                   long 0
  'vccstate              long 0
  'SSD1306_LCDWIDTH          long 0
  'SSD1306_LCDHEIGHT64         long 0
  'displayType           long 0
  'AutoUpdate            long 0
  buffer                byte 0[LCD_BUFFER_SIZE_BOTH_TYPES]

'------------------------------------------------------------------------------------------------------------------------------
PRI SHIFTOUT(Dpin, Cpin, CSpin, Bits, Value)             
    setcommand(1, @Dpin)

PRI WRITEBUFF(Dpin, Cpin, CSpin, Bits, Addr)
    setcommand(2, @Dpin)

PUB start:okay

'' Start SPI Engine - starts a cog
'' returns false if no cog available
    'stop
    okay := cog := cognew(@loop, @command) + 1

{PUB stop
'' Stop SPI Engine - frees a cog
    if cog
       cogstop(cog~ - 1)
    command~
}
PRI setcommand(cmd, argptr)
    command := cmd << 16 + argptr                       '' Write command and pointer
    repeat while command                                '' Wait for command to be cleared, signifying receipt

PUB init(ChipSelect,DataCommand,TheData,TheClock,Reset) ' ,VCC_state,Type)
  ''Startup the SPI system
  start

  ''Initialize variables and initialize the display
  CS:=ChipSelect
  DC:=DataCommand
  DATA:=TheData
  CLK:=TheClock
  RST:=Reset
  'vccstate:=VCC_state
  'displayType := Type
  
  {if(displayType==TYPE_128X32)
    SSD1306_LCDWIDTH := SSD1306_LCDWIDTH
    SSD1306_LCDHEIGHT64 := SSD1306_LCDHEIGHT32
  else }

  'displayWidth := SSD1306_LCDWIDTH
  'displayHeight := SSD1306_LCDHEIGHT64

  ''Setup reset and pin direction  
  'HIGH(RST)
  'dira[RST]~~
  'outa[RST]~~

  ' Reset to Input (pullup on pcb)
  dira[RST]~  ' input
  'outa[RST]~  ' low
  
  dira[DC]~~
  
  ''VDD (3.3V) goes high at start; wait for a ms
  waitcnt(clkfreq/100000+cnt)
  
  ''force reset low
  'LOW(RST)
  outa[RST]~ ' low
  dira[RST]~~ ' output

  ''wait 10ms
  waitcnt(clkfreq/100000+cnt)

  ''remove reset
  'HIGH(RST)
  
  ' Reset to Input (pullup on pcb)
  dira[RST]~  ' input
  
    
  ssd1306_command(SSD1306_DISPLAYOFF)             
  ssd1306_command(SSD1306_SETLOWCOLUMN)  ' low col = 0
  ssd1306_command(SSD1306_SETHIGHCOLUMN) ' hi col = 0
  ssd1306_command(SSD1306_SETSTARTLINE)  ' line #0
  ssd1306_command(SSD1306_SETCONTRAST)            
 
  ssd1306_command($CF)
 
  ssd1306_command($A1)
  ssd1306_command(SSD1306_NORMALDISPLAY)
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME)
  ssd1306_command(SSD1306_SETMULTIPLEX)           
  ssd1306_command($3F)
  ssd1306_command(SSD1306_SETDISPLAYOFFSET)       
  ssd1306_command($0) 'No offset                            
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV)     
  ssd1306_command($80)                            
  ssd1306_command(SSD1306_SETPRECHARGE)
 
  ssd1306_command($F1)
 
  ssd1306_command(SSD1306_SETVCOMDETECT)          
  ssd1306_command($40)
 
  ssd1306_command(SSD1306_SETCOMPINS)          
  ssd1306_command($12)
 
  ssd1306_command(SSD1306_MEMORYMODE)          
  ssd1306_command($00)
 
  ssd1306_command(SSD1306_SEGREMAP | $1)
 
  ssd1306_command(SSD1306_COMSCANDEC)
 
  ssd1306_command(SSD1306_CHARGEPUMP)
 
  ssd1306_command($14)
   
  ssd1306_command(SSD1306_DISPLAYON)'--turn on oled panel
 
  ssd1306_command(SSD1306_NORMALDISPLAY)
 
  clearDisplay

PUB clearDisplay
  ''Clearing the display means just writing zeroes to the screen buffer.
  bytefill(@buffer, 0, 1024) ' ((SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT64)/8))
  UpdateDisplay 'Clearing the display ALWAYS updates the display

  
  
PUB updateDisplay|i,tmp
  ''Writes the screen buffer to the memory of the display
  ssd1306_command(SSD1306_SETLOWCOLUMN)  ' low col = 0
  ssd1306_command(SSD1306_SETHIGHCOLUMN) ' hi col = 0
  ssd1306_command(SSD1306_SETSTARTLINE)  ' line #0

  'HIGH(DC)
   
  outa[DC]~~ 

  WRITEBUFF(DATA, CLK, CS ,@tmp, @buffer)    

PUB write2x8String(str,len,row)|i
  row&=$1 'Force in bounds

  repeat i from 0 to (len <# SSD1306_LCDCHARMAX)-1
      write16x32Char(byte[str][i],row,i) 
     
PUB write16x32Char(ch,row,col)|h,i,j,k,q,r,s,mask,cbase,cset,bset
  if(row==0)OR(row==1)AND((col=>0)AND(col<8))
    ''Write a 16x32 character to the screen at position 0-7 (left to right)
    cbase:=$8000+((ch&$FE)<<6)  ' Compute the base of the interleaved character 
      
    repeat j from 0 to 31       ' For all the rows in the font
      bset := |<(j//8)          ' For setting bits in the OLED buffer. The mask is always a byte and has to wrap
      if(ch&$01)
        mask := $00000002       ' For the extraction of the bits interleaved in the font
      else
        mask := $00000001       ' For the extraction of the bits interleaved in the font
      r:=long[cbase][j]         ' Row is the font data with which to perform bit extraction
       s:=0                     ' Just for printing the font  to the serial terminal (DEBUG)
      h:=@buffer+row*512        ' Get the base address of the OLED buffer
      h+=((j>>3)*128)+(col*16)  ' Compute the offset to the column of data and add to the base...
                                ' ...then add the offset to the character position
      repeat k from 0 to 15     ' For all 16 bits we need from the interlaced font...
        if(r&mask)              ' If the bit is set...
          byte[h][k]|=bset      ' Set the column bit
        else
          byte[h][k]&=!bset     ' Clear the column bit
        mask:=mask<<2           ' The mask shifts two places because the fonts are interlaced

    
     
PUB write4x16String(str,len,row,col)|i,j
  ''Write a string of 5x7 characters to the display @ row and column
  repeat j from 0 to len-1
    Write5x7Char(byte[str][j],row,col)  
    col++
    if(col>15)
      col:=0
      row++
  

PUB write5x7Char(ch,row,col)|i    
  ''Write a 5x7 character to the display @ row and column
  col&=$F
  row&=$7
  repeat i from 0 to 7
      buffer[row*128+col*8+i] := byte[@Font5x7+8*ch+i]

  
PUB ssd1306_Command(thecmd)|tmp 'Send a byte as a command to the display
  ''Write SPI command to the OLED
  'LOW(DC)
  outa[DC]~
  
  SHIFTOUT(DATA, CLK, CS ,@tmp, thecmd)   
 
'**********************************************************************************************************
'**  Assembly language driver
'**********************************************************************************************************
DAT           org
''****************************** 
'' SPI Engine - Command dispatch
''******************************
loop          rdlong  DataPin,        par         wz    '' Wait for command via par
        if_z  jmp     #loop                             '' No command (0), keep looking
              movd    :arg,           #arg0             '' Get 5 arguments ; arg0 to arg4
              mov     ClkPin,         DataPin           ''    │
              mov     DataVal,        #5                ''───┘
:arg          rdlong  arg0,           ClkPin            '' mov arg
              add     :arg,           d0                '' It adds 512 because that is a 1 for the destination field.
                                                        '' This increments the destination to point at each of the
                                                        '' arg destinations.
              add     ClkPin,         #4                '' Main memory addresses go up by 4 (for long)
              djnz    DataVal,        #:arg             '' Keep going until done

              shr     DataPin,        #16               '' The command is the upper 16 bits. Slide it down to the lower 16 bits
              cmp     DataPin,        #1          wz    '' Jump to the single SPI shift routine
        if_z  jmp     #SHIFTONE_             
              cmp     DataPin,        #2          wz    '' Jump to the write buffer routine
        if_z  jmp     #WRITEBUFF_             

              wrlong  zero,par                          ''     Zero command to signify command received
NotUsed_      jmp     #loop                             '' 
'################################################################################################################
'Single OLED SPI shift routine
SHIFTONE_                                               '' SHIFTONE Entry

              mov     DataPin,        #1                '' Configure DataPin
              shl     DataPin,        arg0                            
              test    DataPin,        DataPin     wc        
              muxc    dira,           DataPin
              muxc    outa,           DataPin
              mov     ClkPin,         #1                '' Configure ClockPin
              shl     ClkPin,         arg1
              test    ClkPin,         ClkPin      wc        
              muxc    dira,           ClkPin
              muxc    outa,           ClkPin
              mov     CSelPin,        #1                '' Configure Chip Select
              shl     CSelPin,        arg2              
              test    CSelPin,        CSelPin     wc        
              muxc    dira,           CSelPin
              muxnc   outa,           CSelPin
                                                        '' Send Data MSBFIRST
              mov     DataVal,        arg4              '' Load DataValue
              mov     DataMask,       #%10000000        '' Create MSB mask;load DataMask with "1"
              mov     NumBits,        #8                '' Load number of data bits
MSB_Shift2    test    DataVal,        DataMask    wc    '' Test MSB of DataValue
              muxc    outa,           DataPin           '' Set DataBit HIGH or LOW
              shr     DataMask,       #1                '' Prepare for next DataBit
              test    ClkPin,         ClkPin      wc    '' Should always make Z flag 0
              muxnc   outa,           ClkPin            '' Clock Pin Low
              test    ClkPin,         ClkPin      wc    '' Should always make Z flag 0
              muxc    outa,           ClkPin            '' Clock Pin High
              djnz    NumBits,        #MSB_Shift2       '' Decrement NumBits ; jump if not Zero
              test    CSelPin,        CSelPin     wc    '' Should always make Z flag 0
              muxc    outa,           CSelPin           '' Chip Select to output and high
              wrlong  zero,par                          '' Zero command to signify command received
              jmp     #loop                             '' Go wait for next command
'------------------------------------------------------------------------------------------------------------------------------
WRITEBUFF_                                              ''
              mov     DataPin,        #1                '' Configure DataPin
              shl     DataPin,        arg0                           
              test    DataPin,        DataPin     wc       
              muxc    dira,           DataPin
              muxc    outa,           DataPin
              mov     ClkPin,         #1                '' Configure ClockPin
              shl     ClkPin,         arg1
              test    ClkPin,         ClkPin      wc       
              muxc    dira,           ClkPin
              muxc    outa,           ClkPin
              mov     CSelPin,        #1                '' Configure Chip Select
              shl     CSelPin,        arg2              
              test    CSelPin,        CSelPin     wc       
              muxc    dira,           CSelPin
              muxc    outa,           CSelPin

              mov     BufAdr,         arg4              '' Move the address of the beginning of the buffer into T7
              mov     DataCnt,        bsz               '' Move a 1024 into DataCnt (buffer size, 128x32 or 128x64) 

rdbuff        rdbyte  DataVal,        BufAdr            '' Read a byte
              add     BufAdr,         #1                '' Increment to next byte
              test    CSelPin,        CSelPin     wc          
              muxnc   outa,           CSelPin           '' Drive CS low
              mov     DataMask,       #%10000000        '' Create MSB mask              
              mov     NumBits,        #8                '' Load number of data bits
MSB_Shift3    test   DataVal,         DataMask    wc    '' Test MSB of DataValue
              muxc    outa,           DataPin           '' Set DataBit HIGH or LOW
              shr     DataMask,             #1          '' Prepare for next DataBit
              test    ClkPin,         ClkPin      wc       
              muxnc   outa,           ClkPin            '' Clock Pin Low
              test    ClkPin,         ClkPin      wc       
              muxc    outa,           ClkPin            '' Clock Pin High
              djnz    NumBits,        #MSB_Shift3       '' Decrement NumBits (number of bits); jump if not Zero
              test    CSelPin,        CSelPin     wc       
              muxc    outa,           CSelPin           '' Chip Select to output and high
              djnz    DataCnt,        #rdbuff           '' Do until the buffer is empty (1024)

              wrlong  zero,par                          '' Zero command to signify command received
              jmp     #loop                             '' Go wait for next command
'------------------------------------------------------------------------------------------------------------------------------
{
########################### Assembly variables ###########################
}
zero          long    0                                 '' Constant
d0            long    $200                              '' Destination msb
bsz           long    1024                              '' Buffer size
                                              
DataPin       long    0                                 '' Used for DataPin mask
ClkPin        long    0                                 '' Used for ClockPin mask
DataVal       long    0                                 '' Used to hold DataValue
NumBits       long    0                                 '' Used to hold # of Bits
DataMask      long    0                                 '' Used for temporary data mask
CSelPin       long    0                                 '' Used for Chip Select mask
BufAdr        long    0                                 '' Used for buffer address
DataCnt       long    0                                 '' Used to count for buffer write
                                              
arg0          long    0                                 '' Arguments passed to/from high-level Spin
arg1          long    0                       
arg2          long    0                       
arg3          long    0                       
arg4          long    0                       
              fit
''
'' A 5x7 font snagged off of the internet by a student of mine
''
'' 128 characters * 8 bytes per character == 1024 bytes (1K)
''                  Font        Char
Font5x7       byte %11111111   '$00
              byte %11111111   '$00
              byte %11111111   '$00
              byte %11111111   '$00
              byte %11111111   '$00
              byte %00000000   '$00
              byte %00000000   '$00
              byte %00000000   '$00

              byte %11111111   '$01
              byte %11111100   '$01
              byte %11111000   '$01
              byte %11100000   '$01
              byte %11000000   '$01
              byte %10000000   '$01
              byte %00000000   '$01
              byte %00000000   '$01
              
              byte %11111111   '$02
              byte %10100101   '$02
              byte %10011001   '$02
              byte %10100101   '$02
              byte %11111111   '$02
              byte %00000000   '$02
              byte %00000000   '$02
              byte %00000000   '$02
              
              byte %00000001   '$03
              byte %00000111   '$03
              byte %00001111   '$03
              byte %00111111   '$03
              byte %11111111   '$03
              byte %00000000   '$03
              byte %00000000   '$03
              byte %00000000   '$03
              
              byte %10000001   '$04
              byte %01000010   '$04
              byte %00100100   '$04
              byte %00011000   '$04
              byte %00011000   '$04
              byte %00000000   '$04
              byte %00000000   '$04
              byte %00000000   '$04
              
              byte %00011000   '$05
              byte %00011000   '$05
              byte %00011000   '$05
              byte %00011000   '$05
              byte %00011000   '$05
              byte %00000000   '$05
              byte %00000000   '$05
              byte %00000000   '$05
              
              byte %00000000   '$06
              byte %00000000   '$06
              byte %11111111   '$06
              byte %00000000   '$06
              byte %00000000   '$06
              byte %00000000   '$06
              byte %00000000   '$06
              byte %00000000   '$06
              
              byte %11111111   '$07
              byte %10000001   '$07
              byte %10000001   '$07
              byte %10000001   '$07
              byte %11111111   '$07
              byte %00000000   '$07
              byte %00000000   '$07
              byte %00000000   '$07
              
              byte %10101010   '$08
              byte %01010101   '$08
              byte %10101010   '$08
              byte %01010101   '$08
              byte %10101010   '$08
              byte %00000000   '$08
              byte %00000000   '$08
              byte %00000000   '$08
              
              byte %10101010   '$09
              byte %01010101   '$09
              byte %10101010   '$09
              byte %01010101   '$09
              byte %10101010   '$09
              byte %00000000   '$09
              byte %00000000   '$09
              byte %00000000   '$09
              
              byte %10101010   '$0A
              byte %01010101   '$0A
              byte %10101010   '$0A
              byte %01010101   '$0A
              byte %10101010   '$0A
              byte %00000000   '$0A
              byte %00000000   '$0A
              byte %00000000   '$0A
              
              byte %10101010   '$0B
              byte %01010101   '$0B
              byte %10101010   '$0B
              byte %01010101   '$0B
              byte %10101010   '$0B
              byte %00000000   '$0B
              byte %00000000   '$0B
              byte %00000000   '$0B
              
              byte %10101010   '$0C
              byte %01010101   '$0C
              byte %10101010   '$0C
              byte %01010101   '$0C
              byte %10101010   '$0C
              byte %00000000   '$0C
              byte %00000000   '$0C
              byte %00000000   '$0C
              
              byte %10101010   '$0D
              byte %01010101   '$0D
              byte %10101010   '$0D
              byte %01010101   '$0D
              byte %10101010   '$0D
              byte %00000000   '$0D
              byte %00000000   '$0D
              byte %00000000   '$0D
              
              byte %10101010   '$0E
              byte %01010101   '$0E
              byte %10101010   '$0E
              byte %01010101   '$0E
              byte %10101010   '$0E
              byte %00000000   '$0E
              byte %00000000   '$0E
              byte %00000000   '$0E
              
              byte %10101010   '$0F
              byte %01010101   '$0F
              byte %10101010   '$0F
              byte %01010101   '$0F
              byte %10101010   '$0F
              byte %00000000   '$0F
              byte %00000000   '$0F
              byte %00000000   '$0F
              
              byte %11111111   '$10
              byte %11111111   '$10
              byte %11111111   '$10
              byte %11111111   '$10
              byte %11111111   '$10
              byte %00000000   '$10
              byte %00000000   '$10
              byte %00000000   '$10
              
              byte %01111110   '$11
              byte %10111101   '$11
              byte %11011011   '$11
              byte %11100111   '$11
              byte %11100111   '$11
              byte %00000000   '$11
              byte %00000000   '$11
              byte %00000000   '$11
              
              byte %11000011   '$12
              byte %11000011   '$12
              byte %11000011   '$12
              byte %11000011   '$12
              byte %11000011   '$12
              byte %00000000   '$12
              byte %00000000   '$12
              byte %00000000   '$12
              
              byte %11111111   '$13
              byte %00000000   '$13
              byte %00000000   '$13
              byte %00000000   '$13
              byte %11111111   '$13
              byte %00000000   '$13
              byte %00000000   '$13
              byte %00000000   '$13
              
              byte %11111111   '$14
              byte %11100111   '$14
              byte %10011001   '$14
              byte %11100111   '$14
              byte %11111111   '$14
              byte %00000000   '$14
              byte %00000000   '$14
              byte %00000000   '$14
              
              byte %11111111   '$15
              byte %11111111   '$15
              byte %10000001   '$15
              byte %10000001   '$15
              byte %11111111   '$15
              byte %00000000   '$15
              byte %00000000   '$15
              byte %00000000   '$15
              
              byte %11111111   '$16
              byte %10000001   '$16
              byte %10000001   '$16
              byte %11111111   '$16
              byte %11111111   '$16
              byte %00000000   '$16
              byte %00000000   '$16
              byte %00000000   '$16
              
              byte %11111111   '$17
              byte %10000001   '$17
              byte %10000001   '$17
              byte %10000001   '$17
              byte %11111111   '$17
              byte %00000000   '$17
              byte %00000000   '$17
              byte %00000000   '$17
              
              byte %11111111   '$18
              byte %10000001   '$18
              byte %10000001   '$18
              byte %10000001   '$18
              byte %11111111   '$18
              byte %00000000   '$18
              byte %00000000   '$18
              byte %00000000   '$18
              
              byte %11111111   '$19
              byte %10000001   '$19
              byte %10000001   '$19
              byte %10000001   '$19
              byte %11111111   '$19
              byte %00000000   '$19
              byte %00000000   '$19
              byte %00000000   '$19
              
              byte %11111111   '$1A
              byte %10000001   '$1A
              byte %10000001   '$1A
              byte %10000001   '$1A
              byte %11111111   '$1A
              byte %00000000   '$1A
              byte %00000000   '$1A
              byte %00000000   '$1A
              
              byte %11111111   '$1B
              byte %10000001   '$1B
              byte %10000001   '$1B
              byte %10000001   '$1B
              byte %11111111   '$1B
              byte %00000000   '$1B
              byte %00000000   '$1B
              byte %00000000   '$1B
              
              byte %11111111   '$1C
              byte %10000001   '$1C
              byte %10000001   '$1C
              byte %10000001   '$1C
              byte %11111111   '$1C
              byte %00000000   '$1C
              byte %00000000   '$1C
              byte %00000000   '$1C
              
              byte %11111111   '$1D
              byte %10000001   '$1D
              byte %10000001   '$1D
              byte %10000001   '$1D
              byte %11111111   '$1D
              byte %00000000   '$1D
              byte %00000000   '$1D
              byte %00000000   '$1D
              
              byte %11111111   '$1E
              byte %10000001   '$1E
              byte %10000001   '$1E
              byte %10000001   '$1E
              byte %11111111   '$1E
              byte %00000000   '$1E
              byte %00000000   '$1E
              byte %00000000   '$1E
              
              byte %11111111   '$1F
              byte %10000001   '$1F
              byte %10000001   '$1F
              byte %10000001   '$1F
              byte %11111111   '$1F
              byte %00000000   '$1F
              byte %00000000   '$1F
              byte %00000000   '$1F
              
              byte %00000000   '$20
              byte %00000000   '$20
              byte %00000000   '$20
              byte %00000000   '$20
              byte %00000000   '$20
              byte %00000000   '$20
              byte %00000000   '$20
              byte %00000000   '$20
              
              byte %01011111   '$21
              byte %00000000   '$21
              byte %00000000   '$21
              byte %00000000   '$21
              byte %00000000   '$21
              byte %00000000   '$21
              byte %00000000   '$21
              byte %00000000   '$21
              
              byte %00000011   '$22
              byte %00000101   '$22
              byte %00000000   '$22
              byte %00000011   '$22
              byte %00000101   '$22
              byte %00000000   '$22
              byte %00000000   '$22
              byte %00000000   '$22
              
              byte %00010100   '$23
              byte %00111110   '$23
              byte %00010100   '$23
              byte %00111110   '$23
              byte %00010100   '$23
              byte %00000000   '$23
              byte %00000000   '$23
              byte %00000000   '$23
              
              byte %00100100   '$24
              byte %00101010   '$24
              byte %01111111   '$24
              byte %00101010   '$24
              byte %00010010   '$24
              byte %00000000   '$24
              byte %00000000   '$24
              byte %00000000   '$24
              
              byte %01100011   '$25
              byte %00010000   '$25
              byte %00001000   '$25
              byte %00000100   '$25
              byte %01100011   '$25
              byte %00000000   '$25
              byte %00000000   '$25
              byte %00000000   '$25
              
              byte %00110110   '$26
              byte %01001001   '$26
              byte %01010110   '$26
              byte %00100000   '$26
              byte %01010000   '$26
              byte %00000000   '$26
              byte %00000000   '$26
              byte %00000000   '$26
              
              byte %00000000   '$27
              byte %00000000   '$27
              byte %00000101   '$27
              byte %00000011   '$27
              byte %00000000   '$27
              byte %00000000   '$27
              byte %00000000   '$27
              byte %00000000   '$27
              
              byte %00000000   '$28
              byte %00000000   '$28
              byte %00011100   '$28
              byte %00100010   '$28
              byte %01000001   '$28
              byte %00000000   '$28
              byte %00000000   '$28
              byte %00000000   '$28
              
              byte %01000001   '$29
              byte %00100010   '$29
              byte %00011100   '$29
              byte %00000000   '$29
              byte %00000000   '$29
              byte %00000000   '$29
              byte %00000000   '$29
              byte %00000000   '$29
              
              byte %00100100   '$2A
              byte %00011000   '$2A
              byte %01111110   '$2A
              byte %00011000   '$2A
              byte %00100100   '$2A
              byte %00000000   '$2A
              byte %00000000   '$2A
              byte %00000000   '$2A
              
              byte %00001000   '$2B
              byte %00001000   '$2B
              byte %00111110   '$2B
              byte %00001000   '$2B
              byte %00001000   '$2B
              byte %00000000   '$2B
              byte %00000000   '$2B
              byte %00000000   '$2B
              
              byte %10100000   '$2C
              byte %01100000   '$2C
              byte %00000000   '$2C
              byte %00000000   '$2C
              byte %00000000   '$2C
              byte %00000000   '$2C
              byte %00000000   '$2C
              byte %00000000   '$2C
              
              byte %00001000   '$2D
              byte %00001000   '$2D
              byte %00001000   '$2D
              byte %00001000   '$2D
              byte %00001000   '$2D
              byte %00000000   '$2D
              byte %00000000   '$2D
              byte %00000000   '$2D
              
              byte %01100000   '$2E
              byte %01100000   '$2E
              byte %00000000   '$2E
              byte %00000000   '$2E
              byte %00000000   '$2E
              byte %00000000   '$2E
              byte %00000000   '$2E
              byte %00000000   '$2E
              
              byte %01100000   '$2F
              byte %00010000   '$2F
              byte %00001000   '$2F
              byte %00000100   '$2F
              byte %00000011   '$2F
              byte %00000000   '$2F
              byte %00000000   '$2F
              byte %00000000   '$2F
              
              byte %00111110   '$30
              byte %01010001   '$30
              byte %01001001   '$30
              byte %01000101   '$30
              byte %00111110   '$30
              byte %00000000   '$30
              byte %00000000   '$30
              byte %00000000   '$30
              
              byte %00000000   '$31
              byte %01000010   '$31
              byte %01111111   '$31
              byte %01000000   '$31
              byte %00000000   '$31
              byte %00000000   '$31
              byte %00000000   '$31
              byte %00000000   '$31
              
              byte %01100010   '$32
              byte %01010001   '$32
              byte %01010001   '$32
              byte %01001001   '$32
              byte %01000110   '$32
              byte %00000000   '$32
              byte %00000000   '$32
              byte %00000000   '$32
              
              byte %00100010   '$33
              byte %01001001   '$33
              byte %01001001   '$33
              byte %01001001   '$33
              byte %00110110   '$33
              byte %00000000   '$33
              byte %00000000   '$33
              byte %00000000   '$33
              
              byte %00011000   '$34
              byte %00010100   '$34
              byte %00010010   '$34
              byte %01111111   '$34
              byte %00010000   '$34
              byte %00000000   '$34
              byte %00000000   '$34
              byte %00000000   '$34
              
              byte %00100111   '$35
              byte %01000101   '$35
              byte %01000101   '$35
              byte %01000101   '$35
              byte %00111001   '$35
              byte %00000000   '$35
              byte %00000000   '$35
              byte %00000000   '$35
              
              byte %00111100   '$36
              byte %01001010   '$36
              byte %01001001   '$36
              byte %01001001   '$36
              byte %00110000   '$36
              byte %00000000   '$36
              byte %00000000   '$36
              byte %00000000   '$36
              
              byte %00000001   '$37
              byte %01110001   '$37
              byte %00001001   '$37
              byte %00000101   '$37
              byte %00000011   '$37
              byte %00000000   '$37
              byte %00000000   '$37
              byte %00000000   '$37
              
              byte %00110110   '$38
              byte %01001001   '$38
              byte %01001001   '$38
              byte %01001001   '$38
              byte %00110110   '$38
              byte %00000000   '$38
              byte %00000000   '$38
              byte %00000000   '$38
              
              byte %00000110   '$39
              byte %01001001   '$39
              byte %01001001   '$39
              byte %00101001   '$39
              byte %00011110   '$39
              byte %00000000   '$39
              byte %00000000   '$39
              byte %00000000   '$39
              
              byte %00110110   '$3A
              byte %00110110   '$3A
              byte %00000000   '$3A
              byte %00000000   '$3A
              byte %00000000   '$3A
              byte %00000000   '$3A
              byte %00000000   '$3A
              byte %00000000   '$3A
              
              byte %10110110   '$3B
              byte %01110110   '$3B
              byte %00000000   '$3B
              byte %00000000   '$3B
              byte %00000000   '$3B
              byte %00000000   '$3B
              byte %00000000   '$3B
              byte %00000000   '$3B
              
              byte %00000000   '$3C
              byte %00001000   '$3C
              byte %00010100   '$3C
              byte %00100010   '$3C
              byte %01000001   '$3C
              byte %00000000   '$3C
              byte %00000000   '$3C
              byte %00000000   '$3C
              
              byte %00010100   '$3D
              byte %00010100   '$3D
              byte %00010100   '$3D
              byte %00010100   '$3D
              byte %00010100   '$3D
              byte %00000000   '$3D
              byte %00000000   '$3D
              byte %00000000   '$3D
              
              byte %01000001   '$3E
              byte %00100010   '$3E
              byte %00010100   '$3E
              byte %00001000   '$3E
              byte %00000000   '$3E
              byte %00000000   '$3E
              byte %00000000   '$3E
              byte %00000000   '$3E
              
              byte %00000010   '$3F
              byte %00000001   '$3F
              byte %01010001   '$3F
              byte %00001001   '$3F
              byte %00000110   '$3F
              byte %00000000   '$3F
              byte %00000000   '$3F
              byte %00000000   '$3F
              
              byte %00111110   '$40
              byte %01000001   '$40
              byte %01011101   '$40
              byte %01010001   '$40
              byte %01001110   '$40
              byte %00000000   '$40
              byte %00000000   '$40
              byte %00000000   '$40
              
              byte %01111100   '$41
              byte %00010010   '$41
              byte %00010001   '$41
              byte %00010010   '$41
              byte %01111100   '$41
              byte %00000000   '$41
              byte %00000000   '$41
              byte %00000000   '$41
              
              byte %01111111   '$42
              byte %01001001   '$42
              byte %01001001   '$42
              byte %01001001   '$42
              byte %00110110   '$42
              byte %00000000   '$42
              byte %00000000   '$42
              byte %00000000   '$42
              
              byte %00011100   '$43
              byte %00100010   '$43
              byte %01000001   '$43
              byte %01000001   '$43
              byte %00100010   '$43
              byte %00000000   '$43
              byte %00000000   '$43
              byte %00000000   '$43
              
              byte %01111111   '$44
              byte %01000001   '$44
              byte %01000001   '$44
              byte %00100010   '$44
              byte %00011100   '$44
              byte %00000000   '$44
              byte %00000000   '$44
              byte %00000000   '$44
              
              byte %01111111   '$45
              byte %01001001   '$45
              byte %01001001   '$45
              byte %01001001   '$45
              byte %01000001   '$45
              byte %00000000   '$45
              byte %00000000   '$45
              byte %00000000   '$45
              
              byte %01111111   '$46
              byte %00001001   '$46
              byte %00001001   '$46
              byte %00001001   '$46
              byte %00000001   '$46
              byte %00000000   '$46
              byte %00000000   '$46
              byte %00000000   '$46
              
              byte %00111110   '$47
              byte %01000001   '$47
              byte %01000001   '$47
              byte %01010001   '$47
              byte %00110010   '$47
              byte %00000000   '$47
              byte %00000000   '$47
              byte %00000000   '$47
              
              byte %01111111   '$48
              byte %00001000   '$48
              byte %00001000   '$48
              byte %00001000   '$48
              byte %01111111   '$48
              byte %00000000   '$48
              byte %00000000   '$48
              byte %00000000   '$48
              
              byte %01000001   '$49
              byte %01000001   '$49
              byte %01111111   '$49
              byte %01000001   '$49
              byte %01000001   '$49
              byte %00000000   '$49
              byte %00000000   '$49
              byte %00000000   '$49
              
              byte %00100000   '$4A
              byte %01000000   '$4A
              byte %01000000   '$4A
              byte %01000000   '$4A
              byte %00111111   '$4A
              byte %00000000   '$4A
              byte %00000000   '$4A
              byte %00000000   '$4A
              
              byte %01111111   '$4B
              byte %00001000   '$4B
              byte %00010100   '$4B
              byte %00100010   '$4B
              byte %01000001   '$4B
              byte %00000000   '$4B
              byte %00000000   '$4B
              byte %00000000   '$4B
              
              byte %01111111   '$4C
              byte %01000000   '$4C
              byte %01000000   '$4C
              byte %01000000   '$4C
              byte %01000000   '$4C
              byte %00000000   '$4C
              byte %00000000   '$4C
              byte %00000000   '$4C
              
              byte %01111111   '$4D
              byte %00000010   '$4D
              byte %00001100   '$4D
              byte %00000010   '$4D
              byte %01111111   '$4D
              byte %00000000   '$4D
              byte %00000000   '$4D
              byte %00000000   '$4D
              
              byte %01111111   '$4E
              byte %00000100   '$4E
              byte %00001000   '$4E
              byte %00010000   '$4E
              byte %01111111   '$4E
              byte %00000000   '$4E
              byte %00000000   '$4E
              byte %00000000   '$4E
              
              byte %00111110   '$4F
              byte %01000001   '$4F
              byte %01000001   '$4F
              byte %01000001   '$4F
              byte %00111110   '$4F
              byte %00000000   '$4F
              byte %00000000   '$4F
              byte %00000000   '$4F
              
              byte %01111111   '$50
              byte %00001001   '$50
              byte %00001001   '$50
              byte %00001001   '$50
              byte %00000110   '$50
              byte %00000000   '$50
              byte %00000000   '$50
              byte %00000000   '$50
              
              byte %00111110   '$51
              byte %01000001   '$51
              byte %01010001   '$51
              byte %00100001   '$51
              byte %01011110   '$51
              byte %00000000   '$51
              byte %00000000   '$51
              byte %00000000   '$51
              
              byte %01111111   '$52
              byte %00001001   '$52
              byte %00011001   '$52
              byte %00101001   '$52
              byte %01000110   '$52
              byte %00000000   '$52
              byte %00000000   '$52
              byte %00000000   '$52
              
              byte %00100110   '$53
              byte %01001001   '$53
              byte %01001001   '$53
              byte %01001001   '$53
              byte %00110010   '$53
              byte %00000000   '$53
              byte %00000000   '$53
              byte %00000000   '$53
              
              byte %00000001   '$54
              byte %00000001   '$54
              byte %01111111   '$54
              byte %00000001   '$54
              byte %00000001   '$54
              byte %00000000   '$54
              byte %00000000   '$54
              byte %00000000   '$54
              
              byte %00111111   '$55
              byte %01000000   '$55
              byte %01000000   '$55
              byte %01000000   '$55
              byte %00111111   '$55
              byte %00000000   '$55
              byte %00000000   '$55
              byte %00000000   '$55
              
              byte %00000111   '$56
              byte %00011000   '$56
              byte %01100000   '$56
              byte %00011000   '$56
              byte %00000111   '$56
              byte %00000000   '$56
              byte %00000000   '$56
              byte %00000000   '$56
              
              byte %00111111   '$57
              byte %01000000   '$57
              byte %00111000   '$57
              byte %01000000   '$57
              byte %00111111   '$57
              byte %00000000   '$57
              byte %00000000   '$57
              byte %00000000   '$57
              
              byte %01100011   '$58
              byte %00010100   '$58
              byte %00001000   '$58
              byte %00010100   '$58
              byte %01100011   '$58
              byte %00000000   '$58
              byte %00000000   '$58
              byte %00000000   '$58
              
              byte %00000011   '$59
              byte %00000100   '$59
              byte %01111000   '$59
              byte %00000100   '$59
              byte %00000011   '$59
              byte %00000000   '$59
              byte %00000000   '$59
              byte %00000000   '$59
              
              byte %01100001   '$5A
              byte %01010001   '$5A
              byte %01001001   '$5A
              byte %01000101   '$5A
              byte %01000011   '$5A
              byte %00000000   '$5A
              byte %00000000   '$5A
              byte %00000000   '$5A
              
              byte %01111111   '$5B
              byte %01111111   '$5B
              byte %01000001   '$5B
              byte %01000001   '$5B
              byte %01000001   '$5B
              byte %00000000   '$5B
              byte %00000000   '$5B
              byte %00000000   '$5B
              
              byte %00000011   '$5C
              byte %00000100   '$5C
              byte %00001000   '$5C
              byte %00010000   '$5C
              byte %01100000   '$5C
              byte %00000000   '$5C
              byte %00000000   '$5C
              byte %00000000   '$5C
              
              byte %01000001   '$5D
              byte %01000001   '$5D
              byte %01000001   '$5D
              byte %01111111   '$5D
              byte %01111111   '$5D
              byte %00000000   '$5D
              byte %00000000   '$5D
              byte %00000000   '$5D
              
              byte %00010000   '$5E
              byte %00001000   '$5E
              byte %00000100   '$5E
              byte %00001000   '$5E
              byte %00010000   '$5E
              byte %00000000   '$5E
              byte %00000000   '$5E
              byte %00000000   '$5E
              
              byte %10000000   '$5F
              byte %10000000   '$5F
              byte %10000000   '$5F
              byte %10000000   '$5F
              byte %10000000   '$5F
              byte %00000000   '$5F
              byte %00000000   '$5F
              byte %00000000   '$5F
              
              byte %00000000   '$60
              byte %00000000   '$60
              byte %00000110   '$60
              byte %00000101   '$60
              byte %00000000   '$60
              byte %00000000   '$60
              byte %00000000   '$60
              byte %00000000   '$60
              
              byte %00100000   '$61
              byte %01010100   '$61
              byte %01010100   '$61
              byte %01010100   '$61
              byte %01111000   '$61
              byte %00000000   '$61
              byte %00000000   '$61
              byte %00000000   '$61
              
              byte %01111111   '$62
              byte %01000100   '$62
              byte %01000100   '$62
              byte %01000100   '$62
              byte %00111000   '$62
              byte %00000000   '$62
              byte %00000000   '$62
              byte %00000000   '$62
              
              byte %00111000   '$63
              byte %01000100   '$63
              byte %01000100   '$63
              byte %01000100   '$63
              byte %01000100   '$63
              byte %00000000   '$63
              byte %00000000   '$63
              byte %00000000   '$63
              
              byte %00111000   '$64
              byte %01000100   '$64
              byte %01000100   '$64
              byte %01000100   '$64
              byte %01111111   '$64
              byte %00000000   '$64
              byte %00000000   '$64
              byte %00000000   '$64
              
              byte %00111000   '$65
              byte %01010100   '$65
              byte %01010100   '$65
              byte %01010100   '$65
              byte %01011000   '$65
              byte %00000000   '$65
              byte %00000000   '$65
              byte %00000000   '$65
              
              byte %00001000   '$66
              byte %01111110   '$66
              byte %00001001   '$66
              byte %00001001   '$66
              byte %00000010   '$66
              byte %00000000   '$66
              byte %00000000   '$66
              byte %00000000   '$66
              
              byte %00011000   '$67
              byte %10100100   '$67
              byte %10100100   '$67
              byte %10100100   '$67
              byte %01111000   '$67
              byte %00000000   '$67
              byte %00000000   '$67
              byte %00000000   '$67
              
              byte %01111111   '$68
              byte %00000100   '$68
              byte %00000100   '$68
              byte %00000100   '$68
              byte %01111000   '$68
              byte %00000000   '$68
              byte %00000000   '$68
              byte %00000000   '$68
              
              byte %00000000   '$69
              byte %01000100   '$69
              byte %01111101   '$69
              byte %01000000   '$69
              byte %00000000   '$69
              byte %00000000   '$69
              byte %00000000   '$69
              byte %00000000   '$69
              
              byte %01000000   '$6A
              byte %10000000   '$6A
              byte %10000100   '$6A
              byte %01111101   '$6A
              byte %00000000   '$6A
              byte %00000000   '$6A
              byte %00000000   '$6A
              byte %00000000   '$6A
              
              byte %01101111   '$6B
              byte %00010000   '$6B
              byte %00010000   '$6B
              byte %00101000   '$6B
              byte %01000100   '$6B
              byte %00000000   '$6B
              byte %00000000   '$6B
              byte %00000000   '$6B
              
              byte %00000000   '$6C
              byte %01000001   '$6C
              byte %01111111   '$6C
              byte %01000000   '$6C
              byte %00000000   '$6C
              byte %00000000   '$6C
              byte %00000000   '$6C
              byte %00000000   '$6C
              
              byte %01111100   '$6D
              byte %00000100   '$6D
              byte %00111000   '$6D
              byte %00000100   '$6D
              byte %01111100   '$6D
              byte %00000000   '$6D
              byte %00000000   '$6D
              byte %00000000   '$6D
              
              byte %01111100   '$6E
              byte %00000100   '$6E
              byte %00000100   '$6E
              byte %00000100   '$6E
              byte %01111000   '$6E
              byte %00000000   '$6E
              byte %00000000   '$6E
              byte %00000000   '$6E
              
              byte %00111000   '$6F
              byte %01000100   '$6F
              byte %01000100   '$6F
              byte %01000100   '$6F
              byte %00111000   '$6F
              byte %00000000   '$6F
              byte %00000000   '$6F
              byte %00000000   '$6F
              
              byte %11111100   '$70
              byte %00100100   '$70
              byte %00100100   '$70
              byte %00100100   '$70
              byte %00011000   '$70
              byte %00000000   '$70
              byte %00000000   '$70
              byte %00000000   '$70
              
              byte %00011000   '$71
              byte %00100100   '$71
              byte %00100100   '$71
              byte %00100100   '$71
              byte %11111100   '$71
              byte %00000000   '$71
              byte %00000000   '$71
              byte %00000000   '$71
              
              byte %01111100   '$72
              byte %00001000   '$72
              byte %00000100   '$72
              byte %00000100   '$72
              byte %00000100   '$72
              byte %00000000   '$72
              byte %00000000   '$72
              byte %00000000   '$72
              
              byte %01001000   '$73
              byte %01010100   '$73
              byte %01010100   '$73
              byte %01010100   '$73
              byte %00100100   '$73
              byte %00000000   '$73
              byte %00000000   '$73
              byte %00000000   '$73
              
              byte %00000100   '$74
              byte %00111111   '$74
              byte %01000100   '$74
              byte %01000100   '$74
              byte %00100000   '$74
              byte %00000000   '$74
              byte %00000000   '$74
              byte %00000000   '$74
              
              byte %00111100   '$75
              byte %01000000   '$75
              byte %01000000   '$75
              byte %00100000   '$75
              byte %01111100   '$75
              byte %00000000   '$75
              byte %00000000   '$75
              byte %00000000   '$75
              
              byte %00011100   '$76
              byte %00100000   '$76
              byte %01000000   '$76
              byte %00100000   '$76
              byte %00011100   '$76
              byte %00000000   '$76
              byte %00000000   '$76
              byte %00000000   '$76
              
              byte %01111100   '$77
              byte %01000000   '$77
              byte %00110000   '$77
              byte %01000000   '$77
              byte %01111100   '$77
              byte %00000000   '$77
              byte %00000000   '$77
              byte %00000000   '$77
              
              byte %01000100   '$78
              byte %00101000   '$78
              byte %00010000   '$78
              byte %00101000   '$78
              byte %01000100   '$78
              byte %00000000   '$78
              byte %00000000   '$78
              byte %00000000   '$78
              
              byte %00011100   '$79
              byte %10100000   '$79
              byte %10100000   '$79
              byte %10100000   '$79
              byte %01111100   '$79
              byte %00000000   '$79
              byte %00000000   '$79
              byte %00000000   '$79
              
              byte %01000100   '$7A
              byte %01100100   '$7A
              byte %01010100   '$7A
              byte %01001100   '$7A
              byte %01000100   '$7A
              byte %00000000   '$7A
              byte %00000000   '$7A
              byte %00000000   '$7A
              
              byte %00001000   '$7B
              byte %00111110   '$7B
              byte %01110111   '$7B
              byte %01000001   '$7B
              byte %01000001   '$7B
              byte %00000000   '$7B
              byte %00000000   '$7B
              byte %00000000   '$7B
              
              byte %00000000   '$7C
              byte %00000000   '$7C
              byte %11111111   '$7C
              byte %00000000   '$7C
              byte %00000000   '$7C
              byte %00000000   '$7C
              byte %00000000   '$7C
              byte %00000000   '$7C
              
              byte %01000001   '$7D
              byte %01000001   '$7D
              byte %01110111   '$7D
              byte %00111110   '$7D
              byte %00001000   '$7D
              byte %00000000   '$7D
              byte %00000000   '$7D
              byte %00000000   '$7D
              
              byte %00000100   '$7E
              byte %00000010   '$7E
              byte %00000110   '$7E
              byte %00000100   '$7E
              byte %00000010   '$7E
              byte %00000000   '$7E
              byte %00000000   '$7E
              byte %00000000   '$7E
              
              byte %11111111   '$7F
              byte %11111111   '$7F
              byte %11111111   '$7F
              byte %11111111   '$7F
              byte %11111111   '$7F
              byte %00000000   '$7F
              byte %00000000   '$7F
              byte %00000000   '$7F

{{
┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                   TERMS OF USE: MIT License                                                  │                                                            
├──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    │ 
│files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    │
│modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software│
│is furnished to do so, subject to the following conditions:                                                                   │
│                                                                                                                              │
│The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.│
│                                                                                                                              │
│THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          │
│WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         │
│COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   │
│ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         │
└──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
}}