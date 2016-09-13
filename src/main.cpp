#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <dht.h>
#include <I2CSoilMoistureSensor.h>

//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include "My_MicroOLED.h"

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define OLED_RESET 0  // GPIO0
My_MicroOLED display(OLED_RESET);

#define DHTPIN D4     // what pin DHT connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define BLUE_LED D6
#define GREEN_LED D7
#define RED_LED D8

#define SCL_PIN D1
#define SDA_PIN D2

I2CSoilMoistureSensor sensor;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  pinMode(BLUE_LED, OUTPUT);  // set onboard LED as output
  pinMode(GREEN_LED, OUTPUT);  // set onboard LED as output
  pinMode(RED_LED, OUTPUT);  // set onboard LED as output

  digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage HIGH
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(RED_LED, LOW);

  Serial.begin(9600);
  Serial.println("PlantSensor V3.0 Beta");

  Wire.begin(SDA_PIN, SCL_PIN);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  //sensor.begin(); // reset sensor
  //delay(1000); // give some time to boot up

  // init done

  Serial.print("I2C Soil Moisture Sensor Address: ");
  //Serial.println(sensor.getAddress(),HEX);
  Serial.print("Sensor Firmware version: ");
  //Serial.println(sensor.getVersion(),HEX);
  Serial.print("ESP ChipID: ");
  Serial.println(ESP.getChipId());
  display.version();
  Serial.println();

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Hello, wor");
  /*display.setTextColor(BLACK, WHITE); // 'inverted' text
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


  display.display();
   dht.begin();

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

/*
  delay(10000);
  while (sensor.isBusy()) delay(50); // available since FW 2.3
  float soilMoisture = sensor.getCapacitance();
  float tempChirp = sensor.getTemperature()/(float)10;
  float light = sensor.getLight(true);
  sensor.sleep(); // available since FW 2.3



  Serial.print("Soil Moisture Capacitance: ");
  Serial.print(soilMoisture); //read capacitance register
  Serial.print(", Temperature: ");
  Serial.print(tempChirp); //temperature register
  Serial.print(", Light: ");
  Serial.print(light); //request light measurement, wait and read light register*/

  delay(10000);
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print(", DHT Temperature: ");
  Serial.print(temp);
  Serial.print(", DHT Humidity: ");
  Serial.print(humidity);
  Serial.print(", DHT HeatIndex: ");
  Serial.println(dht.computeHeatIndex(temp, humidity, false));



}
