/*
* Parallax BadgeWX (#20300, Rev A)
*
* BadgeWX_PowerT10.c
*
* Code for Attiny10 power control uC
* To be installed by Parallax Manufacturing
*
* Created: 2018.02.25. 3:56:03 PM
* Updated: 2018.05.17.
* Author : Michael Mulholland
*/


// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
// TODO: SETTINGS TO FIGURE OUT BEFORE PRODUCTION RELEASE
//
// 1. FUSE CODE PROTECT BITS !!!
// 2. Watchdog
// 3. Brownout
//
// 4. Adjust, remove or keep feedback led flashes, which I've used during development to indicate button press counts
//
// 5. Could add sleep mode, with ATTiny waking up on button press. Do later if customers have battery issues.
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
// OUTSTANDING BUGS:
//
// - none known!
//
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************


// Version History
//
// 2013.04.16. Slow down "loading" flashes a bit
//			   Add loading flashes to reset cycle (so power-up behaviour is consistent)	
//				
//
//
// 2013.04.16. Add code to prevent autorun.bin being loaded when starting badge in STA/AP recovery mode
//				- refer to 'todo' note in goWiFiAPSTAMode() 	   
//
// 
// 2013.04.15. Move millis temp var (m) to volatile global. wait() was failing in wifi recovery mode.
//			   Remove millisReset() and replace with initTimer instead. Ensures interrupts always re-started at every call.
// 
//
// 2013.04.14. Add about 2 seconds of led flashes whilst powering up, so user "sees" something until "HELLO, Loading" message appears on OLED 
//
// 
// 2013.04.01. Add WiFi program override, on IO14 (hold low to allow programming over WiFi regardless of IO14)
//				- Note: Not intended to advertise this feature, and probably should remove before production deployment- but jolly handy to have during test!
//
// 2013.03.28. Initial release


#define CLK1MHZ // CLK8MHZ
#define ENABLE_CODE
#define ENABLE_IO14_OVERRIDE

#ifdef CLK1MHZ 

	#define F_CPU 1000000UL // 1MHz
	
#endif

#ifdef CLK8MHZ

	//#define F_CPU 8000000UL // 8MHz

#endif


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


/*
*
* PIN Settings (WARNING! REV A or MODIFIED REV A2 ONLY !!!)
*
* PB0 - WX_Configure (Input NO pullup, external 10K PUP to 3V3 inside WROOM module. Set output low to set WiFi bootloader mode)
* PB1 - Pwr_Switch   (Input pulled up, or output low to flash pwr LED)
* PB2 - Pwr_Enable   (Input NO pullup, external 10K PUP to BATT-5V, up=badge off, output low=badge on, connected to PFET gate)
* PB3 - Prop_RESn	(Input pulled up by default - leave it alone! Do not use this pin in this version!)
*
* Care with PB0!  PB0 is ~5V, and connects to WX_Configure which is 3.3V, and is already pulled-up at the WiFi module.
*
*/


#define WX_CONFIGURE                PB0
#define WX_CONFIGURE_PORT           PORTB
#define WX_CONFIGURE_DDR            DDRB
#define WX_CONFIGURE_PIN            PINB
#define WX_CONFIGURE_PUE            PUEB

#define PWR_SWITCH	                PB1
#define PWR_SWITCH_PORT				PORTB
#define PWR_SWITCH_DDR				DDRB
#define PWR_SWITCH_PIN				PINB
#define PWR_SWITCH_PUE				PUEB

#define PWR_ENABLE	                PB2
#define PWR_ENABLE_PORT		        PORTB
#define PWR_ENABLE_DDR	            DDRB
#define PWR_ENABLE_PIN	            PINB
#define PWR_ENABLE_PUE	            PUEB

#define PROP_RESN	                PB3
#define PROP_RESN_PORT				PORTB
#define PROP_RESN_DDR				DDRB
#define PROP_RESN_PIN	            PINB
#define PROP_RESN_PUE	            PUEB


