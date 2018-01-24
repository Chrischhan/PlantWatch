#ifndef CONFIG_H
#define CONFIG_H

// Define Software and SPIFFS Versions
#define PLANT_WATCH_VERSION 1
#define PLANT_WATCH_SPIFFS_VERSION 1

// enable and disable features in code
//#define OLED
//#define LEDS
//#define SELFUPDATE
#define BATTERY_MONITOR
//#define USE_DHT22
#define USE_SHT30

// Setup debug printing macros.
// comment to disable Serial Logging
#define PLANT_DEBUG

// comment to send ESP to delay instead of DeepSleep
#define USE_DEEPSLEEP

#ifdef USE_DHT22
// Uncomment to enable printing out nice debug messages. (DHT LIB)
//#define DHT_DEBUG
#define DHTPIN D4 // what pin DHT connected to
#endif

#ifdef USE_SHT30
// Uncomment to enable printing out nice debug messages. (DHT LIB)
//#define DHT_DEBUG
#define SHT_ADDRESS 0x45 // what pin DHT connected to
#endif

#define SCL_PIN D1
#define SDA_PIN D2

#define SLEEPSECONDS 900

#ifdef LEDS
#define BLUE_LED D6
#define GREEN_LED D7
#define RED_LED D8
#endif

// Setup debug printing macros.≈
#ifdef PLANT_DEBUG
#define PLANT_PRINT(...)                                                                           \
    {                                                                                              \
        Serial.print(__VA_ARGS__);                                                                 \
    }
#define PLANT_PRINTLN(...)                                                                         \
    {                                                                                              \
        Serial.println(__VA_ARGS__);                                                               \
    }
#else
#define PLANT_PRINT(...)                                                                           \
    {                                                                                              \
    }
#define PLANT_PRINTLN(...)                                                                         \
    {                                                                                              \
    }
#endif

// Uncomment to enable printing out nice debug messages. (for custom OLED Lib)
#ifdef OLED
#define DEBUG_MICROOLED(...) PLANT_PRINT(__VA_ARGS__)
#endif

#endif // CONFIG_H
