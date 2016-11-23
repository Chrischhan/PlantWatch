#include <Arduino.h>
#include <Wire.h>

#include "config.h"

#include <I2CSoilMoistureSensor.h>

#include <PubSubClient.h>

#include <ESP8266WiFiMulti.h>
#ifdef SELFUPDATE
  #include <ESP8266httpUpdate.h>
#endif
#include <FS.h>

#include <ArduinoJson.h>

#include <DHT.h>

I2CSoilMoistureSensor sensor;

#ifdef OLED
  #include "My_MicroOLED.h"
  #include "Images.h"

  #define OLED_RESET 0  // GPIO0

  My_MicroOLED display(OLED_RESET);
#endif

DHT dht(DHTPIN, DHT22); // DHT 22  (AM2302)
ESP8266WiFiMulti wifiMulti;

WiFiClient espClient;
PubSubClient mqtt(espClient);

bool initializeWifiNetwork()
{
  if (!SPIFFS.begin()) {
    PLANT_PRINTLN("Failed to mount file system");
    return false;
  }

  File configFile = SPIFFS.open("/wificonfig.json", "r");
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
  JsonArray& json = jsonBuffer.parseArray(buf.get());

  if (!json.success()) {
    PLANT_PRINTLN("Failed to parse config file");
    return false;
  }

  for (int count = 0; count < json.size() ; ++count)
  {
    JsonObject& wifi = json[count]["wifi"];
    const char* ssid = wifi["ssid"];
    const char* key = wifi["key"];

    PLANT_PRINT("Add Wifinetwork to list. SSID: ");
    PLANT_PRINTLN(ssid);
    // uncomment only for real debug -> security ;-)
    // PLANT_PRINT(", Key: ");
    // PLANT_PRINTLN(key);

    wifiMulti.addAP(ssid, key);
  }

  configFile.close();
  SPIFFS.end();

  PLANT_PRINTLN("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED)
  {
    PLANT_PRINT("WiFi connected, IP address: ");
    PLANT_PRINTLN(WiFi.localIP());
  }
  else
  {
    PLANT_PRINTLN("WiFi not connected");
    #ifdef USE_DEEPSLEEP
      // convert to microseconds
      ESP.deepSleep(SLEEPSECONDS * 1000000);
    #else
      delay(SLEEPSECONDS * 1000);
    #endif
  }

  return true;
}

bool initializeHardware()
{
  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  #ifdef LEDS
    pinMode(BLUE_LED, OUTPUT);  // set onboard LED as output
    pinMode(GREEN_LED, OUTPUT);  // set onboard LED as output
    pinMode(RED_LED, OUTPUT);  // set onboard LED as output
  #endif
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage LOW
  #ifdef LEDS
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
  #endif

  Wire.begin(SDA_PIN, SCL_PIN);

  #ifdef OLED
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  #endif

  sensor.begin(); // reset sensor
  dht.begin();
  return true;
}

void callback(char* topic, byte* payload, unsigned int length)
{
  PLANT_PRINT("Message arrived [");
  PLANT_PRINT(topic);
  PLANT_PRINT("] (length: ");
  PLANT_PRINT(length)
  PLANT_PRINT(") ")
  String readyPayload("");
  for (int i = 0; i < length; i++) {
    PLANT_PRINT((char)payload[i]);
    readyPayload += (char)payload[i];
  }
  PLANT_PRINTLN();

#ifdef SELFUPDATE
  if (0 == strcmp(topic, "SYSTEM/PlantWatch/Version"))
  {
    if (readyPayload != String(PLANT_WATCH_VERSION))
    {
      PLANT_PRINTLN("try to update Version")
      t_httpUpdate_return ret = ESPhttpUpdate.update(
        "192.168.42.108", 80, "/espupdate.php", String(PLANT_WATCH_VERSION));
      switch(ret)
      {
          case HTTP_UPDATE_FAILED:
              PLANT_PRINTLN("[update] Update failed.");
              PLANT_PRINTLN(ESPhttpUpdate.getLastErrorString())
              break;
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

void reconnect()
{
  // Loop until we're reconnected
  while (!mqtt.connected())
  {
    PLANT_PRINT("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(ESP.getChipId(), HEX);;
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
    }
  }
}

void setup()
{

  Serial.begin(115200);
  PLANT_PRINTLN();

  if (not initializeHardware()) {
    PLANT_PRINTLN("Failed to initialize Hardware");
  }
  if (not initializeWifiNetwork()) {
    PLANT_PRINTLN("Failed to initialize Wifi");
  }

  // initialize MQTT
  mqtt.setServer("192.168.42.108", 1883);
  mqtt.setCallback(callback);

  // init done
  PLANT_PRINTLN();
  PLANT_PRINTLN("PlantSensor V3.0 Beta");
  PLANT_PRINT("I2C Soil Moisture Sensor Address: ");
  PLANT_PRINTLN(sensor.getAddress(), HEX);
  PLANT_PRINT("Sensor Firmware version: ");
  PLANT_PRINTLN(sensor.getVersion(), HEX);
  PLANT_PRINT("ESP ChipID: ");
  PLANT_PRINTLN(ESP.getChipId(), HEX);
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

    digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage LOW
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

#ifdef OLED
void drawTemperature(const float& temperature)
{
  display.drawXBitmap(0, 0, ico_temp, ico_width, ico_height, WHITE);
  String tempText = String(temperature, 1);
  String text = tempText + " C";

  char charBuf[10];
  tempText.toCharArray(charBuf, 10);

  int16_t newx, newy;
  uint16_t wid, hig;

  display.setCursor(6,0);
  display.getTextBounds(charBuf, display.getCursorX(), display.getCursorY(), &newx, &newy, &wid, &hig);
  display.drawChar(newx + wid + 1, newy, (unsigned char)247, WHITE, BLACK, 1);
  display.print(text);
}

void drawAirHumidity(const float& airHumidity)
{
  display.drawXBitmap(0, 10, ico2_cloud, ico2_width, ico2_height, WHITE);
  String tempText = String(airHumidity, 1);
  String text = tempText + "%";
  display.setCursor(ico2_width + 1, 10);
  display.print(text);
}

void updateDisplay(const float& voltage, const float& temperature, const float& airHumidity)
{
  display.clearDisplay();
  if (voltage >= 3.9)
  {
    display.drawXBitmap(52, 0, logo_bat_3_3, logo_bat_width, logo_bat_height, WHITE);
  }
  else if (voltage >= 3.6)
  {
    display.drawXBitmap(52, 0, logo_bat_2_3, logo_bat_width, logo_bat_height, WHITE);
  }
  else if (voltage >= 3.3)
  {
    display.drawXBitmap(52, 0, logo_bat_1_3, logo_bat_width, logo_bat_height, WHITE);
  }
  else
  {
    display.drawXBitmap(52, 0, logo_bat_0_3, logo_bat_width, logo_bat_height, WHITE);
  }

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  drawTemperature(temperature);
  drawAirHumidity(airHumidity);

  display.drawXBitmap(0, 20, ico_drop, ico_width, ico_height, WHITE);


  display.display();
}
#endif

void loop() {

  // get all possible Values

  while (sensor.isBusy())
  {
    delay(50); // available since FW 2.3
  }
  float soilMoisture = sensor.getCapacitance();
  float tempChirp = sensor.getTemperature()/(float)10;
  float light = sensor.getLight(true);
  sensor.sleep(); // available since FW 2.3

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.2V):
  float voltage = sensorValue * (4.2 / 1023.0);

  PLANT_PRINT("Soil Moisture Capacitance: ");
  PLANT_PRINT(soilMoisture); //read capacitance register
  PLANT_PRINT(", Temperature: ");
  PLANT_PRINT(tempChirp); //temperature register
  PLANT_PRINT(", Light: ");
  PLANT_PRINT(light); //request light measurement, wait and read light register*/
  PLANT_PRINT(", DHT Temperature: ");
  PLANT_PRINT(temp);
  PLANT_PRINT(", DHT Humidity: ");
  PLANT_PRINT(humidity);
  PLANT_PRINT(", DHT HeatIndex: ");
  PLANT_PRINT(dht.computeHeatIndex(temp, humidity, false));
  PLANT_PRINT(", Voltage: ");
  PLANT_PRINT(voltage);
  PLANT_PRINT(", SensorValue: ");
  PLANT_PRINTLN(sensorValue);

  if(wifiMulti.run() == WL_CONNECTED)
  {
    PLANT_PRINT("WiFi connected, IP address: ");
    PLANT_PRINTLN(WiFi.localIP());
  }
  else
  {
    PLANT_PRINTLN("WiFi NOT connected");
  }

  if (!mqtt.connected())
  {
    reconnect();
  }
  mqtt.loop();

  // publish Values to defined Topics
  String topic = "PlantWatch/";
  topic += String(ESP.getChipId(), HEX);
  topic += "/";
  PLANT_PRINTLN("publish messages");

  mqtt.publish(String(topic + "SoilMoisture").c_str(), String(soilMoisture, 2).c_str());
  mqtt.publish(String(topic + "Temperature1").c_str(), String(tempChirp, 2).c_str());
  mqtt.publish(String(topic + "Temperature2").c_str(), String(temp, 2).c_str());
  mqtt.publish(String(topic + "Light").c_str(), String(light, 2).c_str());
  mqtt.publish(String(topic + "Humidity").c_str(), String(humidity, 2).c_str());
  mqtt.publish(String(topic + "HeatIndex").c_str(), String(dht.computeHeatIndex(temp, humidity, false), 2).c_str());
  mqtt.publish(String(topic + "Voltage").c_str(), String(voltage, 2).c_str());
  mqtt.publish(String(topic + "VoltageAnalogValue").c_str(), String(sensorValue).c_str());

  PLANT_PRINTLN("messages published");

  #ifdef OLED
    updateDisplay(voltage, temp, humidity);
  #endif

  #ifdef USE_DEEPSLEEP
    // convert to microseconds
    ESP.deepSleep(SLEEPSECONDS * 1000000);
  #else
    delay(SLEEPSECONDS * 1000);
  #endif

}