/***** Configure IO *****/
#define configure_as_input(bit)             {bit ## _DDR &= ~(1 << bit);}
#define configure_as_output(bit)            {bit ## _DDR |= (1 << bit);}

/***** Manipulate Pull-up resistors *****/
#define pullup_on(bit)                      {bit ## _PORT |= (1 << bit);}    // Suitable for most AVR's
#define pullup_off(bit)                     {bit ## _PORT &= ~(1 << bit);}

#define pullup_enable(bit)                  {bit ## _PUE |= (1 << bit);}    // Suitable for Attiny10 series	
#define pullup_disable(bit)                 {bit ## _PUE &= ~(1 << bit);}

/***** Manipulate Outputs *****/
#define set_high(bit)                       {bit ## _PORT |= (1 << bit);}
#define set_low(bit)                        {bit ## _PORT &= ~(1 << bit);}
#define toggle(bit)                         {bit ## _PIN |= (1 << bit);}

/***** Test Inputs *****/
#define is_high(bit)                        (bit ## _PIN & (1 << bit))
#define is_low(bit)                         (! (bit ## _PIN & (1 << bit)))

#define is_porthigh(bit)                    (bit ## _PORT & (1 << bit))
#define is_portlow(bit)                     (! (bit ## _PORT & (1 << bit)))

// NOP (Required delay when changing port to output, before reading state)
#define ASM_NOP() asm volatile ("nop" :: )


#define PRESSED 0		// Switch input driven low by switch when pressed
#define NOT_PRESSED 1	// Switch input pulled high (internal pull-up) whilst not pressed

//#define FALSE 0
//#define TRUE (!FALSE)

#define PWR_OFF 0
#define PWR_ON 1

#define OK 0 // Sub function returned OK
#define KO 1 // Sub function returned with error - KnockOut !


// *******************************************************************************************************************************
//
// Function headers

void init_millis_timer();
uint16_t millis();
void wait(uint16_t ms);
void setClock();
void set_badgepwr_on();
void set_badgepwr_off();
void set_pwrled_on();
void set_pwrled_off();
void blink_pwrled_slow(uint8_t reps);
void blink_pwrled_fast(uint8_t reps);
void blink_pwrled_pwron(uint8_t reps);


// *******************************************************************************************************************************



// Global vars

volatile uint16_t _milliseconds; // NOTE: Reset every main-loop, so keeping to int16 (65535 max button press, ~60 seconds)
volatile uint16_t _target_ms;
volatile uint16_t m;

// *******************************************************************************************************************************


uint16_t millis()
{
		
	cli();
	m = _milliseconds;
	sei();
	
	return m;
}



/*inline void millisReset()
{
	cli();
	_milliseconds = 0;
	sei();
	
}*/

void wait(uint16_t ms) {
	
	//millisReset();
	init_millis_timer(); // Temp fix to the interrupt stopping unexpectedly
	
	
	_target_ms = millis() + ms; // CARE: _target_ms is uint16; sum of two uint16's might exceed _target size unless millis reset used carefully!
	while (_target_ms > millis()) { }
		
}

// *******************************************************************************************************************************




void setClock()
{

// Set CPU speed by setting clock prescalar:
// CCP register must first be written with the correct signature - 0xD8
CCP = 0xD8;

//  CLKPS[3:0] sets the clock division factor

#ifdef CLK1MHZ

	CLKPSR = 0b00000011; // 0011 (/8) for 1MHz

#endif


#ifdef CLK8MHZ

	CLKPSR = 0b00000000; // 0000 (/1) for 8MHz

#endif


}

// *******************************************************************************************************************************


inline void set_badgepwr_on()
{
	// * PB2 - Pwr_Enable   (Output high=badge off, output low=badge on, connected to PFET gate)
	
	
	// Output, low (Will drive P-FET Gate low, and switch it on)
	set_low(PWR_ENABLE);
	configure_as_output(PWR_ENABLE);
	
	ASM_NOP(); // NOP before immediately reading state in later code, or might get wrong value
		
}


