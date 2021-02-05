#include <RTCZero.h>
#include <lmic.h>
#include <hal/hal.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Time.h>
#include <Adafruit_SleepyDog.h>

#define WITH_RTC 1              //Use RTC for alarming
//#define WITH_LORA 1             //Use LORA for communication instead of serial connection
//#define WITH_WATCHDOG 1         //Use watchdog to restart the board


const uint8_t ledPin = 13;        //Onboard LED
const uint8_t interruptOut = 10;  //Outer barrier interrupt pin
const uint8_t interruptIn = 11;   //Inner barrier interrupt pin
const uint8_t triggerIn = 12;     //Pin to trigger inner barrier
const uint8_t triggerOut = 5;     //Pin to trigger outer barrier

struct PlannedTask {
  uint32_t TimeStamp;
  uint8_t Command;  
  uint8_t Parameter;
};

int msgPeriod = 20;               //Period between two sends
int showPeriod = 20;             //Period between two showRoomState calls (has to be set to 1200 for assignment)
int defaultPeriod = 20;           //Period between two alarms if no task is available
int resetPeriod = 86400;          //Reset count every 24 hours in seconds
volatile uint8_t count = 0;       //Current number of people
volatile uint8_t prediction = 0;  //Predicted number of people
int timewarpFactor=1;             //Runs time with different speed
bool rtcKickstart = true;        //Starts the rtc/alarm cycle if no tasks were provided
uint32_t initialTime=0;           //Keeps track of the intial time set when the board was booted
uint32_t nextAlarm;               //Timestamp in seconds for the next alarm
const uint32_t capacity=1000;      //Maximum capacity of task list

uint8_t taskCounter = 0;
PlannedTask taskList[capacity];


const long capacityOfClass = 35;  //Capacity of seminar room


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


RTCZero rtc;
//SEMINAR ROOM

static  u1_t nwkSKey[16] = { };
static  u1_t  appSKey[16] = { };
static  u4_t devAddr = 0x01020304; // <-- Change this address for every node!


// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmic/project_config/lmic_project_config.h,
// otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
//static const u1_t PROGMEM APPEUI[8]={ 0xEF, 0xFE, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
//void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
//static const u1_t PROGMEM DEVEUI[8]={ 0x94, 0x63, 0x42, 0x35, 0x29, 0x89, 0x73, 0x56 };
//void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
//static const u1_t PROGMEM APPKEY[16] = { 0x2F, 0x29, 0x46, 0x00, 0x95, 0x5E, 0x79, 0x41, 0x3C, 0x62, 0x3F, 0xA9, 0xF7, 0x52, 0xF7, 0x0F };
//void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

volatile int msgDone = 1;
static osjob_t sendjob;
static osjob_t initjob;


// Pin mapping
// Adapted for Feather M0 per p.10 of [feather]
const lmic_pinmap lmic_pins = {
  .nss = 8,                       // chip select on feather (rf95module) CS
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 4,                       // reset pin
  //    .dio = {6, 5, LMIC_UNUSED_PIN}, // assumes external jumpers [feather_lora_jumper]
  // DIO1 is on JP1-1: is io1 - we connect to GPO6
  // DIO1 is on JP5-3: is D2 - we connect to GPO5
  .dio = {3, 6, LMIC_UNUSED_PIN},
  .rxtx_rx_active = 0,
  .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
  .spi_freq = 8000000,
};

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_RFU1:
      ||     Serial.println(F("EV_RFU1"));
      ||     break;
    */
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.println(F("Received "));
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
        prediction=LMIC.frame[LMIC.dataBeg];
        Serial.print("New prediction: "); Serial.println(prediction);
      }
      // Reset watchdog with every loop to make sure the sketch keeps running.
      // If you comment out this call watch what happens in about 4 iterations!
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_SCAN_FOUND:
      ||    Serial.println(F("EV_SCAN_FOUND"));
      ||    break;
    */
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    case EV_TXCANCELED:
      Serial.println(F("EV_TXCANCELED"));
      break;
    case EV_RXSTART:
      Serial.println(F("EV_RXSTART"));
      break;
    case EV_JOIN_TXCOMPLETE:
      Serial.println(F("EV_JOIN_TXCOMPLETE"));
      break;

    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned) ev);
      break;
  }
}


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  pinMode(interruptOut, INPUT_PULLDOWN);
  pinMode(interruptIn, INPUT_PULLDOWN);

  
  attachInterrupt(digitalPinToInterrupt(interruptOut), out, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptIn), in, RISING);
  NVIC_SetPriority((IRQn_Type) digitalPinToInterrupt(interruptOut), 0);
  NVIC_SetPriority((IRQn_Type) digitalPinToInterrupt(interruptIn), 0);

  pinMode(triggerIn, OUTPUT);
  pinMode(triggerOut, OUTPUT);
  digitalWrite(triggerIn, LOW);   
  digitalWrite(triggerOut, LOW);   
 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

  
  while (!Serial);
  Serial.begin(115200);
  showRoomState();

  rtc.begin(); // initialize RTC
  setTime(getInitialTime());

