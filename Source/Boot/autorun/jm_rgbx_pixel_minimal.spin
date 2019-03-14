'' =================================================================================================
''
''   File....... jm_rgbx_pixel_minimal.spin
''
''********************************************
''
'' WARNING!
''
'' This version has been stripped down to
'' minimal functionallity for BadgeWX
'' to help speed up load time
''
'' Modifications not made by original author
''
'' Use of this code not recommended and not supported!
''
''********************************************


''   Original Author..... Jon "JonnyMac" McPhalen
''               Copyright (C) 2016 Jon McPhalen
''               -- see below for terms of use
''   E-mail..... jon@jonmcphalen.com
''   Started.... 
''   Updated.... 13 OCT 2016
''               -- consolodated for 24- and 32-bit pixels
''               -- updated WS2812b timing and removed swap flag from standard start methods
''
'' =================================================================================================
   
  

var

  
  long  p_pixels                                                ' pointer to active pixel buffer
  long  npixels                                                 ' number of pixels in buffer
  long  tpin                                                    ' active transmit pin

  ' do not modify order; this structure passed to PASM cog
  '
  long  connection                                              ' compressed connection details
  long  resetticks                                              ' ticks in reset period
  long  rgfix                                                   ' swap r&g? + bit count for pixels
  long  t0h                                                     ' bit0 high time (ticks)      
  long  t0l                                                     ' bit0 low time
  long  t1h                                                     ' bit1 high time
  long  t1l                                                     ' bit1 low time
                                                 


 
pub startx(p_buf, count, pin, holdoff, rgswap, bits, ns0h, ns0l, ns1h, ns1l) | ustix             