inline void set_badgepwr_off()
{
	// * PB2 - Pwr_Enable   (Input=badge off, output low=badge on, connected to PFET gate with external 10K gate PUP)
	
	
	// Input, tri-state (external pull-up will switch P_FET off)
	
	set_high(PWR_ENABLE);
	configure_as_input(PWR_ENABLE);
	
	// Always reset WX_Configure to input, so WiFi module doesn't accidentally start in PGM-Firmware mode
	configure_as_input(WX_CONFIGURE); // External pullup as WiFi is 3V3! Do NOT drive HIGH!
	
	
	ASM_NOP(); // NOP before immediately reading state in later code, or might get wrong value
			
}


// *******************************************************************************************************************************


inline void set_pwrled_on()
{
	
	// * PB1 - Pwr_Switch shared with PWR LED (Input pulled up, or output low to flash pwr LED)
	
	// Disable pull-up, then set pin output, low
		
	pullup_disable(PWR_SWITCH);
	set_low(PWR_SWITCH);
	configure_as_output(PWR_SWITCH);
	
}

inline void set_pwrled_off()
{
	// * PB1 - Pwr_Switch shared with PWR LED (Input pulled up, or output low to flash pwr LED)
	
	// Set pin input, and enable pull-up
	
	configure_as_input(PWR_SWITCH);
	pullup_on(PWR_SWITCH);
	pullup_enable(PWR_SWITCH);
	
}


void blink_pwrled_slow(uint8_t reps)
{
	
	cli();
	
	while (reps > 0) {
		
		set_pwrled_on();
		_delay_ms(175);
		
		set_pwrled_off();
		if (reps>1) { _delay_ms(175); } // Don't wait on last off
		
		reps--;
		
	}
	
	sei();
	
}

void blink_pwrled_fast(uint8_t reps)
{
	cli();
	
	while (reps > 0) {
		
		set_pwrled_on();
		_delay_ms(100);
		
		set_pwrled_off();
		if (reps>1) { _delay_ms(100); } // Don't wait on last off
	
		reps--;
		
	}
		
	sei();
}

void blink_pwrled_pwron(uint8_t reps)
{
	cli();
	
	while (reps > 0) {
		
		set_pwrled_on();
		_delay_ms(200);
		
		set_pwrled_off();
		if (reps>1) { _delay_ms(200); } // Don't wait on last off
		
		reps--;
		
	}
	
	sei();
}

// *******************************************************************************************************************************





// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************



			
// Settings and stats for the power button
typedef struct Button {
			
	uint8_t debounce_ms; // Individual button press must be longer than this. Keep low- 20 seems good.
	uint8_t timeout_ms;  // Time to wait for multiple key presses (from end of each press). 150-250 seems good.
										
	volatile uint8_t currentState;
						
	volatile uint8_t pressCount;
	volatile uint16_t lastHold_ms;
			
} Button;
		
		
// Initialize the button
Button button = {10,200,0,0};
		
		
		
// -------------------------------------------------------------------------------------
// Timer and interrupt helpers to keep time (milliseconds)
//
// call millis() to get latest millisecond count since device started or reset
// 
// TODO: handle milliseconds wrap (long var)
// -------------------------------------------------------------------------------------
		
inline void init_millis_timer()
{
			
	cli();
			
	_milliseconds=0;
			
	// Set timer for 1 Hz rate
	OCR0A = F_CPU / 1000 -1;
			
	// Enable COMPA interrupt
	TIMSK0 = 1<<OCIE0A;
			
	// Start timer in CTC mode with scale 1:1
	TCCR0A = 0b00000000;
	TCCR0B = 0b00001001;
			
	// Enable global interrupts
	sei();
			
}
		
		
ISR(TIM0_COMPA_vect)
{

	_milliseconds++;
			
		
}
		
		
		
		
// -------------------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------------------

		
volatile inline uint8_t GetButtonState() {
			
	return is_high(PWR_SWITCH) ? NOT_PRESSED : PRESSED;
	//return (PINB & (1 << PWR_SWITCH_PIN)) ? NOT_PRESSED : PRESSED;
			
}
		

