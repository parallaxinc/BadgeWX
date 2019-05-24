// ------ Libraries and Definitions ------
#include "simpletools.h"
#include "badgewxtools.h"
#include "wifi.h"
#include "oled_asmfast.c"
//#include "ws2812.h"



#define BL_VerString "FwVer1.3\0" // Bump each new public release. DO NOT EXCEED 8 CHARS!!!




// ---------------------------------
//
// TESTING DEFINE BLOCK
//
// REMOVE ALL THESE FOR PRODUCTION !
//
// ---------------------------------

//#define TEST_SSID_EXTRA
//#define TEST_SSID_NONE
//#define TEST_SSID_ONLYONE

// ---------------------------------




#define BADGEWX_REVA_PINS // (Use this for RevA and RevB badges)

// Note- most pin defines removed as now included in badgetools-wx

#ifdef BADGEWX_REVA_PINS

/*  #define RGB_PIN       10
  #define LED_PIN       9  // **** Changed since RevA1
  #define NAV_L         13 // **** Changed since RevA1
  #define NAV_C         12
  #define NAV_R         11 // **** Changed since RevA1
  #define NAV_COM_L     14
  #define NAV_COM_R     15
  #define NAV_TOUCH_L   4
  #define NAV_TOUCH_R   2  // **** Changed since RevA1
  #define IR_OUT        3
  #define IR_IN         23
  #define OLED_DAT      21
  #define OLED_CLK      20
  #define OLED_DC       22
  #define OLED_RST      19
  #define OLED_CS       18
  #define TV_DAC2       26
  #define TV_DAC1       25
  #define TV_DAC0       24
  #define AUD_RT        1
  #define AUD_LF        0
  
  // The following pin names might not match badgetools.h
  #define SD_CS         5
  #define SD_MOSI       6
  #define SD_CLK        7
  #define SD_MISO       8  // **** Changed since RevA1
 */  
  #define WX_RES        16 // **** Changed since RevA2

  #define WX_BOOT       17 // **** Changed since RevA2
  
  #define USER_IO       27  
    
#endif


#ifdef BADGEWX_REVA2_PINS

  #define RGB_PIN       10
  #define LED_PIN       9
  #define NAV_L         13
  #define NAV_C         12
  #define NAV_R         11
  #define NAV_COM_L     14
  #define NAV_COM_R     15
  #define NAV_TOUCH_L   4
  #define NAV_TOUCH_R   2
  #define IR_OUT        3
  #define IR_IN         23
  #define OLED_DAT      21
  #define OLED_CLK      20
  #define OLED_DC       22
  #define OLED_RST      19
  #define OLED_CS       18
  #define TV_DAC2       26
  #define TV_DAC1       25
  #define TV_DAC0       24
  #define AUD_RT        1
  #define AUD_LF        0
  
  // The following pin names might not match badgetools.h
  #define SD_CS         5
  #define SD_MOSI       6
  #define SD_CLK        7
  #define SD_MISO       8
  
  #define WX_RES        17
  #define WX_BOOT       16
  
  #define USER_IO       27  
    
#endif

#define reboot() __builtin_propeller_clkset(0x80)


// Buttons
#define L_NAV_LEFT 6
#define L_NAV_RIGHT 4
#define L_NAV_PRESS 5
#define R_NAV_LEFT 3
#define R_NAV_RIGHT 1
#define R_NAV_PRESS 2
#define L_TOUCH_A 0
#define R_TOUCH_B 0


#define EXITCODE_NONE -1
#define EXITCODE_RUN 0
#define EXITCODE_INFO 1
#define EXITCODE_REFRESH 2
#define EXITCODE_LOAD 3
#define EXITCODE_CONNECT 4
#define EXITCODE_START 5
#define EXITCODE_RESTART 6
#define EXITCODE_ABORT 7
//#define EXITCODE_FORGET 8
#define EXITCODE_OK 9
#define EXITCODE_ACTIVESSID 10
#define EXITCODE_WIFISTATUS 11
#define EXITCODE_SEARCH 12
#define EXITCODE_NEEDPASSWORD 13
#define EXITCODE_WAITFORCONNECTION 14
#define EXITCODE_ADMIN 15
#define EXITCODE_LOGOUT 16
#define EXITCODE_DO_WIFIOFF 17
#define EXITCODE_DO_WIFION 18

//#define EXITCODE_SLEEPDEEP 19
//#define EXITCODE_SLEEPLIGHT 20
//#define EXITCODE_SLEEPWAKE 21


#define SEARCH_ABORT 0
#define SEARCH_ABORTED 1
#define SEARCH_SEARCHING 2
#define SEARCH_COMPLETE 3

#define WIFI_ABORT 0
#define WIFI_CONNECTING 1
#define WIFI_CONNECTED 2
#define WIFI_NOTCONNECTED 3

#define WIFI_POWER_OFF 0
#define WIFI_POWER_ON 1


#define LEFT 0
#define RIGHT 1
#define ABCMODE 2



unsigned int STACK_GUI[40 + 180];
unsigned int STACK_WIFI[40 + 200];
unsigned int STACK_GUI_TIMER[40 + 20];
unsigned int STACK_SYS_TIMER[40 + 20];


#define COG_STOPPED 0
#define COG_STOP 1
#define COG_STARTING 2
#define COG_RUNNING 3
#define COG_ELAPSED 4
#define COG_BUSY 5


void GetModuleName();
void WiFiSearch(void *par);
void WiFiInit();

void WiFiGetConnectedSSID(void *par);
void GUI(void *par);
void GUI_ABC(void *par);
void GUI_TIMER(void *par);
void SYS_TIMER(void *par);

_Bool WiFiWaitForMode(const int WiFiMode);

int menu_wifiGetPassword();
int menu_wifiConnect();
int menu_wifiWaitForConnection();
int menu_wifiManualSearch();
int menu_wifiAdmin();
int menu_wifiSleepDeep();

int DO_WIFION();
int DO_WIFIOFF();

int AllowDownload();

void wifiLogout();
void doRUN();

int menu_wifiStatus();

i2c *eeBus; // I2C bus ID

unsigned char buttons();
char button(char b);


// ------ Global Variables and Objects ------

char *ABC =  "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789\0"; // 37 chars + null


#define ABC_upper "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789"
#define ABC_lower "abcdefghijklmnopqrstuvwxyz 0123456789" 
#define ABC_other "_.,:;?!@#&$%`'\"=+-*/|\\^<>()[]{}~"



#define menuIdxABCMax 36; // strlen(ABC) - 1;
#define menuIdxOtherMax 31;
volatile int menuIdxMax = menuIdxABCMax;

volatile int cogWiFi;
volatile int cogGUI;
volatile int cogGUI_TIMER;
volatile int cogSYS_TIMER;

volatile int cogState_GUI = COG_STOPPED;

volatile int cogState_GUI_TIMER = COG_ELAPSED; 
volatile int GUI_TIMER_MILLISECONDS = 1000;

volatile int cogState_SYS_TIMER = COG_ELAPSED; 
volatile int SYS_TIMER_MILLISECONDS = 1000; 


volatile int wifiConnectionStatus = 0;
volatile int wifiSearchStatus = 0;

volatile int validEEpromImage = 0;

volatile int navLL = 0;
volatile int navLR = 0;
volatile int navLP = 0;
volatile int navRL = 0;
volatile int navRR = 0;
volatile int navRP = 0;

volatile int validPassword = 0;

volatile int justSearched = 0;

volatile int disableAutoRun = 0;

char _moduleName[35]; // 32 maxlen + 2 prefix chars + final null
char *moduleName = _moduleName + 2; // Pointer starts at 3rd char
volatile int moduleNameLen = 0;

char _activeIP[16];
char *activeIP = _activeIP + 2; // Pointer starts at 3rd char

char _activeSSID[35]; // = "                                \0"; // "\0";
char *activeSSID = _activeSSID + 2; // Pointer starts at 3rd char


int event, id, handle;

int _apStatusi;
int _apEnc;
int _apRSSI;

char _apSSID[33]; // SSID 32 chars max + null
char _apPASS[33]; // PASS 32 chars max + null

int wifiFoundCount = 0;

char _menuSpaces[17] = "                \0";
char *menuSpaces = _menuSpaces;

char menuLeft[10][9]; // max 8 chars 
char menuRight[10][9]; // max 8 chars 

const char encName[6][5] = {"OPEN\0"," WEP\0"," WPA\0","WPA2\0","WPA+\0"," MAX\0"};

//const char GC_CANCEL[7] = "CANCEL\0";

volatile int runIdx = 5;

volatile int menuLeft_elements = 0;
volatile int menuRight_elements = 0;

volatile int menuIdxLeft = 0;
volatile int menuIdxRight = 0;
volatile int menuIdxABC = 0;

volatile int menuSelectedLeft = 0; // ButtonHandler cog will set 1 when left button pressed
volatile int menuSelectedRight = 0;

volatile int scrollLeft = LEFT;
volatile int scrollRight = RIGHT;


char OLED[8][17]; // Holds the current display text
char OLEDbuf[17] = "                \0"; // Holds single row during processing (such as trimming and centering)
volatile int updateGUI = 1;

int dotidx = 3; // 12
int dotdir = 1; // 0
//long t; // = CLKFREQ + CNT;  
int ssidIdx = 0;
int ssidIdx_last = 1; // ensure display updated in first loop
int force = 1;  
  
#define menuIdxLeftMin 0
    
volatile int menuIdxLeftMax = 0;
    
#define menuIdxRightMin 0
    
volatile int menuIdxRightMax = 0;
volatile int _lastMenuIdxLeft; // = menuIdxLeft;
volatile int _lastMenuIdxRight; // = menuIdxRight;
   
int lastRunIdx = -1;
    
//volatile long ggTimeout; // = CNT + 160000000; // 2 second ggTimeout
    
#define cursorCharON 5
#define cursorCharOFF ' '
    
volatile char cursorCharLast; // = cursorCharON;
    
#define menuIdxABCMin 0
    
volatile int lastMenuIdxABC; // = menuIdxABC;
volatile int cursorIdx = 0; // Current cursor index, 0-15
volatile int cursorIdxLast = 0;
    
#define cursorRowMin 5
#define cursorRowMax 6
    
volatile int cursorRow = 5; // Current cursor row, 5 or 6 (NOTE: ABC chars will be cursorRow-1)
    
volatile int passwordIdx = 0; // Current password index, 0-31
volatile int passwordIdxLast = 0;
        
int oneTimeUpdate = 1;
    
volatile int lastOtherIdx = 0;
volatile int lastABCIdx = 0;
volatile int ABC_charset = 0;
 
 
volatile int wifiPowerStatus = WIFI_POWER_ON;
  

// OVERRIDE badge_setup

int i2cLock = 0;

i2c *st_eeprom;
int st_eeInitFlag;
volatile int eei2cLock;
volatile int eei2cLockFlag;

volatile int eeHome = 32768;
volatile int eeRecCount, eeNextAddr, eeBadgeOk = 0, 
             eeNext, eeRecsAddr, eeRecs, 
             eeRecHome, eeRecOffice;


int32_t badge_setup(void)
{
  if(!eei2cLockFlag)
  {
    eei2cLock = locknew();
    lockclr(eei2cLock);
    eei2cLockFlag = 1;
  }
  
  if(!st_eeInitFlag) ee_init();
  screen_init(OLED_CS, OLED_DC, OLED_DAT, OLED_CLK, OLED_RST, SSD1306_SWITCHCAPVCC, TYPE_128X64);
  screen_auto(ON);
  return 0;
}



// -----------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Helper functions
//
// -----------------------------------------------------------------------------------------------------------------------------------------------------------


