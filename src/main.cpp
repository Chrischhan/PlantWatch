#include <Arduino.h>
#include <Wire.h>

#include "config.h"

#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>
#include <I2CSoilMoistureSensor.h>

#include <PubSubClient.h>

#ifdef SELFUPDATE
#include <ESP8266httpUpdate.h>
#endif

#ifdef USE_DHT22
#include <DHT.h>
DHT dht(DHTPIN, DHT22); // DHT 22  (AM2302)
#endif

#ifdef USE_SHT30
#include <WEMOS_SHT3X.h>
SHT3X sht30(SHT_ADDRESS);
#endif

I2CSoilMoistureSensor sensor;

#ifdef OLED
#include "Images.h"
#include "My_MicroOLED.h"

#define OLED_RESET 0 // GPIO0

My_MicroOLED display(OLED_RESET);
#endif

ESP8266WiFiMulti wifiMulti;

WiFiClient espClient;
PubSubClient mqtt(espClient);

String mqtt_server;
int mqtt_port;

String updateServer = "";

void callback(char* topic, byte* payload, unsigned int length)
{
    PLANT_PRINT("Message arrived [");
    PLANT_PRINT(topic);
    PLANT_PRINT("] (length: ");
    PLANT_PRINT(length)
    PLANT_PRINT(") ")
    String readyPayload("");
    for (int i = 0; i < length; i++)
    {
        PLANT_PRINT((char) payload[i]);
        readyPayload += (char) payload[i];
    }
    PLANT_PRINTLN();

#ifdef SELFUPDATE
    if (0 == strcmp(topic, "SYSTEM/PlantWatch/Version"))
    {
        if (readyPayload != String(PLANT_WATCH_VERSION))
        {
            PLANT_PRINTLN("try to update Version")
            t_httpUpdate_return ret =
              ESPhttpUpdate.update(updateServer, 80, "/espupdate.php", String(PLANT_WATCH_VERSION));
            switch (ret)
            {
            case HTTP_UPDATE_FAILED:
            {
                PLANT_PRINTLN("[update] Update failed.");
                PLANT_PRINTLN(ESPhttpUpdate.getLastErrorString())
                break;
            }
            case HTTP_UPDATE_NO_UPDATES:
                PLANT_PRINTLN("[update] Update no Update.");
                PLANT_PRINTLN(ESPhttpUpdate.getLastErrorString())
                break;
            case HTTP_UPDATE_OK:
                PLANT_PRINTLN("[update] Update ok."); // may not called we reboot the ESP
                break;
            }
        }
    }
#endif
}

