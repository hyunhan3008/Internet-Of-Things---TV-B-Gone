// LoRaWAN template sketch
bool DBG = true; // debug switch

// macros for debug calls to Serial.printf, with (D) and without (DD)
// new line, and to Serial.println (DDD)
#define D(args...)   { if(DBG) { Serial.printf(args); Serial.println(); } }
#define DD(args...)  { if(DBG) Serial.printf(args); }
#define DDD(s)       { if(DBG) Serial.println(s); }

#include <lmic.h>               // IBM LMIC (LoraMAC-in-C) library
#include <hal/hal.h>            // hardware abstraction for LMIC on Arduino
#include <SPI.h>                // the SPI bus
#include <Adafruit_GFX.h>       // core graphics library
#include "Adafruit_HX8357.h"    // tft display local hacked version
#include "Adafruit_STMPE610.h"  // touch screen local hacked version
#include <Wire.h>               // I²C comms on the Arduino
#include "IOExpander.h"         // unPhone's IOExpander (controlled via I²C)

// the LCD and touch screen
#define TFT_DC   33
Adafruit_HX8357 tft =
  Adafruit_HX8357(IOExpander::LCD_CS, TFT_DC, IOExpander::LCD_RESET);
Adafruit_STMPE610 ts = Adafruit_STMPE610(IOExpander::TOUCH_CS);
// calibration data for converting raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

// battery management
byte BM_I2Cadd = 0x6b;
byte BM_Watchdog = 0x05; // Charge Termination/Timer Control Register
byte BM_OpCon    = 0x07; // Misc Operation Control Register
byte BM_Status   = 0x08; // System Status Register 
byte BM_Version  = 0x0a; // Vender / Part / Revision Status Register 

//LoRaWAN configuration:
// LoRaWAN NwkSKey, network session key
// This is a dummy key, which should be replaced by your individual key
// Copy from ttn device overview webpage - Network Session Key - msb format
static const PROGMEM u1_t NWKSKEY[16] = { 0x44, 0x5F, 0xC3, 0xF3, 0x2E, 0xCB, 0x96, 0x2B, 0x6D, 0x29, 0x27, 0x8D, 0x8A, 0x25, 0xDE, 0xB6 };

// LoRaWAN AppSKey, application session key
// This is a dummy key, which should be replaced by your individual key
// Copy from ttn device overview webpage - App Session Key - msb format
static const u1_t PROGMEM APPSKEY[16] = { 0xAB, 0x64, 0x6D, 0xFE, 0xEF, 0xFF, 0x56, 0xEF, 0xB7, 0x59, 0x73, 0x6E, 0x9B, 0x69, 0xDF, 0x64 };

// LoRaWAN end-device address (DevAddr)
// This is a dummy key, which should be replaced by your individual key
// Copy from ttn device overview webpage - Device Address - msb format
static const u4_t DEVADDR = 0x2601150B ;

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

uint8_t mydata[] = "Hello, World!";
uint8_t loopcount[] = " Loops: "; 
static osjob_t sendjob;

// schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// pin mapping
const lmic_pinmap lmic_pins = {  // modified to suit connection on unphone
  .nss = IOExpander::LORA_CS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = IOExpander::LORA_RESET,
  .dio = {39, 34, LMIC_UNUSED_PIN}  
};

