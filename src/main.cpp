#include <Arduino.h>
#include <Wire.h>
#include <I2CSoilMoistureSensor.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

// Uncomment to enable printing out nice debug messages.
//#define DHT_DEBUG
#include <DHT.h>

// Uncomment to enable printing out nice debug messages.
#define DEBUG_MICROOLED(...) Serial.printf( __VA_ARGS__ )
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

void setup() {
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

  Serial.begin(9600);
  Serial.println("PlantSensor V3.0 Beta");

  Wire.begin(SDA_PIN, SCL_PIN);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  sensor.begin(); // reset sensor
  dht.begin();

  wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
  //wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  // init done

  Serial.print("I2C Soil Moisture Sensor Address: ");
  Serial.println(sensor.getAddress(),HEX);
  Serial.print("Sensor Firmware version: ");
  Serial.println(sensor.getVersion(),HEX);
  Serial.print("ESP ChipID: ");
  Serial.println(ESP.getChipId());
  display.version();
  Serial.println();

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

  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED)
  {
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
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

  Serial.print("Soil Moisture Capacitance: ");
  Serial.print(soilMoisture); //read capacitance register
  Serial.print(", Temperature: ");
  Serial.print(tempChirp); //temperature register
  Serial.print(", Light: ");
  Serial.print(light); //request light measurement, wait and read light register*/
  Serial.print(", DHT Temperature: ");
  Serial.print(temp);
  Serial.print(", DHT Humidity: ");
  Serial.print(humidity);
  Serial.print(", DHT HeatIndex: ");
  Serial.print(dht.computeHeatIndex(temp, humidity, false));
  Serial.print(", Voltage: ");
  Serial.print(voltage);
  Serial.print(", SensorValue: ");
  Serial.println(sensorValue);


  Serial.print("Sleep for ");
  Serial.print(SLEEPSECONDS);
  Serial.println(" seconds");

  // convert to microseconds
  ESP.deepSleep(SLEEPSECONDS * 1000000);
}
