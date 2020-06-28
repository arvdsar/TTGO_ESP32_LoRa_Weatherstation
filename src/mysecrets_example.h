// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
#define MY_APPEUI_KEY 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA

// This should also be in little endian format, see above.
#define MY_DEVEUI_KEY 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA 

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
#define MY_APP_KEY 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA

//Data will be send each time the ESP wakes up.
//time_to_sleep therefor defines how often a message will be send

#define TIME_TO_SLEEP  300      /* Time ESP32 will go to sleep (in seconds) and send a Lora message when it wakes up.*/