void onEvent (ev_t ev) {
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            DDD(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            DDD(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            DDD(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            DDD(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            DDD(F("EV_JOINING"));
            break;
        case EV_JOINED:
            DDD(F("EV_JOINED"));
            break;
        case EV_RFU1:
            DDD(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            DDD(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            DDD(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            DDD(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              DDD(F("Received ack"));
            if (LMIC.dataLen) {
              DDD(F("Received "));
              DDD(LMIC.dataLen);
              DDD(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            DDD(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            DDD(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            DDD(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            DDD(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            DDD(F("EV_LINK_ALIVE"));
            break;
         default:
            DDD(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j) {
    // check if there is not a current TX/RX job running
    if(LMIC.opmode & OP_TXRXPEND) {
        D("OP_TXRXPEND, not sending");
    } else {
        // prepare upstream data transmission at the next possible time
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        loopcount[0]=+1;
        LMIC_setTxData2(1, loopcount, sizeof(loopcount)-1, 0);        
        D("Packet queued");
    }
    // next TX is scheduled after TX_COMPLETE event
}

void setup() {
  if (DBG==true) Serial.begin(115200);
  Wire.setClock(100000); // TODO higher rates trigger an IOExpander bug
  Wire.begin();
  IOExpander::begin();
  uint8_t inputPwrSw = IOExpander::digitalRead(IOExpander::POWER_SWITCH);

  bool powerGood = // bit 2 of status register indicates if USB connected
    bitRead(getRegister(BM_I2Cadd, BM_Status), 2);

  if(!inputPwrSw) {  // when power switch off
    if(!powerGood) { // and usb unplugged we go into shipping mode
      setShipping(true);
    } else { // power switch off and usb plugged in we sleep
      esp_sleep_enable_timer_wakeup(1000000); // sleep time is in uSec
      esp_deep_sleep_start();
    }
  }
 
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
  ts.begin();
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_WHITE);
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, HIGH);

  // LMIC init
  DDD("doing LMIC os_init()...");
  os_init(); // 
  
  // reset the MAC state; session and pending data transfers will be discarded
  DDD("doing LMIC_reset()...");
  LMIC_reset();

  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
  #ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
  #else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
  #endif

  #if defined(CFG_eu868)
  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  // NA-US channels 0-71 are configured automatically
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
  #elif defined(CFG_us915)
  // NA-US channels 0-71 are configured automatically
  // but only one group of 8 should (a subband) should be active
  // TTN recommends the second sub band, 1 in a zero based count.
  // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
  LMIC_selectSubBand(1);
  #endif

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,14);


  // Start job
  do_send(&sendjob);
}

void loop(void) {
  uint8_t inputPwrSw = IOExpander::digitalRead(IOExpander::POWER_SWITCH);

  bool powerGood = // bit 2 of status register indicates if USB connected
    bitRead(getRegister(BM_I2Cadd, BM_Status), 2);

  if(!inputPwrSw) {  // when power switch off...
    if(!powerGood) { // and usb unplugged we go into shipping mode
      setShipping(true);
    } else { // power switch is off and usb plugged in: we sleep
      esp_sleep_enable_timer_wakeup(1000000); // sleep time is in uSec
      esp_deep_sleep_start();
    }
  }
  os_runloop_once(); 
  delay(50);
}

void setShipping(bool value) {
  byte result;
  if(value) {
    result=getRegister(BM_I2Cadd, BM_Watchdog);  // state of timing register
    bitClear(result, 5);                         // clear bit 5...
    bitClear(result, 4);                         // and bit 4 to disable...
    setRegister(BM_I2Cadd, BM_Watchdog, result); // WDT (REG05[5:4] = 00)

    result=getRegister(BM_I2Cadd, BM_OpCon);     // operational register
    bitSet(result, 5);                           // set bit 5 to disable...
    setRegister(BM_I2Cadd, BM_OpCon, result);    // BATFET (REG07[5] = 1)
  } else {
    result=getRegister(BM_I2Cadd, BM_Watchdog);  // state of timing register
    bitClear(result, 5);                         // clear bit 5...
    bitSet(result, 4);                           // and set bit 4 to enable...
    setRegister(BM_I2Cadd, BM_Watchdog, result); // WDT (REG05[5:4] = 01)

    result=getRegister(BM_I2Cadd, BM_OpCon);     // operational register
    bitClear(result, 5);                         // clear bit 5 to enable...
    setRegister(BM_I2Cadd, BM_OpCon, result);    // BATFET (REG07[5] = 0)
  }
}

void setRegister(byte address, byte reg, byte value) {
  write8(address, reg, value);
}

byte getRegister(byte address, byte reg) {
  byte result;
  result=read8(address, reg);
  return result;
}

void write8(byte address, byte reg, byte value) {
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

byte read8(byte address, byte reg) {
  byte value;
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();
  return value;
}