bool initializeWifiNetwork()
{
    if (!SPIFFS.begin())
    {
        PLANT_PRINTLN("Failed to mount file system");
        return false;
    }

    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile)
    {
        PLANT_PRINTLN("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 1024)
    {
        PLANT_PRINTLN("Config file size is too large");
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root         = jsonBuffer.parseObject(buf.get());
    JsonArray& wifi          = root["wifi"];
    const char* updateServer = root["updateServer"];

    for (int count = 0; count < wifi.size(); ++count)
    {
        JsonObject& wifiObject = wifi[count];
        const char* ssid       = wifiObject["ssid"];
        const char* key        = wifiObject["key"];

        PLANT_PRINT("Add Wifinetwork to list. SSID: ");
        PLANT_PRINTLN(ssid);
        // uncomment only for real debug -> security ;-)
        // PLANT_PRINT(", Key: ");
        // PLANT_PRINTLN(key);

        wifiMulti.addAP(ssid, key);
    }
    const char* server = root["mqtt"][0];
    mqtt_server        = String(server);
    mqtt_port          = root["mqtt"][1];

    configFile.close();
    SPIFFS.end();

    PLANT_PRINTLN("PreConnecting Wifi...");
    wifiMulti.run();

    return true;
}

bool initializeHardware()
{
    pinMode(BUILTIN_LED, OUTPUT); // initialize onboard LED as output
#ifdef LEDS
    pinMode(BLUE_LED, OUTPUT); // set onboard LED as output
    pinMode(GREEN_LED, OUTPUT); // set onboard LED as output
    pinMode(RED_LED, OUTPUT); // set onboard LED as output
#endif
    // Connect D0 to RST to wake up
    pinMode(D0, WAKEUP_PULLUP);

    digitalWrite(BUILTIN_LED, HIGH); // turn on LED with voltage LOW
#ifdef LEDS
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
#endif

    Wire.begin(SDA_PIN, SCL_PIN);

#ifdef OLED
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 64x48)
#endif

    sensor.begin(); // reset sensor
#ifdef USE_DHT22
    dht.begin();
#endif
    return true;
}

void reconnect()
{
    int timeout_counter = 10;
    // Loop until we're reconnected
    while (!mqtt.connected())
    {
        PLANT_PRINTLN("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(ESP.getChipId(), HEX);

        // initialize MQTT
        PLANT_PRINT("MQTT Server: ");
        PLANT_PRINT(mqtt_server);
        PLANT_PRINT(" MQTT Port: ");
        PLANT_PRINTLN(mqtt_port);
        mqtt.setServer(mqtt_server.c_str(), mqtt_port);
        mqtt.setCallback(callback);

        // Attempt to connect
        if (mqtt.connect(clientId.c_str()))
        {
            PLANT_PRINTLN("connected");
            // Once connected, publish an announcement...
            mqtt.publish("SYSTEM/PlantWatch/Devices", clientId.c_str());
            // ... and resubscribe
            mqtt.subscribe("SYSTEM/PlantWatch/Version");
            mqtt.subscribe("SYSTEM/PlantWatch/FSVersion");
        }
        else
        {
            PLANT_PRINT("failed, rc=");
            PLANT_PRINT(mqtt.state());
            PLANT_PRINTLN(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);

            if (timeout_counter == 0)
            {
#ifdef USE_DEEPSLEEP
                // convert to microseconds
                ESP.deepSleep(SLEEPSECONDS * 1000000);
#else
                delay(SLEEPSECONDS * 1000);
#endif
            }
            timeout_counter--;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    PLANT_PRINTLN();

    WiFi.mode(WIFI_STA);

    if (not initializeHardware())
    {
        PLANT_PRINTLN("Failed to initialize Hardware");
    }
    if (not initializeWifiNetwork())
    {
        PLANT_PRINTLN("Failed to initialize Wifi");
    }

    // init done
    PLANT_PRINTLN();
    PLANT_PRINTLN("PlantSensor V3.0 Beta");
    PLANT_PRINT("I2C Soil Moisture Sensor Address: ");
    PLANT_PRINTLN(sensor.getAddress(), HEX);
    PLANT_PRINT("Sensor Firmware version: ");
    PLANT_PRINTLN(sensor.getVersion(), HEX);
    PLANT_PRINT("ESP ChipID: ");
    PLANT_PRINTLN(ESP.getChipId(), HEX);
    PLANT_PRINTLN("Check ID in: https://www.wemos.cc/verify_products");
    PLANT_PRINT("Sketch MD5: ");
    PLANT_PRINTLN(String(ESP.getSketchMD5()));
#ifdef OLED
    display.version();
#endif
    PLANT_PRINTLN();

#ifdef LEDS
    // give some time to boot all up
    digitalWrite(RED_LED, HIGH);
#endif
    for (int i = 0; i <= 100; i++)
    {
#ifdef OLED
        display.drawProgressBar(0, 35, 62, 10, i);
        display.display();
#endif
        digitalWrite(BUILTIN_LED, i % 2 == 1 ? HIGH : LOW);
#ifdef LEDS
        if (i == 33)
        {
            digitalWrite(GREEN_LED, HIGH);
        }
        if (i == 66)
        {
            digitalWrite(BLUE_LED, HIGH);
        }
#endif
        delay(20);
    }

#ifdef LEDS
    for (int i = 0; i <= 6; i++)
    {
        digitalWrite(BLUE_LED, i % 2 == 1 ? HIGH : LOW);
        digitalWrite(GREEN_LED, i % 2 == 1 ? HIGH : LOW);
        digitalWrite(RED_LED, i % 2 == 1 ? HIGH : LOW);
        delay(100);
    }

    digitalWrite(BUILTIN_LED, HIGH); // turn on LED with voltage LOW
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
#endif

#ifdef OLED
    // Clear the buffer.
    display.clearDisplay();
    display.display();

/*for (int i = 0; i < 256; i++)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(i);
      display.drawChar(0, 10, (unsigned char) i, WHITE, BLACK, 1);
      display.display();
      delay(500);
    }*/
#endif
}

float convertCtoF(float c)
{
    return c * 1.8 + 32;
}

float convertFtoC(float f)
{
    return (f - 32) * 0.55555;
}

// boolean isFahrenheit: True == Fahrenheit; False == Celcius
float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit)
{
    // Using both Rothfusz and Steadman's equations
    // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
    float hi;

    if (!isFahrenheit)
        temperature = convertCtoF(temperature);

    hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

    if (hi > 79)
    {
        hi = -42.379 + 2.04901523 * temperature + 10.14333127 * percentHumidity +
             -0.22475541 * temperature * percentHumidity + -0.00683783 * pow(temperature, 2) +
             -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature * pow(percentHumidity, 2) +
             -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

        if ((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
            hi -=
              ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

        else if ((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
            hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
    }

    return isFahrenheit ? hi : convertFtoC(hi);
}

void loop()
{
    // get all possible Values

    while (sensor.isBusy())
    {
        delay(50); // available since FW 2.3
    }
    float soilMoisture = sensor.getCapacitance();
    float tempChirp    = sensor.getTemperature() / (float) 10;
    float light        = sensor.getLight(true);
    sensor.sleep(); // available since FW 2.3

    float temp;
    float humidity;

#ifdef USE_DHT22
    temp     = dht.readTemperature();
    humidity = dht.readHumidity();
#endif
#ifdef USE_SHT30
    sht30.get();
    temp     = sht30.cTemp;
    humidity = sht30.humidity;
#endif

#ifdef BATTERY_MONITOR
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.2V):
    float voltage = sensorValue * (4.2 / 1023.0);
#endif

    PLANT_PRINT("Soil Moisture Capacitance: ");
    PLANT_PRINT(soilMoisture); // read capacitance register
    PLANT_PRINT(", Soil-Temperature: ");
    PLANT_PRINT(tempChirp); // temperature register
    PLANT_PRINT(", Light: ");
    PLANT_PRINT(light); // request light measurement, wait and read light register*/
    PLANT_PRINT(", Temperature: ");
    PLANT_PRINT(temp);
    PLANT_PRINT(", Humidity: ");
    PLANT_PRINT(humidity);
    PLANT_PRINT(", HeatIndex: ");
    PLANT_PRINT(computeHeatIndex(temp, humidity, false));
#ifdef BATTERY_MONITOR
    PLANT_PRINT(", Voltage: ");
    PLANT_PRINT(voltage);
    PLANT_PRINT(", SensorValue: ");
    PLANT_PRINT(sensorValue);
#endif
    PLANT_PRINTLN();

    if (wifiMulti.run() == WL_CONNECTED)
    {
        PLANT_PRINT("WiFi connected, IP address: ");
        PLANT_PRINTLN(WiFi.localIP());
    }
    else
    {
        PLANT_PRINTLN("WiFi NOT connected going to DeepSleep");
#ifdef USE_DEEPSLEEP
        // convert to microseconds
        ESP.deepSleep(SLEEPSECONDS * 1000000);
#else
        delay(SLEEPSECONDS * 1000);
#endif
        return;
    }

    if (!mqtt.connected())
    {
        reconnect();
    }

    // publish Values to defined Topics
    String topic = "PlantWatch/";
    topic += String(ESP.getChipId(), HEX);
    topic += "/";
    PLANT_PRINTLN("publish messages to " + topic);
    mqtt.publish(String(topic + "SoilMoisture").c_str(), String(soilMoisture, 2).c_str());
    mqtt.publish(String(topic + "Temperature_Soil").c_str(), String(tempChirp, 2).c_str());
    mqtt.publish(String(topic + "Temperature_DHT").c_str(), String(temp, 2).c_str());
    mqtt.publish(String(topic + "Light").c_str(), String(light, 2).c_str());
    mqtt.publish(String(topic + "Humidity").c_str(), String(humidity, 2).c_str());
    mqtt.publish(String(topic + "HeatIndex").c_str(),
                 String(computeHeatIndex(temp, humidity, false), 2).c_str());
#ifdef BATTERY_MONITOR
    mqtt.publish(String(topic + "Voltage").c_str(), String(voltage, 2).c_str());
    mqtt.publish(String(topic + "VoltageAnalogValue").c_str(), String(sensorValue).c_str());
#endif
    PLANT_PRINTLN("messages published");

#ifdef OLED
    display.updateDisplay(voltage, temp, humidity);
#endif

    mqtt.loop();
    mqtt.disconnect();
    delay(2000);
#ifdef USE_DEEPSLEEP
    // convert to microseconds
    ESP.deepSleep(SLEEPSECONDS * 1000000);
#else
    delay(SLEEPSECONDS * 1000);
#endif
}
