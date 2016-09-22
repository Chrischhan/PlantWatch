#include <Arduino.h>
#include <Wire.h>
#include <I2CSoilMoistureSensor.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>

#include <ArduinoJson.h>
// Uncomment to enable printing out nice debug messages.
//#define DHT_DEBUG
#include <DHT.h>

// Setup debug printing macros.
#define PLANT_DEBUG
#ifdef PLANT_DEBUG
  #define PLANT_PRINT(...) { Serial.print(__VA_ARGS__); }
  #define PLANT_PRINTLN(...) { Serial.println(__VA_ARGS__); }
#else
  #define PLANT_PRINT(...) {}
  #define PLANT_PRINTLN(...) {}
#endif

// Uncomment to enable printing out nice debug messages.
#define DEBUG_MICROOLED(...) PLANT_PRINT( __VA_ARGS__ )
#include "My_MicroOLED.h"

#define OLED_RESET 0  // GPIO0

#define DHTPIN D4     // what pin DHT connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define BLUE_LED D6
#define GREEN_LED D7
#define RED_LED D8

#define SCL_PIN D1
#define SDA_PIN D2

#define SLEEPSECONDS 10

I2CSoilMoistureSensor sensor;
My_MicroOLED display(OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);
ESP8266WiFiMulti wifiMulti;

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
    // TODO: Enter DeepSleep and wait for correct connection
  }

  return true;
}

bool initializeHardware()
{
  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(BLUE_LED, OUTPUT);  // set onboard LED as output
  pinMode(GREEN_LED, OUTPUT);  // set onboard LED as output
  pinMode(RED_LED, OUTPUT);  // set onboard LED as output

  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage LOW
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  sensor.begin(); // reset sensor
  dht.begin();
  delay(1000);// give some time to boot all up
  return true;
}

void setup() {

  Serial.begin(115200);
  PLANT_PRINTLN();
  Serial.setDebugOutput(true);
  PLANT_PRINTLN();

  if (not initializeHardware()) {
    PLANT_PRINTLN("Failed to initialize Hardware");
  }
  if (not initializeWifiNetwork()) {
    PLANT_PRINTLN("Failed to initialize Wifi");
  }

  // init done
  PLANT_PRINTLN();
  PLANT_PRINTLN("PlantSensor V3.0 Beta");
  PLANT_PRINT("I2C Soil Moisture Sensor Address: ");
  PLANT_PRINTLN(sensor.getAddress(),HEX);
  PLANT_PRINT("Sensor Firmware version: ");
  PLANT_PRINTLN(sensor.getVersion(),HEX);
  PLANT_PRINT("ESP ChipID: ");
  PLANT_PRINTLN(ESP.getChipId());
  display.version();
  PLANT_PRINTLN();

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);// give some time to boot all up

  // Clear the buffer.
  display.clearDisplay();

  // text display tests
  /*display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Hello, wor");
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.print(3.141592);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("0x");
  display.println(0xDEADBEEF, HEX);*/

  /*for (int i = 0; i <= 100; i++)
  {
    display.drawProgressBar(0, 19, 63, 10, i);
    display.display();
    delay(20);
  }*/

}

void loop() {
  /*digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage HIGH
  //delay(500);                      // wait one second
  //digitalWrite(BUILTIN_LED, LOW);   // turn off LED with voltage LOW
  //delay(500);                      // wait one second
  digitalWrite(BLUE_LED, HIGH);           // turn on LED with voltage HIGH
  delay(500);
  digitalWrite(BLUE_LED, LOW);            // turn off LED with voltage LOW
  digitalWrite(GREEN_LED, HIGH);           // turn on LED with voltage HIGH
  delay(500);
  digitalWrite(GREEN_LED, LOW);            // turn off LED with voltage LOW
  digitalWrite(RED_LED, HIGH);           // turn on LED with voltage HIGH
  delay(500);                      // wait one second
  digitalWrite(RED_LED, LOW);            // turn off LED with voltage LOW

*/

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


  //PLANT_PRINT("Sleep for ");
  //PLANT_PRINT(SLEEPSECONDS);
  //PLANT_PRINTLN(" seconds");

  // convert to microseconds
  ESP.deepSleep(SLEEPSECONDS * 1000000);
  //delay(10000);
}