'' Start smart pixel driver driver
'' -- p_buf is pointer to [long] array holding pixel data
'' -- count is # of pixels supported by array at p_buf
'' -- pin is serial output to pixels
'' -- holdoff is the delay between data bursts
''    * units are 100us (0.1ms) 10 units = 1ms
'' -- rgswap is red/green swap flag
'' -- bits is 24 for RGB (WS2812x, SK6812), 32 for RBGW (SK6812RGBW)
'' -- ns0h is 0-bit high timing (ns)
'' -- ns0l is 0-bit low timing (ns)
'' -- ns1h is 1-bit high timing (ns)
'' -- ns1l is 1-bit low timing (ns)

  'stop                                                          ' stop if running
  dira[pin] := 0                                                ' clear tx pin in this cog
                                                                 
  ustix := clkfreq / 1_000_000                                  ' ticks in 1us
                                                                 
  ' set cog parameters

  use(p_buf, count, pin, bits)                                  ' set connection details
  
  resetticks    := ustix * 100 * (1 #> holdoff <# 200)          ' note: 80us min reset timing
  rgfix         := rgswap <> 0                                  ' promote non-zero to true
  t0h           := ustix * ns0h / 1000                          ' set pulse timing values
  t0l           := ustix * ns0l / 1000                              
  t1h           := ustix * ns1h / 1000                              
  t1l           := ustix * ns1l / 1000                              
                                                                 
  cognew(@pixdriver, @connection)                               ' start the cog

  

pub use(p_buf, count, pin, bits) | c

'' Assigns buffer at p_buf to pixel driver
'' -- p_buf is pointer to long array
'' -- count is # of elements in the array 
'' -- pin is serial output to pixel string
'' -- bits is bit count for pixel type (24 or 32)

   longmove(@p_pixels, @p_buf, 3)
   
   
   npixels := count

   c := p_pixels | ((npixels-1) << 16) | (pin << 26)            ' compress for driver cog

   connection := c                                              ' set new connection


dat { auto-run driver } 

                        org     0

pixdriver               mov     t1, par                         ' hub address of parameters -> t1
                        movd    :read, #connect                 ' location of cog parameters -> :read(dest)
                        mov     t2, #7                          ' get 7 parameters
:read                   rdlong  0-0, t1                         ' copy parameter from hub to cog
                        add     t1, #4                          ' next hub element
                        add     :read, INC_DEST                 ' next cog element                         
                        djnz    t2, #:read                      ' done?
                        
setup                   mov     p_hub, connect                  ' extract pointer to pixel array         
                        shl     p_hub, #16
                        shr     p_hub, #16

                        mov     pixcount, connect               ' extract/fix pixel count
                        shl     pixcount, #6                    ' (remove bits flag, tx pin)
                        shr     pixcount, #22                   ' align, 0..MAX_PIXELS-1
                        add     pixcount, #1                    ' fix, 1..MAX_PIXELS

                        mov     t1, connect                     ' extract pin
                        shl     t1, #1                          ' (remove bits flag)
                        shr     t1, #27                         ' align, 0..31
                        mov     txmask, #1                      ' create mask for tx
                        shl     txmask, t1                    
                        andn    outa, txmask                    ' set to output low
                        or      dira, txmask                     

                        mov     pixelbits, #24                  ' assume 24-bit pixels
                        rcl     connect, #1             wc, nr  ' check bit 31
        if_c            add     pixelbits, #8                   ' if set add 8 for 32-bit pixels

                        mov     t1, #0
                        wrlong  t1, par                         ' tell hub we have connection
                        
rgbx_main               rdlong  connect, par            wz      ' check connection
        if_nz           jmp     #setup                            
                                                                 
                        mov     addr, p_hub                     ' point to rgbbuf[0]
                        mov     npix, pixcount                  ' set # active pixels
                                                                 
frame_loop              rdlong  colorbits, addr                 ' read a channel
                        add     addr, #4                        ' point to next
                        tjz     swapflag, #shift_out            ' skip fix if swap = 0   

' Correct placement of color bytes                    
' -- $RR_GG_BB_WW --> $GG_RR_BB_WW                               
                                                                 
fix_colors              mov     t1, colorbits                   ' copy for red
                        mov     t2, colorbits                   ' copy for green
                        and     colorbits, HX_0000FFFF          ' isolate blue and white
                        and     t1, HX_FF000000                 ' isolate red
                        shr     t1, #8                          ' move red from byte3 to byte2
                        or      colorbits, t1                   ' add red back in 
                        and     t2, HX_00FF0000                 ' isolate green
                        shl     t2, #8                          ' move green from byte2 to byte3
                        or      colorbits, t2                   ' add green back in 

shift_out               mov     nbits, pixelbits                ' set for pixel used
:loop                   rcl     colorbits, #1           wc      ' msb --> C
        if_c            mov     bittimer, bit1hi                ' set bit timing  
        if_nc           mov     bittimer, bit0hi                 
                        or      outa, txmask                    ' tx line 1  
                        add     bittimer, cnt                   ' sync bit timer  
        if_c            waitcnt bittimer, bit1lo                 
        if_nc           waitcnt bittimer, bit0lo                 
                        andn    outa, txmask                    ' tx line 0             
                        waitcnt bittimer, #0                    ' hold while low
                        djnz    nbits, #:loop                   ' next bit
                        djnz    npix, #frame_loop               ' done with all leds?                     

reset_delay             mov     bittimer, resettix              ' set reset timing  
                        add     bittimer, cnt                   ' sync timer 
                        waitcnt bittimer, #0                    ' let timer expire 
                                       
                        jmp     #rgbx_main                      ' back to top

' --------------------------------------------------------------------------------------------------

INC_DEST                long    1 << 9                          ' to increment D field

HX_0000FFFF             long    $0000FFFF                       ' byte masks
HX_00FF0000             long    $00FF0000                         
HX_FF000000             long    $FF000000

connect                 res     1                               ' packed connection details
resettix                res     1                               ' frame reset timing
swapflag                res     1                               ' if !0, swap R & G
bit0hi                  res     1                               ' bit0 high timing
bit0lo                  res     1                               ' bit0 low timing
bit1hi                  res     1                               ' bit1 high timing    
bit1lo                  res     1                               ' bit1 low timing

p_hub                   res     1                               ' pointer to pixel buffer in use                              
pixcount                res     1                               ' # pixels in buffer                                 
txmask                  res     1                               ' mask for output pin

pixelbits               res     1                               ' bits per pixel                                                                
bittimer                res     1                               ' timer for reset/bit
addr                    res     1                               ' address of current rgbw pixel
npix                    res     1                               ' # of pixels to process
colorbits               res     1                               ' rgbw for current pixel
nbits                   res     1                               ' # of bits to process
                                                                 
t1                      res     1                               ' work vars
t2                      res     1                                
t3                      res     1                                
                                                                 
                        fit     496                                   
                                                                 
                        
dat { license }

{{

  Copyright (C) 2016 Jon McPhalen  

  Terms of Use: MIT License

  Permission is hereby granted, free of charge, to any person obtaining a copy of this
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify,
  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be included in all copies
  or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

}}  