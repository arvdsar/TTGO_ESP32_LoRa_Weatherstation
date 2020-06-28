/*******************************************************************************
 * Simple weather station using TTGO ESP32 LoRa module with BME280 sensor.
 * By: Alexander
 * This code might not be the cleanest but it works. I was in a hurry to 
 * get it running.
 * 
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 * 
 * It might not be the cleanest code, but it works and brings the ESP32
 * to sleep after the period defined in secrets.h
 * Change mysecrets_example.h to secrets.h and add your own parameters in there.
 *
 * This uses OTAA (Over-the-air activation),  where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy. Therefore check in the TTN
 * console what is the airtime of a message and adjust your sleep period accordingly
 * to make sure you do exceed the airtime of 30 seconds / 24hrs

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * In platformio.ini you can set the frequency type of the LoraTransmitter.
 * Use platformio to upload you sketch to your board
 * 
 * By the way, it does not use deepsleep but modemsleep which keeps the memory
 * alive and removes the need to join the LoRa network each time. It doesn't really
 * matter for the runtime on battery, it matters roughly 0.5mA and the TTGO boards are 
 * not really powersavers. Sleep current 9-10 mA so that is why a solar panel with 
 * TP4056 charger is attached ;-).
 *******************************************************************************/

#include <Arduino.h>
#include <mysecrets.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define ADC_PIN 13

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C


//Provide your APP EUI, DEV EUI and Application key in the mysecrets.h file
static const u1_t PROGMEM APPEUI[8]={ MY_APPEUI_KEY };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}


static const u1_t PROGMEM DEVEUI[8]={ MY_DEVEUI_KEY };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}


static const u1_t PROGMEM APPKEY[16] = { MY_APP_KEY };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t payload[10];
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations). Not used in this sketch. 
//const unsigned TX_INTERVAL = 60;

int DoneSending = 0;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 33,32},
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        int16_t payloadTemp = (bme.readTemperature()*100);
        // int -> bytes
        byte tempLow = lowByte(payloadTemp);
        byte tempHigh = highByte(payloadTemp);
        // place the bytes into the payload
        payload[1] = tempLow;
        payload[0] = tempHigh;

        int payloadHumid = (bme.readHumidity()*100);
        byte humidLow = lowByte(payloadHumid);
        byte humidHigh = highByte(payloadHumid);
        payload[3] = humidLow;
        payload[2] = humidHigh;

        float voltage = analogRead(ADC_PIN);
        int payloadVoltage = (voltage * (4.02 / 3871))*100;;
        byte voltageLow = lowByte(payloadVoltage);
        byte voltageHigh = highByte(payloadVoltage);
        payload[5] = voltageLow;
        payload[4] = voltageHigh;
        
        long payloadPressure = bme.readPressure()*100;
        payload[6] = (byte) ((payloadPressure & 0xFF000000) >> 24 );
        payload[7] = (byte) ((payloadPressure & 0x00FF0000) >> 16 );
        payload[8] = (byte) ((payloadPressure & 0x0000FF00) >> 8  );
        payload[9] = (byte) ((payloadPressure & 0X000000FF)       );
    

        LMIC_setTxData2(1, payload, sizeof(payload)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}
void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
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
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
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
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));

            }
            DoneSending = 1;

            Serial.println("go to Sleep");
            esp_light_sleep_start();

            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
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
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}




//float voltage;
void setup() {
    Serial.begin(9600);

 

    bool status;
      // default settings
      // (ou can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }



    Serial.println(F("Starting"));
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
   Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +  " Seconds");

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    LMIC_setAdrMode(0);
    LMIC_setDrTxpow(DR_SF7, 14);

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);


}


void loop() {

        os_runloop_once();
    if (DoneSending == 1){
        DoneSending =0;
        do_send(&sendjob);
    }

}