// -------------------------------------------------------------------------------------
		
		
		
volatile inline uint8_t GetPowerState() {
			
	// Note: Deliberately checking PIN instead of PORT here
	//       as we want the actual PIN state, not what we have set the port to.
	//       That's because the FET gate is pulled up, and we want to act based on the external trace state
			
	//return is_high(PWR_ENABLE) ? PWR_OFF : PWR_ON;
			
	// Scratch that- set the port when changing state, so read port here too (stability and will still work if we ditch the external pup)
	return is_porthigh(PWR_ENABLE) ? PWR_OFF : PWR_ON; //return (PORTB & (1 << PWR_ENABLE)) ? PWR_OFF : PWR_ON;
			
}
		

// -------------------------------------------------------------------------------------

		
		
volatile inline uint8_t waitwhilepressed(uint16_t ms) {
			
	init_millis_timer(); // fix to the interrupt stopping unexpectedly
	
	
	_target_ms = millis() + ms; //; Define as uint32; sum of two uint16's
						
	while (_target_ms > millis()) {
				
		if (GetButtonState() == NOT_PRESSED) {
					
			return KO; // return immediately with error flag
					
		}
				
				
	}
					
	return OK;
			
}
		
		
		
volatile inline uint8_t waituntilpressed(uint16_t ms) {
			
	init_millis_timer(); // fix to the interrupt stopping unexpectedly
	
	
	_target_ms = millis() + ms; //; Define as uint32; sum of two uint16's
			
	while (_target_ms > millis()) {
				
		if (GetButtonState() == PRESSED) {
					
			return OK; // return immediately with OK flag
					
		}
				
				
	}
			
	return KO; // timeout flag
			
}
		
		
		
// -------------------------------------------------------------------------------------
				

#define BLINK_WAIT 500
		
inline void blinkParallaxTechTel() {
			
	blink_pwrled_fast(8);
	wait(BLINK_WAIT);
	blink_pwrled_slow(8);
	wait(BLINK_WAIT);
	blink_pwrled_fast(8);
	wait(BLINK_WAIT);
			
	blink_pwrled_slow(9);
	wait(BLINK_WAIT);
	blink_pwrled_fast(9);
	wait(BLINK_WAIT);
	blink_pwrled_slow(7);
	wait(BLINK_WAIT);
			
	blink_pwrled_fast(8);
	wait(BLINK_WAIT);
	blink_pwrled_slow(2);
	wait(BLINK_WAIT);
	blink_pwrled_fast(6);
	wait(BLINK_WAIT);
	blink_pwrled_slow(7);
	wait(BLINK_WAIT);
			
}
		
		
// -------------------------------------------------------------------------------------
		