#ifdef WITH_RTC
  rtc.enableAlarm(rtc.MATCH_YYMMDDHHMMSS);
  rtc.attachInterrupt(alarmMatch);
  NVIC_SetPriority(RTC_IRQn, 3);
#endif
  
#ifdef WITH_LORA
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  LMIC_setSession (0x13, devAddr, nwkSKey, appSKey);

  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.

  // Disable link check validation
  //LMIC_setLinkCheckMode(0);
  
  LMIC_setClockError(MAX_CLOCK_ERROR * 80 / 100);
  
  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)

  //os_setCallback(&initjob, initfunc);
  LMIC_setDrTxpow(DR_SF7, 14); 

// Enable/disable link check validation.
// LMIC sets the ADRACKREQ bit in UP frames if there were no DN frames
// for a while. It expects the network to provide a DN message to prove
// connectivity with a span of UP frames. If this no such prove is coming
// then the datarate is lowered and a LINK_DEAD event is generated.
// This mode can be disabled and no connectivity prove (ADRACKREQ) is requested
// nor is the datarate changed.
// This must be called only if a session is established (e.g. after EV_JOINED)
  LMIC_setLinkCheckMode(0);

  LMIC_setAdrMode (0);
#endif

  // First a normal example of using the watchdog timer.
  // Enable the watchdog by calling Watchdog.enable() as below.
  // This will turn on the watchdog timer with a ~4 second timeout
  // before reseting the Arduino. The estimated actual milliseconds
  // before reset (in milliseconds) is returned.
  // Make sure to reset the watchdog before the countdown expires or
  // the Arduino will reset!

#ifdef WITH_WATCHDOG
  int countdownMS = Watchdog.enable(16000);
  Serial.print("Enabled the watchdog with max countdown of ");
  Serial.print(countdownMS, DEC);
  Serial.println(" milliseconds!");
#endif

  Serial.println("Starting");
  digitalWrite(ledPin, LOW);
}


void alarmMatch() {
  //Serial.println("Hoffentlich");
  runTaskControlled(11,0,true);
  runTaskControlled(12,0,true);
  if (getTimeInSeconds()>nextAlarm){
    runAvailTasks();
  }
  setAlarmFromSeconds(getTimeInSeconds()+showPeriod);
}

void loop() {
  String readString="";

  while (Serial.available()){    
    delay(3);  //delay to allow buffer to fill 
    if (Serial.available() >0) {
      char c = Serial.read();  //gets one byte from serial buffer
      readString += c; //makes the string readString
    } 
  }

  if (readString.length() >0) {
    Serial.print(readString); Serial.print(" Length of string: "); Serial.println(readString.length());
    uint32_t time;
    uint32_t cmd;
    uint32_t param;
    int found=1;

    char *str,*str1;
    str1=(char*) calloc(readString.length()+2,1);
    strcpy(str1,readString.c_str());
    str=str1;
    Serial.print("Length after copying: "); Serial.println(strlen(str));

    
    int countit=0;
    while (strlen(str)>3) {
      found=sscanf(str,"%d:%d:%d\n",&time,&cmd,&param);
      Serial.print("INSERT TS: ");
      Serial.print(time);
      Serial.print(" CMD: ");
      Serial.print(cmd);
      Serial.print(" ARGS: ");
      Serial.println(param);

      if (found!=EOF || found!=3){
        if (time==0 && cmd==0){
          setTime(param);
        } else if (time==0){
          //Serial.print("time==0 "); Serial.print(cmd); Serial.print(" ");Serial.println(param);
          runTaskOnce(cmd,param);
        } else {
          insertTask(time,cmd,param);
        }
      }
      int i=0;
      while (str[0]!= '\n') {
        str++;
      }
      str++;
      //Serial.println(strlen(str));


    }
    free(str1);
    printTasklist();
}

if(rtcKickstart){
  rtcKickstart = false;
  alarmMatch();
}
  
//This runs all the time because nextAlarm is initizalised as 0
#ifndef WITH_RTC
  if (getTimeInSeconds()>nextAlarm){
    runAvailTasks();
  }
  showRoomState();
#endif

  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
  delay(200);
  
#ifdef WITH_WATCHDOG
  Watchdog.reset();
#endif

#ifdef WITH_LORA
  os_runloop_once();
#endif
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
 
int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