_Bool startsWith(const char *restrict prefix, const char *restrict string)
{
    while(*prefix)
    {
        if(*prefix++ != *string++)
            return 0;
    }

    return 1;
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


unsigned char buttons() {
  
  // [L touch][L NAV left][L NAV ctr][L NAV right][R NAV left][R NAV ctr][R NAV right][R touch]
  int btns = 0b00000000;
  long tDecay[2];                    // Declare tDecay variable  
  
  input(NAV_COM_L);                  // Set common lines to inputs
  input(NAV_COM_R);
  
  input(NAV_C);                      // Set pins to inputs
  input(NAV_R);

  high(NAV_L);                       // Set pin high
  
  btns |= (input(NAV_COM_L) << 6);
  btns |= (input(NAV_COM_R) << 3);
  
  input(NAV_L);
  high(NAV_C);
  
  btns |= (input(NAV_COM_L) << 5);
  btns |= (input(NAV_COM_R) << 2);
  
  input(NAV_C);
  high(NAV_R);
  
  btns |= (input(NAV_COM_L) << 4);
  btns |= (input(NAV_COM_R) << 1);
  
  input(NAV_R);
   
  return btns;
}


char button(char b) {
  // Make sure b is 0-8
  b = (b < 0 ? 0 : b);
  b = (b > 7 ? 7 : b);
  
  int m = 1 << b;
  return (buttons() & m) >> b;
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------





void centerToOLEDbuf(char *oled) {

  // Trim to 16 chars and center string for display
  int length = strlen(oled);
  int offset = 0;
  
  if (length > 16) { 
  
      length = 16;
      
  } else { 
  
      offset = (16 - length) / 2;
  }
  
  
  // Ensure no overflow on screen. Perhaps not needed...
  while (offset + length > 16) {
    --offset;
  }     
    
  
  // Clear buf
  strncpy(OLEDbuf, "                ", 17); // Copy 16 spaces and 1 null byte
  
  
  // Copy oled to buf with padding       
  for( int i = 0; i < length; i++ ) {
    
    OLEDbuf[offset+i] = oled[i];
  
  }  


}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


volatile _Bool isValidEEpromImage() {
    
   
     volatile int _eepromval = 0;
      
      
     eeBus = i2c_newbus(28,  29,   0); // Set up I2C bus, get bus ID
      while(i2c_busy(eeBus, 0b1010000));
          
     
     // 1. Check word 3 - Start of Code pointer (must always be $0010)
     i2c_in(eeBus, 0b1010000, 6, 2, (char*) &_eepromval, 2);  // expect 0010, or decimal 16, for valid eeprom image
     while(i2c_busy(eeBus, 0b1010000));
     
     if (_eepromval != 0x0010) { return 0; }
     
      
     // 2. Get word 5 - Start of Stack Space pointer
     i2c_in(eeBus, 0b1010000, 10, 2, (char*) &_eepromval, 2);  // expect 1st free stack address AFTER stackpointer
     while(i2c_busy(eeBus, 0b1010000));
          
     // 3. Check word prior to Stack Space pointer address (deduct 2 from address)
     i2c_in(eeBus, 0b1010000, _eepromval-2, 2, (char*) &_eepromval, 2);  // expect FFF9, or decimal 65529, for valid eeprom image
     while(i2c_busy(eeBus, 0b1010000));
     
     return (_eepromval == 0xFFF9) ? 1:0;
  
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void invalidateEEpromImage() {
    
   
     eeBus = i2c_newbus(28,  29,   0); // Set up I2C bus, get bus ID
     while(i2c_busy(eeBus, 0b1010000));
          
     
     // 1. Check word 3 - Start of Code pointer (must always be $0010)
     i2c_out(eeBus, 0b1010000, 6, 2, (char*) 9999, 2);  // clear to 9999; invalidate image
     while(i2c_busy(eeBus, 0b1010000));
              
  
     // Note: Whilst this doesn't actually erase the entire lower eeprom, it does invalidate the code
     //       - at the next programming cycle the entire eeprom will be erased anyhow by the programming process
     
     // Upper EEPROM - that's another story- 
     // TODO: Clear upper EEprom option? byte-by-byte ?
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void doRUN() {
  
    // Clear screen and display useful text before executing the run command
    // so that at next boot time, this msg will appear (if the run fails, or the user doesn't refresh the screen)
  
    // Stop any GUI cogs before writing to screen
    // TODO: Check if GUI cog is actually running before calling end ?
    
    //cog_end(cogGUI);
    
    if (cogState_GUI != COG_STOPPED) {
      
        cogState_GUI = COG_STOP;
        while (cogState_GUI != COG_STOPPED) { } // Wait for GUI to actually stop
    
    }
  
       
      
    clear();
    cursor(0,0);
    text_size(LARGE);
    oledprint("BadgeWX");
    text_size(SMALL);
    cursor(0,6);
    oledprint(" blocklyprop.com");
    
    waitcnt(CNT + CLKFREQ/2);
    reboot();
                        
                          
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


// Clear the OLED display buffer, which is rendered by the GUI cog
void clearOLED() {
  
  
  int idx = 8; // 8 rows
  while (idx-- > 0) {
    
    strncpy(OLED[idx], "                ", 17); // Copy 16 spaces and 1 null byte
  
  }  
  
    
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void clearMenus() {
  
  int idx = 10;
  while (--idx >= 0) {
    
      strncpy(menuLeft[idx], "        ", 9); // Copy 8 spaces and 1 null byte
      strncpy(menuRight[idx], "        ", 9); // Copy 8 spaces and 1 null byte
      
  }
  
  menuLeft_elements = 0;
  menuRight_elements = 0;
  
  menuIdxLeft = 0;
  menuIdxRight = 0; 
  
  menuSelectedLeft = 0;
  menuSelectedRight = 0;
  
}



// -----------------------------------------------------------------------------------------------------------------------------------------------------------



void updateOLED_row(int rowIdx) {
	
	// Call only from the GUI cog!
	
	cursor(0, rowIdx);
	oledprint("%s", OLED[rowIdx]);
	
	
	
}



// -----------------------------------------------------------------------------------------------------------------------------------------------------------



void updateOLED() {
	
  // Call only from the GUI cog!
	
  int rowIdx = 7; // Don't update row 8 (idx 7), as menuUpdate will do that
	  
  //while (--rowIdx >= 0) {
  while (rowIdx-- > 0) {
 
      //cursor(0, rowIdx);
      //oledprint("%s", OLED[rowIdx]);
  
      updateOLED_row(rowIdx);
	
  }
	
}



// -----------------------------------------------------------------------------------------------------------------------------------------------------------


int StringLength(char *searchString, int maxLen) {

   int idx = 0;
  
   while ((++idx < maxLen) && (searchString[idx] != '\0')) { }
    
   return idx; 
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------




// -----------------------------------------------------------------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void oledWriteMenus(int idxLeft, int idxRight) {
  
    //strlen(menuRight[menuIdxRight])], menuRight[menuIdxRight]); 
    menuSpaces = _menuSpaces + strlen(menuLeft[idxLeft]) + strlen(menuRight[idxRight]);
    
    cursor(0,7);
    oledprint("%s%s%s", menuLeft[idxLeft], menuSpaces, menuRight[idxRight]); 
  
}  


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void oledWriteMenuLeft(int idxLeft) {
  
    menuSpaces = _menuSpaces + 8 + strlen(menuLeft[idxLeft]);
    
    cursor(0,7);
    oledprint("%s%s", menuLeft[idxLeft], menuSpaces); 
  
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void oledWriteMenuRight(int idxRight) {
  
    menuSpaces = _menuSpaces + 8 + strlen(menuRight[idxRight]);
    
    cursor(8,7);
    oledprint("%s%s", menuSpaces, menuRight[idxRight]); 
  
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------






// -----------------------------------------------------------------------------------------------------------------------------------------------------------


_Bool WiFiWaitForMode(const int WiFiMode) {
	
    int tries = 3;
	
    while ( (--tries >= 0) && (wifi_mode(WiFiMode) != WiFiMode) ) { }

    return (tries == 0) ? 0 : 1;	
		
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void wifiClearAllCredentials() {
  
  
    // Clear stored credentials
    memset(_apSSID, '\0', sizeof(_apSSID));  // Clear to null
    memset(_apPASS, '\0', sizeof(_apPASS));  // Clear to null
    memset(_activeSSID, '\0', sizeof(_activeSSID));  // Clear to null
    memset(_activeIP, '\0', sizeof(_activeIP));  // Clear to null 
    validPassword = 0;
     
}  



// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void wifiClearActiveCredentials() {
  
  
    // Clear stored credentials
    memset(_activeSSID, '\0', sizeof(_activeSSID));  // Clear to null
    memset(_activeIP, '\0', sizeof(_activeIP));  // Clear to null 
        
} 


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void wifiLogout() {
  
 
    // Pretty dumb function for now....
    // Clear stored SSID/IP info
    // Send fake JOIN command to module to disconnect it from network
    
    wifiClearAllCredentials();
    
    
    // Start the wifi connection
    wifi_start(31, 30, 115200, WX_ALL_COM);
 
    // Invalidate stored password in WiFi module for current SSID
    // TODO: Some sort of "logout" command might be smarter.... trouble is, when we swap mode to STA the module will auto-try to re-login (I think), unless we distort the password
    //        - maybe an API command that sets the module "OFFLINE" and "ONLINE" will do the job, so it either can (or cannot) attempt to login
    
    // Note- removed as open networks will ignore the pw, and still connect:)
    //wifi_join(_activeSSID, "p2l3x");
    
    WiFiWaitForMode(AP); // Set AP mode to disconnect
    
    WiFiWaitForMode(STA); // Ensure station mode again (And hope the module doesn't auto-reconnect)
    
    // TODO: In this case, this is not really "FORGET" network. We don't have an API for the ESP module to delete credentials.
     
    wifi_stop();
    
    wifiConnectionStatus = NOT_CONNECTED;
    
    
}



// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------









// -----------------------------------------------------------------------------------------------------------------------------------------------------------
//
// MAIN
//
// -----------------------------------------------------------------------------------------------------------------------------------------------------------


int main() {
  
  
  //cogLED = cog_run(bothLedsON, STACK_LED);
  
  badge_setup();
  
  // Clear RGB LEDs
  
  /*int ws_start(ws2812_t *driver, int usreset, int ns0h, int ns0l, int ns1h, int ns1l, int type);
  
  ws2812 *ws2812b;
  int RGBleds[4];
  
  ws2812b = ws2812b_open();
  for (int __ldx = 0; __ldx < 4; __ldx++) {
    RGBleds[__ldx] = 0x000000;
  }
  ws2812_set(ws2812b, RGB_PIN, RGBleds, 4); */


  // strip.startx(@pixels1, STRIP_LEN, STRIP_PIN, 1_0, true, 24, 250,700,600,500) ' Timing for RevA 1818, RevB 1823
  // pub startx(p_buf, count, pin, holdoff, rgswap, bits, ns0h, ns0l, ns1h, ns1l)
  
  // default simplelib settings: return ws_start(state, 50, 350, 800, 700, 600, TYPE_GRB);
  // int ws_start(ws2812_t *state, int usreset, int ns0h, int ns0l, int ns1h, int ns1l, int type)
  
  // Override startup routine!
  /*ws2812b = ws2812b_open();
  ws2812_stop(ws2812b);
  ws2812b = (ws2812_t * ) malloc(sizeof(ws2812_t));
  //ws_start(ws2812b, 300, 300, 900, 900, 350, TYPE_GRB);
  ws_start(ws2812b, 300, 250,700,600,500, TYPE_GRB);
  
  
  // Clear or set colors!
  for (int __ldx = 1; __ldx <= 4; __ldx++) {
    RGBleds[constrainInt(__ldx, 1, 4) - 1] = 0x000000;
  }
  ws2812_set(ws2812b, RGB_PIN, RGBleds, 4);*/

  
  
  // NOTE: If this code always follows autorun.bin or autorunSD.bin, then no need to clear screen,
  //       nor to write HELLO!, as that will already be on the screen.
  //
  //	   Pros/Cons
  //		- Clearing screen creates flicker
  //		- But what if user calls this BL from other code, with artifacts on the screen....
  //
  //	   Solution- clear for now.
  
  clearOLED();
  
  /*text_size(LARGE);
  cursor(1,0);
  oledprint("%s", "HELLO!");*/

  //cursor(0,0);
  //text_size(LARGE);
  //oledprint("BadgeWX");
  
    
  // ---
    
  DIRA = DIRA & ~(1<<30); // Important! Release pins before running WiFi cog
  DIRA = DIRA & ~(1<<31);
  
  // ---
  

  WiFiInit();
  
  // This cog will terminate itself when complete, and set appropriate wifiStatus flag
  // wifiStatus = WIFI_CONNECTED or wifiStatus = WIFI_NOTCONNECTED
  // ggTimeout limit about 20 seconds (IP getting can be slow)
  //cogWiFi = cog_run(WiFiGetConnectedSSID, STACK_WIFI); 
  
  
  // ---
  
  
  
  cogGUI_TIMER = cogstart(GUI_TIMER, NULL, STACK_GUI_TIMER, sizeof(STACK_GUI_TIMER));
  cogSYS_TIMER = cogstart(SYS_TIMER, NULL, STACK_SYS_TIMER, sizeof(STACK_SYS_TIMER));
  
  
  // ---
  
  
  // Is there a current valid image in EEprom (if something Runnable, then include Run menu option!)
  validEEpromImage = isValidEEpromImage() ? 1:0;
  
  // if EEprom image check failed, try 3 times more
  if (validEEpromImage == 0) { validEEpromImage = isValidEEpromImage() ? 1:0; }
  if (validEEpromImage == 0) { validEEpromImage = isValidEEpromImage() ? 1:0; }
  if (validEEpromImage == 0) { validEEpromImage = isValidEEpromImage() ? 1:0; }
  
  // ---
  
  
  volatile int result = EXITCODE_RESTART;
  
  
  // ---
  
  
  // Keep looping, calling the next menu based on previous exit code
  
  while (1) {
	  
			
				switch (result) {
				
					case EXITCODE_RESTART: // Full re-start- call after attempting to Join new network
					
						 //cogWiFi = cog_run(WiFiGetConnectedSSID, STACK_WIFI);
			         cogWiFi = cogstart(WiFiGetConnectedSSID, NULL, STACK_WIFI, sizeof(STACK_WIFI));			 
       
						 // Fall through to EXITCODE_WAITFORCONNECTION 
				
    
					case EXITCODE_WAITFORCONNECTION:
     
                result = menu_wifiWaitForConnection();
                
                //cogState_LED = COG_STOP; // Only relevant at powerup (switches off the 2 status leds)
                break;
     
     
            //case EXITCODE_START: 
					
						 //result = menu_start(); // blocking
						 //break; 
				
				
            case EXITCODE_RUN:
					
						 doRUN(); // Will never return (Propeller Reboot) 
						 
					
            case EXITCODE_SEARCH:
					
						 result = menu_wifiManualSearch(); // blocking
						 break;
       
            case EXITCODE_ADMIN:
            
                result = menu_wifiAdmin(); // blocking
                break;
       
       
            case EXITCODE_LOGOUT: // Logout from network to go offline, then return to WiFiStatus menu
            
                wifiLogout();
                // Fall through to EXITCODE_WIFISTATUS
				
				
            case EXITCODE_WIFISTATUS:
					
						 /*if (wifiStatus == WIFI_CONNECTED) {
					
								result = menu_wifiConnected(); // blocking 
					
						 } else { // Must not be connected
				  
								result = menu_wifiNotConnected(); // blocking 
					
						 }*/
						 
						 result = menu_wifiStatus(); // blocking
						 break;
						 
						 
					case EXITCODE_LOAD:
					
						 result = AllowDownload(); // blocking
						 break; 
						 
					
					case EXITCODE_CONNECT:
					
						 result = menu_wifiConnect(); // blocking
						 break;
						 
					
					case EXITCODE_NEEDPASSWORD:
					
						 result = menu_wifiGetPassword(); // blocking
						 break;
       
      
                /*case EXITCODE_SLEEPDEEP:
					
						 result = menu_wifiSleepDeep(); // blocking
						 break;*/
       
       
       
            case EXITCODE_DO_WIFIOFF:
             
                result = DO_WIFIOFF(); // blocking
						 break;
       
       
            case EXITCODE_DO_WIFION:
             
                result = DO_WIFION(); // blocking
						 break;
       
        /*
            case EXITCODE_SLEEPLIGHT:
					
						 result = menu_wifiSleepLight(); // blocking
						 break;
       
            case EXITCODE_SLEEPWAKE:
					
						 result = menu_wifiSleepWake(); // blocking
						 break;*/
													
				} // switch (result) 
          
          
          
				// Always stop GUI cog after each menu
				// Note: Just simple, as we have couple different GUI cogs at the moment (ABC and default)
				if (cogState_GUI == COG_RUNNING) {
				  
						cogState_GUI = COG_STOP;
						while (cogState_GUI != COG_STOPPED) { } // Wait for GUI to actually stop
               waitcnt(CNT + CLKFREQ/1000); // Wait 1ms after the cog signals the stop (as we might try to re-use it fast, and the signal comes just before the actual stop occurs)
			  
				}          
            
            
			} // while (1)
				
			
// ---

// End of main. Will never reach here !

}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------











// -----------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Menus
//
// -----------------------------------------------------------------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


// WiFiInit
// - Call once at startup
// - Set module defaults and get module name
// - If fail, report status to OLED and blink 0/1 appropriately

void WiFiInit() {
	
    // Reset the WiFi module - sometimes it get's badly stuck! (TODO: Maybe only in dev... let's see)
    /*high(WX_BOOT); // disable bootloader- IMPORTANT!
    low(WX_RES);
    waitcnt(CNT + CLKFREQ/1000); // 1 ms
    input(WX_RES);
    
    // Wait a sec before releasing the lock pin, to prevent the ESP module uploading bootloader code to the Propeller
    waitcnt(CNT + CLKFREQ/10); // 100 ms
    input(WX_BOOT); */
	
  
 
    // Start the wifi connection
    wifi_start(31, 30, 115200, WX_ALL_COM);
    
    // Reset all stored credentials & flags
    wifiClearAllCredentials();
 
    // Instruct module to join a network, using ESP-flash-stored credentials if available
    //WiFiWaitForMode(STA); // Setting STA mode should have the ESP module auto-join the last known network in range
    //wifi_print(CMD, NULL, "%cJOIN:\r", CMD);
 
    // Get module name
    // NOTE: Takes about 3ms when WiFi module active, or ggTimeout after 3 secs if WiFi module disabled or missing/faulty (reset shunted to gnd perhaps?)
	
    memset(_moduleName, '\0', sizeof(_moduleName));  // Clear to null
    
    //wifi_print(CMD, NULL, "%cSLEEP:1\r", CMD);
    
        
    int tries = 3;
    while (--tries >= 0) {
            
        wifi_print(CMD, NULL, "%cCHECK:module-name\r", CMD);
        wifi_scan(CMD, NULL, "%34c", &_moduleName); // Will return prefix  S,   module-name   $0d   (eg.  S,WX4567f4$0d   )
			
        // Use %34c in the scan for maximum 32 chars + 2 prefix chars. 
        // Note: %s won't work due to potential spaces in the reply (spaces are parsed into blocks)
		  
        // After the scan, search for the last $0d to determine module-name length
        // (as module name has $0d at the end of it, followed by spaces)
		   

        if (startsWith("S", _moduleName)) { // Success
			
            // Get module-name length and clear suffix $0d to null
            moduleNameLen = sizeof(_moduleName); // 35 at time of writing code
            while (_moduleName[--moduleNameLen] != 13) { } // Last char always null, so this works!
            _moduleName[moduleNameLen] = '\0';
            --moduleNameLen; // remove S prefix 
            --moduleNameLen; // remove , prefix      

            wifi_stop();						 
            return;
				
        } // if
			
    } // while (--tries >= 0)
 
 
 
    // ---
    
    
    // If we get here, WiFi module didn't respond at power-up!
    
    // NOTE: This was the very first check of the WiFi module presence when Bootloader starts
    //       If the user has the PGM? or RESET jumper installed, the WiFi module won't respond
    //       Suggest user checks the badge, and then powercycle
    //
    // NOTE: This might be silly for real users
    //        - Surely they won't run this code without booting it from the WiFi module?!!
    //       Leave this check stage for now, as it might be handy template to move to the 1st HELLO screen (autorun.bin)
    //        - Also, reading the module name is a requirement in any case.
     
		  
    clear();
    text_size(SMALL);
    cursor(0,0);
		
    oledprint("   WiFi module  ");
    oledprint(" not responding ");
    oledprint("                ");
    oledprint("PGM mode active?");
    oledprint("                ");
    oledprint("Powercycle Badge");
    oledprint("to retry.       ");
    oledprint("          IGNORE"); 
  
    // Halt program here?
     
    // TODO: Consider auto-countdown on IGNORE button. 
    //       Is it possible customer might want to run without WiFi module powered up? Just RUN pre-installed code? (Badge could have WiFi disabled by shunting RES to GND)
    //       SD card could also hold autorun firmware, but having code in EEprom might be handy for environments that don't want WiFi active, and don't need SD costs.

      
    while(1) { 
     
      
        if (button(R_NAV_PRESS)) { // IGNORE pressed
          
            // Wait until not pressed
            while(button(R_NAV_PRESS)) {}
            
            // If code exists in memory, run it. Else show splash screen
            // This is the IGNORE action, as not much else to do- and user might want to run code whilst WiFi module disabled.
            doRUN();
          
        }

    } // while(1)
		
}



// -----------------------------------------------------------------------------------------------------------------------------------------------------------


// Loop waiting for connected SSID and IP
void WiFiGetConnectedSSID(void *par) {
  
  
    
    
    wifiClearAllCredentials(); // TODO: Might only want to clear activeCredentials, if we want user the chance to re-edit password (although not secure!)
  
  
    // Set flag
    wifiConnectionStatus = WIFI_CONNECTING;
  
  
    // Start the wifi connection
    wifi_start(31, 30, 115200, WX_ALL_COM);
    
  
    // Ensure ESP module in station mode
    // TODO: Error handling if module doesn't make it into STA mode?
    WiFiWaitForMode(STA);
    

    // Wait until valid connection (check ssid and IP address)
    // Note: Check for valid IP address FIRST! (The ssid you get before a valid IP may not be the same AP as finally gets the connection!)
	   
    int tries = 40; // Each check has 1/2 second pause between, so try 40 times for ~20 seconds overall
     
    while ( (--tries >= 0) && (wifiConnectionStatus == WIFI_CONNECTING) ) {
        
              
        // Get station IP address (may take some seconds)  
		
        waitcnt(CNT + CLKFREQ/2); // 1/2 second pause between each check - pause at front as the 1st ipsearch takes some seconds anyhow, and don't want to delay a forced exit
			  
        wifi_print(CMD, NULL, "%cCHECK:station-ipaddr\r", CMD);
        wifi_scan(CMD, NULL, "%s\0", &_activeIP);
			
        if ( (startsWith("S", _activeIP)) && (!startsWith("0", activeIP)) ) {
		  
            // Got Valid IP! Grab the SSID...
            wifi_print(CMD, NULL, "%cCHECK:wifi-ssid\r", CMD);
            //wifi_poll(&event, &id, &handle); // Important! Must poll after check:wifi-ssid (Hmm... not now that the IP is read 1st!)
            wifi_scan(CMD, NULL, "%s\0", &_activeSSID);
			
            if (startsWith("S", _activeSSID)) {
				 
                wifiConnectionStatus = WIFI_CONNECTED; // Hooray!
                break;

            } // if
  
        } // if     
    
    } // while (--tries >= 0) {  
      
    
    // ---	
	
	
    wifi_stop();
  
    if (wifiConnectionStatus != WIFI_CONNECTED) { wifiConnectionStatus = WIFI_NOTCONNECTED; } // Override status if nothing was found
      
    //cog_end(cogWiFi);    
    cogstop(cogWiFi);  

}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------

/*
void StringToOLED(int row, int col, char * textPtr) {

    strcpy(OLED[row][col], textPtr);

}                                    
*/

// -----------------------------------------------------------------------------------------------------------------------------------------------------------



int menu_wifiWaitForConnection() {
  
  
  //cogGUI = cog_run(GUI, STACK_GUI);
  cogGUI = cogstart(GUI, NULL, STACK_GUI, sizeof(STACK_GUI));
  
  while (cogState_GUI != COG_RUNNING) { }
  
  
   
  strcpy(OLED[1], "    Badge-WX    ");
  strcpy(OLED[3], "   Connecting   ");
  strcpy(OLED[5], "   ..........   ");
  
  menuLeft_elements = 0;
  menuRight_elements = 0;
    
  if (validEEpromImage == 1) {
    
    strcpy(menuRight[menuRight_elements++], "RUN");
    strcpy(OLED[0], "              <>");
  
  }
  
  strcpy(menuRight[menuRight_elements++], "CANCEL");    
  //strcpy(menuRight[menuRight_elements++], GC_CANCEL); 
  
  
  updateGUI = 1;
  
    
  // ---
  
  // Progress bar
  
  dotidx = 3; // 12
  dotdir = 1; // 0
  //t = CLKFREQ + CNT;   
  
  SYS_TIMER_MILLISECONDS = 1000;
  cogState_SYS_TIMER = COG_RUNNING;
  
  // ---
  
  // Wait until network connection happens, or button pressed
  
  while (wifiConnectionStatus == WIFI_CONNECTING) {
    
    
    // Countdown timer display
    // Remove a dot from the display once every 1 second-ish
    
    //if (t < CNT) { // 1 second elapsed
    if (cogState_SYS_TIMER == COG_ELAPSED) {  
      
      if (dotdir == 0) { 
         
          if (dotidx > 2) {
            
              OLED[5][dotidx] = '.';
              --dotidx;
              updateGUI = 1;
              //t = CLKFREQ + CNT;
             SYS_TIMER_MILLISECONDS = 1000;
             cogState_SYS_TIMER = COG_RUNNING;
          
          } else {
            
              dotdir = 1; // Change direction
              ++dotidx;
            
          }            
        
      } else { // dotdir must be 1
        
          if (dotidx < 13) {
            
              //strcpy(OLED[5][dotidx], " ");
              OLED[5][dotidx] = '*';
              ++dotidx;
              updateGUI = 1;
              //t = CLKFREQ + CNT;
              SYS_TIMER_MILLISECONDS = 1000;
              cogState_SYS_TIMER = COG_RUNNING;
          
          } else {
              
              dotdir = 0; // Change direction
              --dotidx;
              
          } 
          
      } // if (dotdir == 0) {  
      
    } // if (t < CNT)
    
    
    //t = t + CLKFREQ;
  
    // ---
  
  
    // Handle button presses
    
    if (menuSelectedRight == 1) {
                
                  
            menuSelectedRight = 0;
                        
            // Abort search if currently in progress
            if (wifiConnectionStatus == WIFI_CONNECTING) {
             
                wifiConnectionStatus = WIFI_ABORT; 
                
            } 
            
            
            if (startsWith("R", menuRight[menuIdxRight])) { return EXITCODE_RUN; }
            
            if (startsWith("C", menuRight[menuIdxRight])) { // CANCEL
            
                strcpy(OLED[3], "   Cancelling   ");
                strcpy(OLED[5], "   ----------   ");
                
                updateGUI = 1;
                
                while ( wifiConnectionStatus == WIFI_ABORT ) { } // Wait until wifi connection cog actually exits (IMPORTANT!)
                
                //return EXITCODE_SEARCH; 
                
            } // if             

                        
     
     } // if
    
    
    
  } // while (wifiStatus == WIFI_SEARCHING)
  
  
  // ---
  
  //while(1) { }
  
  // If we get here, wifiConnection process has returned or user aborted
  // State could be "CONNECTED" or "NOTCONNECTED"
  // Go to the next menu!    
  return EXITCODE_WIFISTATUS;
  
  
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------

 
// -----------------------------------------------------------------------------------------------------------------------------------------------------------



int menu_wifiStatus() {
  
  //int fwvisible = 0;
  
  
  cogGUI = cogstart(GUI, NULL, STACK_GUI, sizeof(STACK_GUI));
  while (cogState_GUI != COG_RUNNING) { }
  
   
  //strcpy(OLED[0], "              <>");
  
  
  if (wifiConnectionStatus == WIFI_CONNECTED) {
        
        // Display SSID and IP, with connected message
          
        centerToOLEDbuf(activeSSID); // Truncate and center SSID for display
        strcpy(OLED[1], OLEDbuf);
          
        strcpy(OLED[3], "   Connected!   ");
        
      
        centerToOLEDbuf(activeIP); // use pointer to activeIP+3, which drops the status prefix
        strcpy(OLED[5], OLEDbuf);
  
  
  } else { // Not connected!
  
  
        strcpy(OLED[1], "    Badge-WX    ");
        
        if (justSearched == 1) {
          
            justSearched=0;
            strcpy(OLED[3], " WiFi Not Found ");
            
            
        } else {
          
            strcpy(OLED[3], "  WiFi Offline  ");
            
        }                  
            
            strcpy(OLED[5], "Try SEARCH menu!");
                             
  
  }         
  
  
  // override for wifi module off state
  if (wifiPowerStatus == WIFI_POWER_OFF) {
      strcpy(OLED[3], " WiFi Power Off "); 
      strcpy(OLED[5], "                "); 
  }       
  
 
 
  menuLeft_elements = 0;
  menuRight_elements = 0;
  
  if (validEEpromImage == 1) {
    
    
    if (disableAutoRun == 0) {
      
        strcpy(menuRight[menuRight_elements++], "RUN (9)");
        runIdx = 9; // enable
        disableAutoRun = 1; // Only allow to start once (first time user gets to this menu)
        
    } else {
      
        strcpy(menuRight[menuRight_elements++], "RUN");
    }
    
  
  }    
  
  // Yeah, this seems silly- but I want LOAD always after RUN, followed by SEARCH, then ADMIN last. (Not LOAD, ADMIN, SEARCH)
  if (wifiConnectionStatus == WIFI_CONNECTED) { 
      
      strcpy(menuRight[menuRight_elements++], "LOAD");
      strcpy(menuRight[menuRight_elements++], "SEARCH");
      strcpy(menuRight[menuRight_elements++], "ADMIN");
                      
  } else {
  
      if (wifiPowerStatus == WIFI_POWER_ON) { strcpy(menuRight[menuRight_elements++], "SEARCH"); }
      strcpy(menuRight[menuRight_elements++], "ADMIN");
      
  }  
  
  
  
  if (wifiPowerStatus == WIFI_POWER_ON) {
  
    strcpy(menuRight[menuRight_elements++], "WIFI-OFF"); 
    
        
  } else {
    
    strcpy(menuRight[menuRight_elements++], "WIFI-ON"); 
    
    
  }         
        
  if (menuRight_elements > 1) {
    strcpy(OLED[0], "              <>"); 
    //strncpy(OLED[0], "              <>", 16); 
  }    
  
  
  updateGUI = 1;
  
  /*if (disableAutoRun == 0) {
      runIdx = 9; // enable run countdown from 9
      disableAutoRun = 1; // Only allow autorun once; the first time user sees this menu
  
  }  */
 
 
  while(1) {
    
    
    
    // Handle button presses
    
    if (menuSelectedRight == 1) {
                
                  
            menuSelectedRight = 0;
            runIdx = -1; // disable

            if (startsWith("R", menuRight[menuIdxRight])) { return EXITCODE_RUN; }
            if (startsWith("L", menuRight[menuIdxRight])) { return EXITCODE_LOAD; }
            if (startsWith("S", menuRight[menuIdxRight])) { return EXITCODE_SEARCH; } 
            //if (startsWith("F", menuRight[menuIdxRight])) { return EXITCODE_FORGET; }
            if (startsWith("A", menuRight[menuIdxRight])) { return EXITCODE_ADMIN; } 
            
            
            if (startsWith("W", menuRight[menuIdxRight])) { 
            
            
                    clearMenus();
                  
                    strcpy(OLED[3], " Configuring... "); 
                    strcpy(OLED[5], "                ");
                  
                    /*cidx = 58; // ascii 9  (+1)
                    
                    while(--cidx > 48) {
                      OLED[5][8] = (cidx);
                      updateGUI = 1;
                    
                      waitcnt(CNT + CLKFREQ); // wait 1 second
                      
                    }*/

                   
                    
                    if (wifiPowerStatus == WIFI_POWER_ON) { 
                    
                        wifiPowerStatus = WIFI_POWER_OFF;
                        
                        return EXITCODE_DO_WIFIOFF;
                    
                    } else {
                      
                      
                        wifiPowerStatus = WIFI_POWER_ON;
                    
                        return EXITCODE_DO_WIFION;
                  
                    }                  
            
            
            } // W
            
            
            
            
            
              
     
     } // if (menuSelectedRight == 1)
     
     
     
     /*if (menuSelectedLeft == 1) {
      
        // Secret menu.... Show firmware version top left
        menuSelectedLeft = 0;
      
        if (fwvisible==0) {
          strcpy(OLED[0], "FW=1.3        <>");
          fwvisible=1;
        } else {
        
          strcpy(OLED[0], "              <>");
          fwvisible=0; 
        }
                    
        updateGUI = 1; 
       
     } */  

    if (menuSelectedLeft == 1) {

      menuSelectedLeft = 0;
      
      if (menuLeft_elements == 0) {
      
        strcpy(menuLeft[menuLeft_elements++], BL_VerString);  // Show bootloader version
        
                
      } else {
        
        // Hide bootloader version
        strncpy(menuLeft[--menuLeft_elements], "        ", 9); // Copy 8 spaces and 1 null byte
        
          
      }
      
      updateGUI = 1;        
    }                
       
    
  } // while (1)
 
 
 
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


int menu_wifiAdmin() {
  
  
  cogGUI = cogstart(GUI, NULL, STACK_GUI, sizeof(STACK_GUI));
  while (cogState_GUI != COG_RUNNING) { }
   
 
  strcpy(OLED[0], " Admin        <>");
  //strcpy(OLED[1], "    Badge-WX    ");
  //strcpy(OLED[1], " Administration ");
  strcpy(OLED[1], " -------------- ");
  //strcpy(OLED[3], "                ");
  //strcpy(OLED[4], "                ");
  //strcpy(OLED[5], "                ");                           
  //strcpy(OLED[6], "                ");
  
  // Left menu
  strcpy(menuLeft[0], "YES");
  menuLeft_elements = 1;
  
  
  // Right menu
  strcpy(menuRight[0], "NO");
  strcpy(menuRight[1], "CANCEL");
  //strcpy(menuRight[1], GC_CANCEL);
  menuRight_elements = 2;  
  
  volatile int sure = 0;
  
  // --
  
  // Clear EEPROM?
  strcpy(OLED[3], " ERASE EEPROM ? ");
  updateGUI = 1;
  
  
  while(1) {
    
    
    
    // Handle button presses
    
    if (menuSelectedLeft == 1) {
                
                  
            menuSelectedLeft = 0;
            
            // Must be YES!
            
            if (sure == 0) {
            
                strcpy(OLED[3], "  ERASE EEPROM  ");
                strcpy(OLED[5], " ARE YOU SURE ? "); 
                sure = 1;
                updateGUI = 1;
            
            } else { // already sure!
            
                strcpy(OLED[3], " ERASING EEPROM ");
                strcpy(OLED[5], "  PLEASE WAIT!  "); 
                updateGUI = 1;
                invalidateEEpromImage();
                break;            
            }                        
            
    } // if (menuSelectedLeft == 1)            
    
    
    
    if (menuSelectedRight == 1) {
                
                  
            menuSelectedRight = 0;
            
            if (startsWith("N", menuRight[menuIdxRight])) { break; }
            if (startsWith("C", menuRight[menuIdxRight])) { return EXITCODE_WIFISTATUS; }
                                    
     
     } // if (menuSelectedRight == 1)
       
    
  } // while (1)
  
  
  
  // ---
  
  
  
  sure = 0;
  
  // --
  
  // TODO: Could hide this option if not currently connected?
  //  if (wifiConnectionStatus == WIFI_CONNECTED) {
    
  
  // Logout from WiFi Network (Disconnect)?
  strcpy(OLED[3], " LOGOUT WIFI ?  ");
  strcpy(OLED[5], "                ");
  updateGUI = 1;
  
  
  while(1) {
    
    
    
    // Handle button presses
    
    if (menuSelectedLeft == 1) {
                
                  
            menuSelectedLeft = 0;
            
            // Must be YES!
            
            if (sure == 0) {
            
                strcpy(OLED[3], "  LOGOUT WIFI   ");
                strcpy(OLED[5], " ARE YOU SURE ? ");
                sure = 1; 
                updateGUI = 1;
            
            } else { // already sure!
            
                strcpy(OLED[3], "LOGGING OFF WIFI");
                strcpy(OLED[5], "  PLEASE WAIT!  "); 
                updateGUI = 1;
                
                return EXITCODE_LOGOUT;            
            }                        
            
    } // if (menuSelectedLeft == 1)            
    
    
    
    if (menuSelectedRight == 1) {
                
                  
            menuSelectedRight = 0;
            
            if (startsWith("N", menuRight[menuIdxRight])) { break; }
            if (startsWith("C", menuRight[menuIdxRight])) { return EXITCODE_WIFISTATUS; }
                                    
     
     } // if (menuSelectedRight == 1)
       
    
  } // while (1)
  
  
  
  // ---
  
  // All done, return to WiFi Status menu
  return EXITCODE_WIFISTATUS;
  
 
}




// -----------------------------------------------------------------------------------------------------------------------------------------------------------


int DO_WIFIOFF() {
  
    
    wifi_start(31, 30, 115200, WX_ALL_COM);

    wifi_print(CMD, NULL, "%cSLEEP:2,0\r", CMD); // Sleep without RFCAL set

    wifi_stop();
  

    //wifiClearAllCredentials();
    wifiConnectionStatus = NOT_CONNECTED;

    // All done, return to WiFi logout menu
    return EXITCODE_WIFISTATUS;
  
 
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


int DO_WIFION() {

    
      // Keep boot pin high to prevent autoload from wifi module
      high(WX_BOOT);
                                          
                        
      // Toggle reset pin, low then input
      low(WX_RES);
      waitcnt(CNT + CLKFREQ / 2000); // wait 1/2 millisecond
      set_direction(WX_RES, 0); //  
                        
                    
      // clear boot pin
      waitcnt(CNT + CLKFREQ / 4); // wait 1/4 second, allow ESP to reboot (typically 180ms)
      
      
      // Force sleep to return in default mode, waking by itself....
      wifi_start(31, 30, 115200, WX_ALL_COM);
      WiFiWaitForMode(STA+AP);
      waitcnt(CNT + CLKFREQ / 2);
      WiFiWaitForMode(STA);
      wifi_print(CMD, NULL, "%cJOIN:\r", CMD); // Attempt to join last-known network, with credentials stored in ESP module
      wifi_stop();
      
      
      set_direction(WX_BOOT, 0); // Reset IO14 as input after reboot
                    
                        
      wifiPowerStatus = WIFI_POWER_ON;          
      
                                                            
      return EXITCODE_RESTART; // Will try connecting with last known credentials
      

}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------



int AllowDownload() {
  
    
	//cogGUI = cog_run(GUI, STACK_GUI);
  cogGUI = cogstart(GUI, NULL, STACK_GUI, sizeof(STACK_GUI));
  while (cogState_GUI != COG_RUNNING) { }
  
    
    strcpy(OLED[0], "WiFi Programming");
    strcpy(OLED[1], "   activated!   ");
    
    
    
    
    if (moduleNameLen < 16) { // Fits on 1 line, so center align
    
          char *p = OLED[3] + ((16-moduleNameLen)/2); // Center module name; add offset x chars
          strncpy(p, moduleName, moduleNameLen);
    
    } else { // else need 2 lines; just display as-is      
    
          strncpy(OLED[3], moduleName, 16);
          char *p = moduleName + 16; 
          strncpy(OLED[4], p, moduleNameLen-16);
    }
      
    strcpy(menuRight[0], "   ABORT");
    menuRight_elements = 1;
    
    updateGUI = 1;
    
    
    high(WX_BOOT); // Set IO14 high to allow OTA updates
    low(LED_PIN);
    
    while (menuSelectedRight == 0) { 
    
    
         waitcnt(CNT + CLKFREQ/8);
         toggle(LED_PIN); 
    
    } // Wait for button press
    
    menuSelectedRight = 0;
    
    set_direction(WX_BOOT, 0); // Reset IO14 as input to cancel OTA updates
    
    set_direction(LED_PIN, 0); // LED off and release
    
        
    return EXITCODE_WIFISTATUS;
    
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void noWiFiError() {
  
     strcpy(OLED[2], "     No WiFi    ");
		strcpy(OLED[3], " Networks Found ");
		updateGUI = 1;
		
		waitcnt(CNT + CLKFREQ);
		waitcnt(CNT + CLKFREQ);
  
  
}  


int menu_wifiManualSearch() {
  
  
  // Start search in another cog, to allow progress bar and abort in main
  // However... check if already running, and abort it!
  /*if (wifiSearchStatus != SEARCH_COMPLETE) {
    wifiSearchStatus = SEARCH_ABORT;
    while ( wifiSearchStatus == SEARCH_ABORT ) { } // Wait until wifi connection cog actually exits (IMPORTANT!)  
  }*/ 
      
  //cogWiFi = cog_run(WiFiSearch, STACK_WIFI);
  cogWiFi = cogstart(WiFiSearch, NULL, STACK_WIFI, sizeof(STACK_WIFI));
  
  //cogGUI = cog_run(GUI, STACK_GUI);
  cogGUI = cogstart(GUI, NULL, STACK_GUI, sizeof(STACK_GUI));
  while (cogState_GUI != COG_RUNNING) { }
  
  
  // Disconnect existing WiFi network and clear ALL IP/SSID data before new search
  
  // TODO...
  
  
  // Menu
  menuLeft_elements = 0;
  
  //strcpy(menuRight[0], "RUN (9)");
  //strcpy(menuRight[1], "INFO");
  //strcpy(menuRight[2], "REFRESH");
  strcpy(menuRight[0], "CANCEL"); // TODO: This should only appear once WiFi is connected! Keep here for testing.
  //strcpy(menuRight[1], "REFRESH");
  
  menuRight_elements = 1;
  
  
  
  
  // Initiate search, showing results on screen for user to scroll across
  
  /*strcpy(OLED[2], "    Badge-WX    ");
  strcpy(OLED[4], "   Connecting   ");
  strcpy(OLED[5], "   ..........   ");*/
  
  strcpy(OLED[0], "    Badge-WX    ");
  strcpy(OLED[2], "  Searching for ");
  strcpy(OLED[3], "  WiFi networks ");
  strcpy(OLED[5], "  ............. ");
  updateGUI = 1;
 
  
  // Dot/Star scroll whilst waiting...
  
  dotidx = 2;
  dotdir = 1;
  //t = CLKFREQ + CNT;
  
  SYS_TIMER_MILLISECONDS = 1000;
  cogState_SYS_TIMER = COG_RUNNING;


  while (wifiSearchStatus != SEARCH_COMPLETE) { // Wait for search to complete 
    
		// TODO: Allow ABORT during this loop! Menu option!
		
		 if (cogState_SYS_TIMER == COG_ELAPSED) { // 1 second elapsed
			
			
			if (dotdir == 0) { 
			   
				if (dotidx > 2) {
				  
					OLED[5][dotidx] = '.';
					--dotidx;
					updateGUI = 1;
					//t = CLKFREQ + CNT;
	           SYS_TIMER_MILLISECONDS = 1000;
              cogState_SYS_TIMER = COG_RUNNING;		
   	
				} else {
				  
					dotdir = 1; // Change direction
					++dotidx;
				  
				}            
			  
			} else { // dotdir must be 1
			  
				if (dotidx < 15) {
				  
					//strcpy(OLED[5][dotidx], " ");
					OLED[5][dotidx] = '*';
					++dotidx;
					updateGUI = 1;
					//t = CLKFREQ + CNT;
              SYS_TIMER_MILLISECONDS = 1000;
              cogState_SYS_TIMER = COG_RUNNING;
				
				} else {
					
					dotdir = 0; // Change direction
					--dotidx;
					
				} 
				
			} // if (dotdir == 0) {  
		  
		} // if (t < CNT)
    

    
		// Handle button presses
		
		if (menuSelectedRight == 1) {
					  
						
					menuSelectedRight = 0;
					
						
					//if (startsWith("R", menuRight[menuIdxRight])) { return EXITCODE_SEARCH; } // Gonna come back here!
					if (startsWith("C", menuRight[menuIdxRight])) {  // Cancel the search!
					
					
						  if ( wifiSearchStatus != SEARCH_COMPLETE) {
							
								wifiSearchStatus = SEARCH_ABORT;
															
								strcpy(OLED[2], "   Cancelling   ");
								strcpy(OLED[3], "     Search     ");
								strcpy(OLED[5], "   ----------   ");
					
								updateGUI = 1;
					
								
								// TODO: What if state changed before assignment.... need some way to ensure this works as expected.... maybe "if search cog not active"!
								while ( wifiSearchStatus == SEARCH_ABORT ) { } // Wait until wifi connection cog actually exits (IMPORTANT!)
								
							   
						  }
												  
						  return EXITCODE_WIFISTATUS; 
						  
						  
					} // if (startsWith("C", menuRight[menuIdxRight]))
					
					
					
					
					//if (startsWith("S", menuRight[menuIdxRight])) { return EXITCODE_SEARCH; }   
										  
		 
		} // if (menuSelectedRight == 1)
		
		
		
	} // while wifiSearchStatus != SEARCH_COMPLETE
  
  
	  // ---
	  
	  if (wifiFoundCount == 0) {
		
		// No networks found, so return back to generic wifiStatus menu
		// Set flag for specifc message on screen (of this event)
		justSearched = 1;
		
		/*strcpy(OLED[2], "     No WiFi    ");
		strcpy(OLED[3], " Networks Found ");
		updateGUI = 1;
		
		waitcnt(CNT + CLKFREQ);
		waitcnt(CNT + CLKFREQ);*/
		
     noWiFiError(); 
		return EXITCODE_WIFISTATUS;
		
		
	  }
   
   
	  
	  // ---
   
    // Important 1ms pause (TODO: maybe old requirement...)
    //
    
    waitcnt(CNT + CLKFREQ/1000);
    
    // ---
    
    
    
	  // -------------------------------------------
  

	  // WiFi Search Completed
	  
	  // Update screen with results
	  // Allow scroll through the networks
	  
	    
	  
	  clearOLED();
     
  
  
  
	  // TODO: This displays the Found Network Count.... remove for production code! ... Or position somehow nicely!
	  
	  // Clear buf
	  strncpy(OLEDbuf, "                ", 17); // Copy 16 spaces and 1 null byte
	  
	  
	  // TODO: Location of WIFI FOUND COUNT!
	  
	  SPUTL(wifiFoundCount, 10, 16, ' ', OLEDbuf); // SPUTL(unsigned long u, int base, int width, int fill_char, char *obuf)
	  strcpy(OLED[1], OLEDbuf);
	  //updateGUI = 1;
	  
	  //
  
    

    // Add CONNECT button
    strcpy(menuLeft[0], "CONNECT");
    //strcpy(menuLeft[0], "CANCEL");
    menuLeft_elements = 1;
    menuIdxLeft = 0;
                  
    updateGUI = 1;     
 
 
 
	// ---
  
	
  
	// ---
  
	ssidIdx = 0;
	ssidIdx_last = 1; // ensure display updated in first loop
	force = 1;
  
	wifi_start(31, 30, 115200, WX_ALL_COM);
  
 
  
  
  // Clear nav flags
  navLL = 0;
  navLR = 0;
  
  
  // Loop doing APGET and updating screen with each nav key press
  while (1) {
    
    
      
      while ((force == 0) && (navLL == 0) && (navLR == 0)) {
        
        
            // Handle button presses if nothing else going on
            
      
            if (menuSelectedRight == 1) {
                    
                      
                  menuSelectedRight = 0;
                  
                  wifi_stop(); // ! IMPORTANT !
                  
                  return EXITCODE_WIFISTATUS;  // CANCEL
      
            
            } else if (menuSelectedLeft == 1) {
              
              
                  menuSelectedLeft = 0;
                  
                  wifi_stop(); // ! IMPORTANT !
                  
                  validPassword = 0;
                  return EXITCODE_CONNECT;
              
            }              
      
		  } // while ((force == 0) && (navLL == 0) && (navLR == 0))
      
      
      
		// Update display with next SSID/etc... if scroll button pressed 
	  
		
		if (navLL == 1) {
				--ssidIdx;
		} else if (navLR == 1) {
				++ssidIdx;
		}
          
          
          
        // Limits    
        ssidIdx = (ssidIdx < 0) ? 0 : ssidIdx;
        ssidIdx = (ssidIdx > wifiFoundCount-1) ? wifiFoundCount-1 : ssidIdx;
		
		
		
		// If ssidIdx has changed, then update display
		if ( (ssidIdx_last != ssidIdx) || (force == 1) ) { // Still use force here, as user might have been tiddling the nav keys during the search, changing the index :)
			
			   ssidIdx_last = ssidIdx;
			
          
          // Clear memory 
				// TODO: May not be needed, if scan adds null char to end of strings ?)
				memset(_apSSID, '\0', sizeof(_apSSID));  // Clear to null
				memset(_apPASS, '\0', sizeof(_apPASS));  // Clear to null
				_apStatusi = 0;
				_apEnc = 0;
				_apRSSI = 0;
			
			
				
    	     // APGET
          // Note: This must return status "S" to be valid. Ignore status "N" , or "E" and try again
          int tries = 5;
          while (--tries >= 0) {   
				
              wifi_print(CMD, NULL, "%cAPGET:%d\r", CMD, ssidIdx);
              wifi_scan(CMD, NULL, "%1s%d%d%1s%s", &_apStatusi, &_apEnc, &_apRSSI, &_apSSID, &_apSSID); // SSID twice to remove comma
				  
              if (_apStatusi == 'S') { break; }
              
          }
          
          if (_apStatusi != 'S') {
                
            
              /*strcpy(OLED[2], "     No WiFi    ");
    		      strcpy(OLED[3], " Networks Found ");
    		      updateGUI = 1;
    		
    		      waitcnt(CNT + CLKFREQ);
    		      waitcnt(CNT + CLKFREQ);*/
    		
              noWiFiError(); 
    		      return EXITCODE_WIFISTATUS;    
            
            
            
           }                     
				   
				   /*
				   
					 if (cogState_GUI == COG_RUNNING) {
						  
								cogState_GUI = COG_STOP;
								while (cogState_GUI != COG_STOPPED) { } // Wait for GUI to actually stop
					   waitcnt(CNT + CLKFREQ/1000);
					  
					 } 
				  
				  
					clear();
								
					cursor(0,3);
					oledprint("%c", _apStatusi); // S
					cursor(0,4);
					oledprint("%d", _apEnc); // 4
					cursor(0,5);
					oledprint("%d", _apRSSI); // 209
					cursor(0,6);
					oledprint("%s", _apSSID); // OfficeWiFi
				  
							
					while (1) { }*/
				  
				  
								   
				  
				// ------------------------------------------------------------------
				// Display
				// ------------------------------------------------------------------
				  
				  
				  
				// RSSI
				  
				strncpy(OLEDbuf, "                ", 17); // Clear buf // Copy 16 spaces and 1 null byte
				int rssi = (_apRSSI * 100) / 255;
				SPUTL(rssi, 10, 15, ' ', OLEDbuf); // SPUTL(unsigned long u, int base, int width, int 
				strcpy(OLED[2], OLEDbuf);
				OLED[2][15] = '%';
				  
				  
				  
				// Locked/Unlocked status
				strncpy(OLED[2], encName[_apEnc], 4);
				  
				  
				  
				// SSID name
				centerToOLEDbuf(_apSSID); // Truncate and center SSID for display
				strcpy(OLED[4], OLEDbuf);
				  
				  
				  
				// v.1   Update Top cursor, based on current idx
				/*if (wifiFoundCount > 0) {
				   
					if (ssidIdx == 0) {
						
						strcpy(OLED[0], " WiFi>        <>");
					  
					} else if (ssidIdx == wifiFoundCount-1) {
						
						strcpy(OLED[0], "<WiFi         <>");
					  
					} else {
						
						strcpy(OLED[0], "<WiFi>        <>");
					} 
						   
				} */
    
    
    
         // v.2   Update Top cursor, based on current idx
				/*if (wifiFoundCount > 0) {
				   
					if (ssidIdx == 0 && wifiFoundCount == 1) {
						
						strcpy(OLED[0], " WiFi           ");
            
            }  else if (ssidIdx == 0) {
						
						strcpy(OLED[0], " WiFi>          ");
					  
					} else if (ssidIdx == wifiFoundCount-1) {
						
						strcpy(OLED[0], "<WiFi           ");
					  
					} else {
						
						strcpy(OLED[0], "<WiFi>          ");
					} 
						   
				} */ // ELSE no WiFi found... hide menu   
    
    
        // v.3   Update Top cursor, based on current idx
				if (wifiFoundCount > 1) {
				   
					if (ssidIdx == 0) {
						
						//strcpy(OLED[0], " WiFi>");
						strncpy(OLED[0], " WiFi>", 6); 
					  
					} else if (ssidIdx == wifiFoundCount-1) {
						
						//strcpy(OLED[0], "<WiFi "); // Keep last space - fixed width - else prior char won't be cleared
              strncpy(OLED[0], "<WiFi ", 6);
					  
					} else {
						
						//strcpy(OLED[0], "<WiFi>");
              strncpy(OLED[0], "<WiFi>", 6);
      
					} 
						   
				}  // ELSE no WiFi found... no need to show menu  
    
               
				  
				  
				// Redraw display now
				updateGUI = 1;
				  
				  
				// ---
				  
				waitcnt(CNT + CLKFREQ/3); // Important! Set delay appropriate for human preference!... prevent AP's flying past too fast!
				
		} // if (ssidIdx_last != ssidIdx)
           
                
        // Clear nav flags
		
        navLL = 0;
        navLR = 0;
        force = 0; // Ensures 1st AP immediately displayed
    
	} // while (1) { 
  
} // menu_wifiManualSearch()


// -----------------------------------------------------------------------------------------------------------------------------------------------------------


void WiFiSearch(void *par) {
  
  // TODO: scan/poll required after fire-and-forget instructions, like cmd-event/etc.. ?
  //        - would be nice to remove all that potentially cog-locking fluff.
  
  
  wifiSearchStatus = SEARCH_SEARCHING;
  
  //wifiScanDone =   0;
  wifiFoundCount = 0;
  
  wifi_start(31, 30, 115200, WX_ALL_COM);
  
  
  // ----------------------------------------------------------
  
  
  // Clear stored credentials... NO!
  // Only clear when login to new network, not just because we search!
  //wifiClearAllCredentials();
      
    
  // ----------------------------------------------------------
  
  
  WiFiWaitForMode(STA_AP);  // Important! Required for WiFi search
      
  
  // ----------------------------------------------------------
  
     
  /*
  event = 'X';
  while ( (event != 'S') && (wifiSearchStatus == SEARCH_SEARCHING) ) {
      
      wifi_print(CMD, NULL, "%cSET:cmd-events,1\r", CMD); // Enable event reporting
      wifi_scan(CMD, NULL, "%1s%d", &event, &id); // Expect: S,0
      
  } 
  */
  
  // ----------------------------------------------------------
    
  int apscanCount = 0;
    
  // Note: If calling APSCAN after a user has cancelled a previous search, then it will block on the first APSCAN call!
      
  while ( (wifiSearchStatus == SEARCH_SEARCHING) && (apscanCount<2) ) { // Scan must take longer than 1/2 second, or try again!
      
      apscanCount = 0;
      
      
      event = 'X';
      while ( (event != 'S') && (wifiSearchStatus == SEARCH_SEARCHING) ) {
          
          wifi_print(CMD, NULL, "%cAPSCAN\r", CMD); // Initiate WiFi scan
          wifi_scan(CMD, NULL, "%1s%d", &event, &id); // Expect: S,0
          
          //waitcnt(CNT + CLKFREQ/4); // Moderate pause- don't need to keep polling rapidly
          
      }
       
      
      // -----------------------------------------------------------
  
  
  
  
      
      //wifi_print(CMD, NULL, "%cAPSCAN\r", CMD); // Initiate WiFi scan
      //wifi_scan(CMD, NULL, "%s", _scanReply); // Expect: S,0
      /*wifi_poll(&event, &id, &handle); 
      
      cursor(0,2);
      oledprint("%c  %d  %d", event, id, handle);*/
      
      
        
      // ... then after about 2 seconds of scan time...
      // Expect event (poll): !A,1,0
      // Where ! is the event prefix, A the event type, 1 is the count of discovered WiFi networks, and 0 is fixed always zero 
      // sscp_send(prefix, "A,%d,0", scanCount);
        
      event = 'X';  
      
      while ( (event != 'A') && (wifiSearchStatus == SEARCH_SEARCHING) ) {
    
            waitcnt(CNT + CLKFREQ/4); // Moderate pause- don't need to keep polling rapidly as event comes after about 2 seconds min, and maybe 6-8 seconds maximum.
            
            wifi_poll(&event, &id, &wifiFoundCount); // event = 'A', handle = %d (count of found AP's)
            
            apscanCount++; // Scan must take longer than 1/2 second, or try again!
            
            // Note: Could scan the details here, but no need as handle already contains the AP count
            //wifi_scan(CMD, NULL, "%1s%d%s", &event, &id, _scanReply); // Expect: A,x,0
            
       }
       
       
       
  }  
  
  
  
  
  // Really MUST go back to STA mode... keep this even when aborting
  // If the cog totally fails we might consider reseting the ESP module
  
  
  WiFiWaitForMode(STA); // Important! 
  
  
  // ---
  
  
  
  // ----------------------------------------------------------
  
  
   
  
    
  
  wifi_stop();
  
    
  
  //wifiScanDone = 1; // Calling cog will block until this flag set
  wifiSearchStatus = SEARCH_COMPLETE; 
 
  /*cog_end*/cogstop(cogWiFi);
 
} 


// -----------------------------------------------------------------------------------------------------------------------------------------------------------






// -----------------------------------------------------------------------------------------------------------------------------------------------------------




int menu_wifiConnect() {
  
  
  
  // if password required for current network, then return exitcode-needpassword!
  
  if ((_apEnc > 0) && (validPassword == 0)) {  // 0 = AUTH_OPEN, all others require password!
  
        return EXITCODE_NEEDPASSWORD; 
        
  }
  
  
  
  // Reset flag
  wifiConnectionStatus = WIFI_CONNECTING;
  
  // Clear ip and ssid
  wifiClearActiveCredentials();
  
  
  

  //wifi_stop();
  wifi_start(31, 30, 115200, WX_ALL_COM);
  

  
  
  // Disconnect from current network by switching to AP mode
  // Note: This is not always necessary! Calling wifi_join also works most times without a prior disconnect
  // but in development I had some cases where it didn't work, so keep here until further investigation possible.
  // Interesting... wifi_join sets the module to 'STA+AP' mode momentarily, so that probably doesn't drop the old connection
  // like setting mode to AP would.
  // the ESP module to STA+AP mode, which appears to allow a new connection
  WiFiWaitForMode(AP);  // Important!
  
  
  // Join new network
  wifi_join(_apSSID, _apPASS); // Join SSID current selected by SEARCH (ESP will swap to STA+AP mode, join, then return in STA mode)
  
  
  // TODO: If fails to connect, invalidate stored password? validPassword = 0;
  
  
  
  
  //high(LED_PIN);
  
  // Note: Don't check if this worked or not, as the return with exitcode_restart will check and handle appropriately
  
  wifi_stop();
  
  // ? Wait for connection, before calling wifi_stop ?
  
  //while(1) { }
  
  return EXITCODE_RESTART; // This will go get the IP and SSID, then update wifiConnectionStatus, activeSSID and activeIP if successful.
  
  
}





// -----------------------------------------------------------------------------------------------------------------------------------------------------------





int menu_wifiGetPassword() {
      
      
      //cogGUI = cog_run(GUI_ABC, STACK_GUI);
      cogGUI = cogstart(GUI_ABC, NULL, STACK_GUI, sizeof(STACK_GUI));
      
      memset(_apPASS, '\0', sizeof(_apPASS));  // Clear to null
      
      while (cogState_GUI != COG_RUNNING) { }
            
      
      strcpy(OLED[0], "<ABC>  <DEL-INS>");
      strcpy(OLED[2], "Enter Password: ");
      
      // 4 will be password line
      // 5 will be underscore to indicate cursor position
      // Keep 3 and 6 blank for clarity
  
      strcpy(menuLeft[0], "CHARSET");
      menuLeft_elements = 1;
      
      // menuRight[0] MUST MUST BE "OK" option. GUI_ABC toggles that between OK and CANCEL!
      strcpy(menuRight[0], "OK"); // TODO: How to implement cancel ? (IF ALL DELETED !! passlen == 0)
      //strcpy(menuRight[1], "CANCEL");
      menuRight_elements = 1;
      

      //strcpy(ABC, ABC_upper);
      
      scrollLeft = ABCMODE; // Set left nav wheel to scroll the abc block
      //menuIdxCenter = 0;
      //menuCenter_elements = strlen(ABC);
      
      menuIdxMax = menuIdxABCMax;
    
      updateGUI = 1;
      
      
        
      while (1) {
      
      
      // TODO: handle button press, and swap ABC
      if (menuSelectedLeft == 1) {
            
          menuSelectedLeft = 0;
          runIdx = -1; // disable
      
          // Charset toggle pressed
          
          if (ABC_charset == 0) {
                                  
            ABC_charset = 1;
            
            //strcpy(ABC, ABC_lower);
            strncpy(ABC, ABC_lower, 37);
            strcpy(OLED[0], "<abc>  <DEL-INS>");
                        
          } else if (ABC_charset == 1) {
            
            lastABCIdx = menuIdxABC; // Grab idx for next time user enters "ABC" charset
            menuIdxABC = lastOtherIdx; // Position cursor at previous idx
            menuIdxMax = menuIdxOtherMax;
            
            ABC_charset = 2;
            
            //strcpy(ABC, ABC_other);
            strncpy(ABC, ABC_other, 34);
            strcpy(OLED[0], "<_#%>  <DEL-INS>");
             
             
          } else { //  if (ABC_charset == 2) {
            
            lastOtherIdx = menuIdxABC; // Grab idx for next time user enters "other" charset
            menuIdxABC = lastABCIdx; // Position cursor at previous idx
            menuIdxMax = menuIdxABCMax;
            
            ABC_charset = 0;
            
            //strcpy(ABC, ABC_upper);
            strncpy(ABC, ABC_upper, 37);
            strcpy(OLED[0], "<ABC>  <DEL-INS>");
            
            
          }
                        
          
          updateGUI = 1;         
          
    } // if (menuSelectedLeft == 1)
    
    
    // -----
    
    // 
    
    
    if (menuSelectedRight == 1) {
                
                  
          menuSelectedRight = 0;
          runIdx = -1; // disable
          
          
          if (menuRight[0][0] == 'O') {
            
            
            // OK pressed
			
			validPassword = 1;
            
            return EXITCODE_CONNECT;
            
            
            
            
        } else { // if (menuRight[0][0] == 'C') {
          
         
          // CANCEL pressed 
          
          // Return to main menu
          
          return EXITCODE_WIFISTATUS; // REFRESH;
          
          
          
        }                              
            
            
            
    }     
    
    
    
    
    
    
    
  
  } // while (1)
      
      
      
      
      
      
  
}


// -----------------------------------------------------------------------------------------------------------------------------------------------------------






  

// -----------------------------------------------------------------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------------------------------------------------------------------



// -----------------------------------------------------------------------------------------------------------------------------------------------------------  
 
 

// -----------------------------------------------------------------------------------------------------------------------------------------------------------


  

  
// -----------------------------------------------------------------------------------------------------------------------------------------------------------  
// -----------------------------------------------------------------------------------------------------------------------------------------------------------  




// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------










// -----------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Other Cogs
//
// -----------------------------------------------------------------------------------------------------------------------------------------------------------




/*
void bothLedsON() {
       
   cogState_LED = COG_RUNNING;
   
   while(cogState_LED == COG_RUNNING) {
   
      high(LED_PIN);
      low(LED_PIN);
   
   }
      
     
   input(LED_PIN);
   
   cog_end(cogLED); // Trash this cog  
  
}  
*/





// -----------------------------------------------------------------------------------------------------------------------------------------------------------  


void SYS_TIMER(void *par) {
  
  // Set SYS_TIMER_MILLISECONDS with countdown value.
  // Then set cogState_SYS_TIMER = COG_RUNNING, to set things off or re-arm
	
	// Check for: cogState_SYS_TIMER == COG_ELAPSED   or   end the timer by setting SYS_TIMER_SECONDS = COG_ELAPSED
  
  int milliseconds = 0;
  cogState_SYS_TIMER = COG_ELAPSED;
  
  while(1) {
    
    while(cogState_SYS_TIMER == COG_ELAPSED) { } // Wait till timer flag armed
    
    milliseconds = SYS_TIMER_MILLISECONDS + 1; // Grab HUB global value
    
    cogState_SYS_TIMER = COG_BUSY;
    
    while((cogState_SYS_TIMER == COG_BUSY) && (--milliseconds > 0)) {  
      waitcnt(CNT + CLKFREQ / 1000); // wait 1 millisecond  
    }    
    
    // if running, re-arm with new timer value.
    // if milliseconds = 0 then elapse
    
    if (milliseconds == 0) { cogState_SYS_TIMER = COG_ELAPSED; } 
      
  }        
    
} 


// -----------------------------------------------------------------------------------------------------------------------------------------------------------  


void GUI_TIMER(void *par) {
  
  // Set GUI_TIMER_MILLISECONDS with countdown value.
  // Then set cogState_GUI_TIMER = COG_RUNNING, to set things off or re-arm
	
	// Check for: cogState_GUI_TIMER == COG_ELAPSED   or   end the timer by setting GUI_TIMER_SECONDS = COG_ELAPSED
  
  int milliseconds = 0;
  cogState_GUI_TIMER = COG_ELAPSED;
  
  while(1) {
    
    while(cogState_GUI_TIMER == COG_ELAPSED) { } // Wait till timer flag armed
    
    milliseconds = GUI_TIMER_MILLISECONDS + 1; // Grab HUB global value
    
    cogState_GUI_TIMER = COG_BUSY;
    
    while((cogState_GUI_TIMER == COG_BUSY) && (--milliseconds > 0)) {  
      waitcnt(CNT + CLKFREQ / 1000); // wait 1 millisecond  
    }    
    
    // if running, re-arm with new timer value.
    // if milliseconds = 0 then elapse
    
    if (milliseconds == 0) { cogState_GUI_TIMER = COG_ELAPSED; } 
      
  }        
    
}  
 


// -----------------------------------------------------------------------------------------------------------------------------------------------------------  



	  
void GUI(void *par) {
  
	 
    // Write to OLED, and handle button presses
	 // Run in cog to ensure user interface snappiness!
  
    cogState_GUI = COG_STARTING;
    
 
    runIdx = -1; // disable
    updateGUI = 0;
    
    scrollLeft = LEFT;
    
    clearOLED();
    clearMenus();
  
    
    // Remove clear of actual screen to improve experience... Ensure all char arrays are always cleared instead.
    //clear(); // Clear actual OLED display
    
    
    
    
 
    text_size(SMALL);
  
    //menuIdxLeftMin = 0;
    menuIdxLeftMax = 0;
    
    //menuIdxRightMin = 0;
    menuIdxRightMax = 0;
    
  
            
    _lastMenuIdxLeft = menuIdxLeft;
    _lastMenuIdxRight = menuIdxRight;
   
    
    lastRunIdx = -1;
    
    
    //ggTimeout = CNT + 160000000; // 2 second ggTimeout
    GUI_TIMER_MILLISECONDS = 2000;
    cogState_GUI_TIMER = COG_RUNNING;
    
    
    
    // Init complete, set status flag
    cogState_GUI = COG_RUNNING;
        
    
    
    // Loop until stop flag set
    while(cogState_GUI == COG_RUNNING) { 
    
        
        // ---
              
        
        menuIdxLeftMax = menuLeft_elements - 1; // Keep here so menu can be updated on the fly; TODO: Replace with auto-loop (no need to store elements count manually!)
        menuIdxRightMax = menuRight_elements - 1;
        
        
        
        // ---
        
              
        
        
        
        
        // ---
        
        
        
        // ---
        
        
        
		    
    		if ( updateGUI == 1 ) { 
    		
    			  updateGUI = 0;
    			
          oledWriteMenus(menuIdxLeft, menuIdxRight);
          
         
                                
    			  updateOLED();
       
        //while(1) { }
       
    		}
      
      
      
        // ---
        
        // scrollLeft = CENTER
        
        // ---
        
        
    //if (disableAutoRun == 0) {
        
        // Special case for RUN
        
        if ((lastRunIdx == -1) && (runIdx > -1)) {
          
            // Start the countdown, reset timer!
            //ggTimeout = 160000000 + CNT; // 2 second ggTimeout
            GUI_TIMER_MILLISECONDS = 2000;
            cogState_GUI_TIMER = COG_RUNNING;
          
        }
        
        lastRunIdx = runIdx;
                  
          
          // Reset RUN countdown?
          
          if ( runIdx > -1 ) { // Set to -1 to disable the countdown
                
              if ( menuIdxRight != 0) { // RUN menu not in view, so disable autorun and update menu name
                      
                      runIdx = -1;
                      //menuRight[0][5] = 48 + runIdx; // Insert run digit; add 48 for ASCII conversion 
                      strcpy(menuRight[0], "RUN");
                       
                      oledWriteMenuRight(menuIdxRight);
                      //ggTimeout = CNT + 160000000; // 2 second ggTimeout
              }
              
              else if ((menuIdxRight == 0) && (runIdx > 0) && (/*CNT > ggTimeout*/cogState_GUI_TIMER == COG_ELAPSED)) { // Time elapsed whilst on RUN menu, so countdown idx
                
                      --runIdx;
                      menuRight[0][5] = 48 + runIdx; // Insert run digit; add 48 for ASCII conversion 
                      
                      
                      
                      if (runIdx == 0) {
                          
                          //cogState_GUI = COG_STOPPED;
                          //doRUN();
                          
                          menuSelectedRight = 1; // Simulate click event on RUN
                          cogState_GUI = COG_STOP;
                      
                      } else {                      
                      
                          oledWriteMenuRight(menuIdxRight);
                      
                      }  
                      
                      //ggTimeout = 80000000 + CNT; // Reset to 1 second ggTimeout                        
                      GUI_TIMER_MILLISECONDS = 1000;
                      cogState_GUI_TIMER = COG_RUNNING;
              }
          
          } // if ( runIdx > -1 )                    
         
          
    //} //  if (disableAutoRun == 0)          
		
		  
        // ---
        
        
        
        if ( (scrollLeft == LEFT) && (_lastMenuIdxLeft != menuIdxLeft) ) {
          
            _lastMenuIdxLeft = menuIdxLeft;
            oledWriteMenuLeft(menuIdxLeft);
            
            
            waitcnt(CNT + CLKFREQ/3); // Important! Set delay appropriate for human preference!
          
        
        }
        
        
        
        if ( (scrollRight == RIGHT) && (_lastMenuIdxRight != menuIdxRight) ) {
          
            _lastMenuIdxRight = menuIdxRight;
            oledWriteMenuRight(menuIdxRight);
            
            waitcnt(CNT + CLKFREQ/3); // Important! Set delay appropriate for human preference!
          
        }         
         
        
        /*if ( (_lastMenuIdxLeft != menuIdxLeft) || (_lastMenuIdxRight != menuIdxRight) ) {
          
          _lastMenuIdxLeft = menuIdxLeft;
          _lastMenuIdxRight = menuIdxRight;

          //updateOLED_row(7);
          oledWriteMenus(menuIdxLeft, menuIdxRight);
                      
                    
          waitcnt(CNT + CLKFREQ/3); // Important! Set delay appropriate for human preference!
          
        } // if 
        */
        
        
        
        // ---
        
                 
        
        
        
        
        // ---
    
    
        if (button(L_NAV_LEFT)) {
          
           navLL = 1; 
           
          
                                      
                --menuIdxLeft;
                      
                menuIdxLeft = (menuIdxLeft < menuIdxLeftMin) ? menuIdxLeftMax : menuIdxLeft;
                  
           
           
           
                              
        }
        
        if (button(L_NAV_RIGHT)) {
          
           navLR = 1;
           
          
                          
                ++menuIdxLeft;
           
                menuIdxLeft = (menuIdxLeft > menuIdxLeftMax) ? menuIdxLeftMin : menuIdxLeft;
                
                     
           
        }       
        
        // ---
        
        if (button(R_NAV_LEFT)) {
           
           navRL = 1;
           
           --menuIdxRight;
           
           menuIdxRight = (menuIdxRight < menuIdxRightMin) ? menuIdxRightMax : menuIdxRight;
                
                
           
        }
        
        
        if (button(R_NAV_RIGHT)) {
           
           navRR = 1;
           
           ++menuIdxRight;
           
           menuIdxRight = (menuIdxRight > menuIdxRightMax) ? menuIdxRightMin : menuIdxRight;
                
                  
           
        }
        
        
        if (button(L_NAV_PRESS)) {
          
            
            navLP = 1;
            
            // Wait until not pressed
            while(button(L_NAV_PRESS)) {}
            
            menuSelectedLeft = 1;
          
        }
        
        if (button(R_NAV_PRESS)) {
          
            navRP = 1;
            
            // Wait until not pressed
            while(button(R_NAV_PRESS)) {}
            
            menuSelectedRight = 1;
          
        }       
        
        // --- 

			
    
    
    }  // while(1)  


  cogState_GUI = COG_STOPPED;
  /*cog_end*/cogstop(cogGUI);


} // GUI()





// -----------------------------------------------------------------------------------------------------------------------------------------------------------  







// 2nd version of GUI, which handles password entry
void GUI_ABC(void *par) {
  
    
    cogState_GUI = COG_STARTING;
    
    
    
    
    // Write to OLED, and handle button presses
    // Run in cog to ensure user interface snappiness!
    
      
    //cursorCharON = 5; // '-'; // 0; // Solid block
    //cursorCharOFF = ' '; // 7; // Outline
    cursorCharLast = cursorCharON;
    
    //menuIdxABCMin = 0;
    
    lastMenuIdxABC = menuIdxABC;
    //volatile int currentABCRow = 4; // use row 4 and 5, this int holds the starting row
    
    cursorIdx = 0; // Current cursor index, 0-15
    cursorIdxLast = 0;
    
    //cursorRowMin = 5;
    //cursorRowMax = 6;
    
    cursorRow = 5; // Current cursor row, 5 or 6 (NOTE: ABC chars will be cursorRow-1)
    
    passwordIdx = 0; // Current password index, 0-31
    passwordIdxLast = 0;
        
    oneTimeUpdate = 1;
    
    //volatile int abcIdx = 0;
    //volatile int passlen = 0;
    //volatile int cursorRow = 4;
    
    clearOLED();
    clearMenus();
    
    
    text_size(SMALL);
  
    
    menuIdxABC = 0;
    cursorIdx = 0;        
        
    //long ggTimeout = CNT + 160000000; // 2 second ggTimeout
    //ggTimeout = CNT + 80000000;
    GUI_TIMER_MILLISECONDS = 1000;
    cogState_GUI_TIMER = COG_RUNNING;
    
    

           
        
    // Init complete, set status flag
    cogState_GUI = COG_RUNNING;
        
    
    
    // Loop until stop flag set
    while(cogState_GUI == COG_RUNNING) { 
    
        
        // ---
              
        // Update current cursor position based on length of stored password
        // This assumes max 2 line password... future adjustment required for 64 char passwords
        cursorIdx = (passwordIdx > 15) ? (passwordIdx - 16) : passwordIdx;
        cursorIdx = (cursorIdx < 0) ? 0 : cursorIdx; // Don't let passwordIdx of -1 spoil the display
        cursorRow = (passwordIdx > 15) ? cursorRowMax : cursorRowMin;
               
        // ---
        
        
        // Handle cursor blinking effect      
        
        if ( cogState_GUI_TIMER == COG_ELAPSED /*(CNT > ggTimeout)*/ ) {
          
         
              cursorCharLast = (cursorCharLast == cursorCharON) ? cursorCharOFF : cursorCharON;                                          
              
              cursor(cursorIdx, cursorRow); // Blink cursor on row under the current ABC row
              oledprint("%c", cursorCharLast);
              
              //ggTimeout = CNT + 40000000; // Reset ggTimeout
              GUI_TIMER_MILLISECONDS = 500;
              cogState_GUI_TIMER = COG_RUNNING;
                 
        }          
        
        
        // ---
        
        
        // Update menu if transistioning around first password char
                
        if ( (cursorIdxLast != cursorIdx) /*&& (passwordIdx > -1) && (passwordIdx < 32)*/ ) { // Cursor has moved!
          
          
          /*if ( (cursorIdx == 0) && (cursorRow == cursorRowMin) && (_apPASS[0]=='\0') ) { // If cursor idx now 0, then change Right menu to CANCEL!
               
                strcpy(menuRight[0], "CANCEL");
                oledWriteMenuRight(0);
          }
          
          if ( (cursorIdxLast == 0) && (cursorRow == cursorRowMin) && (cursorIdx > 0) ) { // If idx changed from 0 to 1, then change Right menu to OK         
              
                strcpy(menuRight[0], "OK");
                oledWriteMenuRight(0);
                
          }*/
         
          
          waitcnt(CNT + CLKFREQ/3); // Important! Set delay appropriate for human preference! 
          cursorIdxLast = cursorIdx;
          
        } 
        
        
        if ( (passwordIdxLast == 0) && (passwordIdx < 0) ) {
        
                // change Right menu to CANCEL!
                strcpy(menuRight[0], "CANCEL");
                //strcpy(menuRight[0], GC_CANCEL);
                
                oledWriteMenuRight(0);
        
        } else if ( (passwordIdxLast < 0) && (passwordIdx == 0) ) {      
          
                //change Right menu to OK
                strcpy(menuRight[0], "OK");
                oledWriteMenuRight(0);
                
                waitcnt(CNT + CLKFREQ/3); // Important! Set delay appropriate for human preference! 
        }        
        
        
        
        /*if ( (passwordIdxLast < 0) && (passwordIdx == 0) ) { //  != passwordIdx) {
       
        //if (cursorIdx + passwordIdx == 0) {   // catch the condition when going from -1 to 0 idx, which skipped the previous delay
            
            waitcnt(CNT + CLKFREQ/3); // Important! Set delay appropriate for human preference! 
          
          
        } */            
        
        passwordIdxLast = passwordIdx;
        
        
        // ---
        
        
        // Redraw display ?
		    
    		if ( updateGUI == 1 ) { 
    		
    			updateGUI = 0;
    			
          oledWriteMenus(0, 0); // Only 0 idx in ABC mode :)
          
          //OLED[cursorRow][cursorIdx] = ABC[menuIdxABC]; 
          
          
          // Store latest char-change to password string
          //++passwordIdx;
          //_apPASS[passwordIdx] = ABC[menuIdxABC];
          
          
          // Also update display with current cursor, so that scrolled in chars are refreshed (especially when swapping charset)
          cursor(cursorIdx, cursorRow-1);
          oledprint("%c", ABC[menuIdxABC]);
          
          updateOLED_row(0);
          //updateOLED_row(7);
          
          //updateOLED();
       
    		}
      
      
      if ( oneTimeUpdate == 1 ) {
        
          oneTimeUpdate = 0;
          updateOLED();
          
          cursor(0, cursorRowMin-1); // The initial character when opening the menu
          oledprint("%c", 'A');
          
          
        
      }




        // ---
        
        
        
        
        // Redraw based on passwordIdx change
        
        /*if (passwordIdx != passwordIdxLast) {
          
         
         
          
          
        } */         
             
         
        
        
        
        
        // ---
        
                 
        
        
        
        
        // ---
    
    
        if (button(L_NAV_LEFT)) {
           
           // ABC scroll
           
           // scroll the currently-in-view char
           
           --menuIdxABC;
           menuIdxABC = (menuIdxABC < menuIdxABCMin) ? menuIdxMax : menuIdxABC; // Assumes 40 ABC chars
           
           //OLED[cursorRow][cursorIdx] = ABC[menuIdxABC]; 
           
           
           cursor(cursorIdx, cursorRow-1);
           oledprint("%c", ABC[menuIdxABC]);
           
           if (passwordIdx < 0) { passwordIdx = 0; }
                                         
           _apPASS[passwordIdx] = ABC[menuIdxABC];
           
           waitcnt(CNT + CLKFREQ/6); // Important! Set delay appropriate for human preference!
           
           
                              
        }
        
        if (button(L_NAV_RIGHT)) {
           
           // ABC scroll
           
           // scroll the currently-in-view char
           
           ++menuIdxABC;
           menuIdxABC = (menuIdxABC > menuIdxMax) ? menuIdxABCMin : menuIdxABC;
           
           //OLED[cursorRow][cursorIdx] = ABC[menuIdxABC]; 
           
           
           cursor(cursorIdx, cursorRow-1);
           oledprint("%c", ABC[menuIdxABC]);
           
           if (passwordIdx < 0) { passwordIdx = 0; }
           _apPASS[passwordIdx] = ABC[menuIdxABC];
           
           waitcnt(CNT + CLKFREQ/6); // Important! Set delay appropriate for human preference!
           
        }       
        
        // ---
        
      if (button(R_NAV_LEFT)) {
         
             
             if (passwordIdx > 0) {
               
               
               
                  // Remove char at previous position (as backward scroll is delete function)
                  //OLED[cursorRow][cursorIdx] = ' '; 
                  _apPASS[passwordIdx] = '\0';
               
               
                
                
                  // adjust idx
                  --passwordIdx;
             
                  // Remove from display
                  //cursor(cursorIdx-1, cursorRow+1);
                  
                  
                  // Clear old cursor
                  cursor(cursorIdx, cursorRow);
                  oledprint("%c", cursorCharOFF);
                  cursorCharLast = cursorCharOFF;
                  //ggTimeout = CNT -1; // force immediate redraw
                  cogState_GUI_TIMER = COG_ELAPSED;              
                  
                  
                  
                                    
                  
                  cursor(cursorIdx, cursorRow-1);
                  oledprint("%c", ' ');
                  
                  // Update cursor idx
                  //--cursorIdx;
                  //cursorIdx = (cursorIdx < 0) ? 0 : cursorIdx;
                  
                  // If cursor idx now 0, then change Right menu to cancel!
                  /*if ( (cursorIdx == 0) && (OLED[4][0]==' ')) {
                   
                    strcpy(menuRight[0], "CANCEL");
                    oledWriteMenuRight(0);
                    
                  } else {
                    
                    strcpy(menuRight[0], "OK");
                    oledWriteMenuRight(0);
                    
                  }  */                             
              
            
              } else if (passwordIdx == 0) {
                
               
                  // Have pressed left for 2nd time at last char, so delete it! 
                  // No need to move cursor as it stays a position 0

                  _apPASS[passwordIdx] = '\0';
                  
                  --passwordIdx; // passwordIdx will now be -1, which means ZERO password stored
                  
                  // Remove 1st char from screen (use a space to hide it)
                  cursor(0, cursorRow-1);
                  oledprint("%c", ' ');
                
                
              }                 
                         
        }
        
        if (button(R_NAV_RIGHT)) {
           
           
           
              if (passwordIdx < 31) { // 32 is max pwd length (idx 0 - 31)
             
                  
                  // Cursor position
                  
                  // if current view item is not a space, inc passlen
                  
                  /*if (OLED[4][cursorIdx] <> ' ') {
                    
                      ++passlen;
                      passlen = (passlen > 15) ? 15 : passlen;
                      
                  }*/
    
                  
                  //RecalcPassLen(&passlen); 
                  
                  ++passwordIdx;
                  
                  // Add current abc char to next passwordIdx position
                  _apPASS[passwordIdx] = ABC[menuIdxABC];
                  
                  // Remove old cursor
                  cursor(cursorIdx, cursorRow);
                  oledprint("%c", cursorCharOFF);
                  cursorCharLast = cursorCharOFF;
                  //ggTimeout = CNT -1; // force immediate redraw
                  cogState_GUI_TIMER = COG_ELAPSED;
                  
                  
                  // Update idx's
                  cursorIdx = (passwordIdx > 15) ? (passwordIdx - 16) : passwordIdx;
                  cursorRow = (passwordIdx > 15) ? cursorRowMax : cursorRowMin;
        
                  
                  // Add next char to display
                  cursor(cursorIdx, cursorRow - 1);
                  oledprint("%c", ABC[menuIdxABC]);
                  
                  
                  
                  
                  
                  /*               
                  if ( (cursorIdx < (passlen  +1)) && (OLED[4][0] !=' ') ) {
                                    
                      cursor(cursorIdx, cursorRow + 1);
                      oledprint("%c", cursorOFF);
                      cursorCharLast = cursorOFF;
                      cursorggTimeout = CNT -1; // force immediate redraw
                      //oledprint("%s", (cursorIdx < 15) ? " -" : "-");
                    
                      ++cursorIdx;
                      cursorIdx = (cursorIdx > 15) ? 15 : cursorIdx;
                      
                      // Add new char at cursor position, using same as previously selected
                      OLED[cursorRow][cursorIdx] = ABC[menuIdxABC]; 
                      cursor(cursorIdx, cursorRow);
                      oledprint("%c", OLED[cursorRow][cursorIdx]);
               
                   }                                                               
                   */ 
                
              }             
                  
           
        }
        
        if (button(L_NAV_PRESS)) {
          
            // Wait until not pressed
            while(button(L_NAV_PRESS)) {}
            
            menuSelectedLeft = 1;
          
        }
        
        if (button(R_NAV_PRESS)) {
          
            // Wait until not pressed
            while(button(R_NAV_PRESS)) {}
            
            menuSelectedRight = 1;
          
        }       
        
        // --- 

			
    
    
    }  // while(cogState_GUI == COG_RUNNING) 
    
    
    cogState_GUI = COG_STOPPED;
    /*cog_end*/cogstop(cogGUI);


} // void
  
  



// -----------------------------------------------------------------------------------------------------------------------------------------------------------




// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------