inline void goWiFiRecoveryMode() {
			
	// WiFi recovery mode
	// Flash led, and drive WX_Config low so WiFi enters bootloader mode
			
	// Flash pwr led 3 times - feedback
	blink_pwrled_fast(8);
			
			
	// Drive the WiFi module pin to AP recovery mode
	
	set_low(WX_CONFIGURE);
	configure_as_output(WX_CONFIGURE);
				
	wait(100);
			
			
	// Power on badge
	set_badgepwr_on();
			
			
	// Wait for 400mSeconds for WiFi module to power-up and realize that WX is low
	// TODO: Delay period guessed ... validate!
	wait(400);
			
			
	// Revert the WiFi bootloader pin to input
	configure_as_input(WX_CONFIGURE);
			
			
	// Flash pwr led 1 more time - feedback
	blink_pwrled_slow(1);
			
			
	// WiFi module should now be active, in recovery mode- so AP activated.
			
	// TODO: Does having IO14 pulled low restrict access to recovery?
			
	// TODO: ESP-Firmware needs to ignore IO14 is WX low
	// TODO: After that... consider flashing LED here in loop (AP_RECOVERY_MODE), until button held
			
	// .... ok, try this:
	// Blink led until next button press, to indicate AP recovery mode active
			
	while (waituntilpressed(1000) == KO) {
				
		blink_pwrled_fast(2);
				
	}
			
			
	// Power off badge immediately
	set_badgepwr_off();
			
	// Wait until button no longer pressed (ensure main loop doesn't get confused by held button)
	while (GetButtonState() == PRESSED) { }
				
	wait(500);
			
}
		
		
// -------------------------------------------------------------------------------------
		

inline void goWiFiAPSTAMode() {
	
	// TODO: If special badge firmware becomes necessary, we could have the 4-press detect routine
	//       autorun a special bin which displays "Recovery Mode" on the OLED... any maybe the module name and IP address too !
	
			
	// WiFi STA+AP mode
	// Flash led, and pulse WX_Config low 4 times to enter mode
			
	
	// Ensure wifi module is powered on
	set_badgepwr_on();
	
	// Must hold GPIO0 low for a moment, to prevent any autorun.bin code being run!
	// allow 100-300ms for wifi module to start up in "flash" mode first though :)
	
	wait(100); // TODO: What is the optimal delay here?
	//_delay_ms(1);
	set_low(WX_CONFIGURE);
	configure_as_output(WX_CONFIGURE);
		
	// Flash pwr led 3 times - feedback
	blink_pwrled_slow(4);
		
	// Wait long enough so that Parallax-ESP code has already got passed where it would do the autorun	
	wait(2000); // TODO: What is the minimum delay here?
	
	configure_as_input(WX_CONFIGURE); // External pullup as WiFi is 3V3! Do NOT drive HIGH!
	wait(100);
	
	// Now proceed to pulse WX_Config 4 times, to enter recovery mode
	
	
	// Pulse WX_Configure low 4 times, to enter STA+AP mode
	//configure_as_output(WX_CONFIGURE);
	//ASM_NOP();
			
	for(volatile uint8_t i=0;i<4;i++) {
				
		if (i>0) { wait(150); }
			
		set_low(WX_CONFIGURE);
		configure_as_output(WX_CONFIGURE);
		
		wait(150);
		
		configure_as_input(WX_CONFIGURE); // External pullup as WiFi is 3V3! Do NOT drive HIGH!
				
	}
	
	
	// Flash pwr led 1 more time - feedback
	blink_pwrled_slow(1);
	
	
	// Set WX_Config low to disable badge protection (IO14 protection override)
	
	wait(500); // TODO: What is the optimal delay here?
	set_low(WX_CONFIGURE);
	configure_as_output(WX_CONFIGURE);
	
			
	// Flash pwr led 1 more time - feedback
	blink_pwrled_slow(1);
			
			
	// WiFi module should now be active, with STA+AP wireless more activated.
			
	// TODO: Does having IO14 pulled low restrict access to recovery?
	// TODO: ESP-Firmware needs to ignore IO14 is WX low
			
	// TODO: After that... consider flashing LED here in loop (AP_RECOVERY_MODE), until button held
			
	// .... ok, try this:
	// Blink led until next button press, to indicate AP recovery mode active
			
	while (waituntilpressed(1000) == KO) {
				
		blink_pwrled_fast(4);
		
		init_millis_timer(); // Temp fix to the interrupt stopping unexpectedly
				
	}
			
			
	// Power off badge immediately
	set_badgepwr_off();
			
	// Wait until button no longer pressed (ensure main loop doesn't get confused by held button)
	while (GetButtonState() == PRESSED) { }
					
	wait(500);
			
}
		
		
		
		
// -------------------------------------------------------------------------------------
		
