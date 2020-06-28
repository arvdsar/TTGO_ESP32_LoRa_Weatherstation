Simple weather station using TTGO ESP32 LoRa module with BME280 sensor.
This code uses OTAA with TheThingsNetwork.

Rename mysecrets_example.h to secrets.h and add your own parameters in there.

Register you application and device in the TTN Console and enter the appropriate
APP_EUI, DEV_EUI and Application Key into the secrets.h file.
Additionally change the sleeptime in secrets.h to the interval you want to use
to send data. Keep in mind the fairuse policy of TTN (30 seconds per 24 hrs)

Use Platformio to compile and upload the sketch. 

This sketch intentionally uses esp_light_sleep_start and not deep_sleep.
Using light_sleep the memory remains active and therefore the LoRa parameters
are kept. If you would use deep_Sleep, the device has to join the TTN Network 
each time it is waking up. This is a waste of airtime.

 It doesn't really matter for the runtime on battery, it matters roughly 0.5mA 
 and the TTGO boards are not really powersavers. 
 Sleep current 9-10 mA so that is why a solar panel with TP4056 charger is 
 attached in my setup.