//volatile uint8_t powerState;

void handleButtonPress() {
	
						
			
	// Easter Egg....
	// Regardless of power_state, if user holds button for X seconds, then secret message blink!
			
	// NOTE:    This demonstrates a possible reset function- hold button long time for special reset!
	//			and is useful in testing power stability, but could be disabled for production.
			
	if (button.lastHold_ms > 20000) {
				
		// WiFi module firmware recovery mode
		// TODO: Consider if this a good idea in production release!.. Implications of accidental entry...
				
		goWiFiRecoveryMode();
				
		// Return immediately - do not change power state based on this long press
		return;
					
					
				
	}
			
	if (button.lastHold_ms > 10000) {
				
		// Hmmm. What mischief can be had....
		// TODO: Morse Code led blinker!
				
		blinkParallaxTechTel();
				
		// Return immediately - do not change power state based on this long press		
		return;
				
	}
			
			
	
	uint8_t powerState = GetPowerState();
	
	if (powerState == PWR_ON) {
		
		
		
				// 1 press = power off badge
				// 2 press = reset propeller (so it re-starts whatever program currently loaded)
				// OPTION: Could have 4 press reset WiFi and Propeller? - shortcut of power-off/power-on. MUST change 4-press WiFi recovery to long single press, so it doesn't confuse!
								
				if (button.pressCount == 6) {
					
					// Secret WiFi protection override mode
					#ifdef ENABLE_IO14_OVERRIDE
					
							if (is_low(WX_CONFIGURE)) {
						
								// Mode active, so disable now
								configure_as_input(WX_CONFIGURE); // External pullup as WiFi is 3V3! Do NOT drive HIGH!
								blink_pwrled_slow(6);
											
						
							} else {
						
								// Mode not active, so enable now
								set_low(WX_CONFIGURE);
								configure_as_output(WX_CONFIGURE);
								blink_pwrled_fast(6);
						
							}
							
							// Accompanying CODE changed in Parallax-ESP firmware (cgiprop.c file)
							
							/*
							#ifdef WIFI_BADGE
							int ICACHE_FLASH_ATTR IsAutoLoadEnabled(void)
							{
								//return GPIO_INPUT_GET(AUTO_LOAD_PIN) == AUTO_LOAD_PIN_STATE;
								
								return ((GPIO_INPUT_GET(AUTO_LOAD_PIN) == AUTO_LOAD_PIN_STATE) && (GPIO_INPUT_GET(RESET_BUTTON_PIN) != 0));
															
							}
							#endif
							*/
							
							
					#else
					
							blink_pwrled_fast(1);
							
					#endif						
				
				} else if (button.pressCount == 2) {
														
							
					// Power cycle
					set_badgepwr_off();
							
					// _delay_ms(50);  // Use small delay here *IF* following blink code removed
					
					blink_pwrled_fast(2); // blink feedback and delay at same time!
							
					set_badgepwr_on();
					
					// Badge switch-on takes about 2 seconds until "HELLO, Loading" message appears on badge OLED,
					// so flash led for about 2 seconds
					
					blink_pwrled_pwron(6); // Each flash-period about 300ms
							
							
				} else {
							
					blink_pwrled_slow(2);
					
					set_badgepwr_off();
							
				}
		
										
								
		} 
				
		else { // if (powerState == PWR_OFF) { // Must be PWR_OFF if reaching here! No other option!
								
		
				// 1 press = power on badge
				// 4 press = Wifi recovery mode (consider changing this to long 10second? single press)
		
				if (button.pressCount == 4) {
			
					// WiFi STA+AP mode
									
					goWiFiAPSTAMode();
									
																			
			
				} else {
						
					// Power on badge (default to this action if not 4 presses!)
									
					set_badgepwr_on();
					
					// Badge switch-on takes about 2 seconds until "HELLO, Loading" message appears on badge OLED,
					// so flash led for about 2 seconds
					
					//blink_pwrled_fast(10); // Each flash-period about 200ms
					blink_pwrled_pwron(6); // Each flash-period about 300ms
						
				}
						
												
		
		}
		
						
				
}




// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************



// -------------------------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------------------------		
		
int main(void)
{
			
	// Configure stuff
			
	setClock();
			
	// Global milliseconds clock - ** MUST CALL THIS BEFORE ANY wait CALLS **
	init_millis_timer();
			
	
	// * PB0 - WX_Configure (Input NO pullup, external 10K PUP to 3V3 inside WROOM module. Set output low to set WiFi bootloader mode)
	// * PB1 - Pwr_Switch   (Input pulled up, or output low to flash pwr LED)
	// * PB2 - Pwr_Enable   (Input NO pullup, external 10K PUP to BATT-5V, up=badge off, output low=badge on, connected to PFET gate)
	// * PB3 - Tiny_RESn	(Input pulled up by default - leave it alone! Do not use this pin in this version!)
	// *
			
	// * Care with PB0!  PB0 is ~5V, and connects to WX_Configure which is 3.3V, and is already pulled-up at the WiFi module.
						
	// Pin default states
			
	//configure_as_input(PWR_SWITCH);
	pullup_enable(PWR_SWITCH);
	pullup_on(PWR_SWITCH);
			
	//configure_as_output(PWR_ENABLE);
	set_badgepwr_off();
	
	
			
	// Main loop
	// TODO: Power Saving option - Sleep until button press detected !
			
	while(1)
	{
												
		// Reset button counter / stats
		button.pressCount = 0;
		button.lastHold_ms = 0;
																
		// Reset milliseconds counter - Keep variables small as possible / prevent overflow & wrapping weirdness
		//millisReset(); // milliseconds = 0;
		init_millis_timer(); // Temp fix to the interrupt stopping unexpectedly
				
				
		while(millis() < button.timeout_ms) {
								
			// Get current button state
			button.currentState = GetButtonState();
				
								
			// If not pressed, re-loop
			if (button.currentState == NOT_PRESSED) continue;
				
			// Short delay to ensure press-bounce ignored
			//wait(5); // DO NOT call WAIT inside this loop, as millis reset
			_delay_ms(10);
				
			// Here switch must be pressed, wait debounce period, but fail if button released
			if (waitwhilepressed(button.debounce_ms) == KO) continue;
				
				
			// Got here means button still held down (good press)
			// Wait until button released
			while (GetButtonState() == PRESSED) {}
				
				
			// Button surely not-pressed now
			// Store hold-time and Increment total press counter
			button.lastHold_ms = millis(); // - button.debounce_ms;
			button.pressCount++;
			
			//millisReset(); // milliseconds = 0;
			init_millis_timer(); // Temp fix to the interrupt stopping unexpectedly
					
										
			// loop for another button press
				
		}
				
				
				
		// Handle button presses (count & hold time)
				
		if (button.pressCount > 0) {
					
			#ifdef DEV_DEBUG_MODE
					
				// Debug Out - Measure pulses on scope to prove count and last_hold duration
				// NOTE: ONLY ON TEST BOARD! NOT PRODUCTION UNIT!
					
				/*for(uint8_t i=0;i<button.counter;i++) {
						
					if (i>0) { wait(100); }
					set_high(PWR_ENABLE);
					wait(100);
					set_low(PWR_ENABLE);
						
				}
					
				wait(button.lastHold_ms);
					
				set_high(PWR_ENABLE);
				wait(10);
				set_low(PWR_ENABLE);
				*/
					
			#endif
					
			handleButtonPress();
					
		}
				
				
	}
			
	return 0;
			
}









// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************


// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************


// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************


// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************


// *******************************************************************************************************************************
// *******************************************************************************************************************************
// *******************************************************************************************************************************
