//#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ThingsBoard.h>

//Dallas
//#include <OneWire.h>
//#include <DallasTemperature.h>

//BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


//TOKEN - used to determine which device this is.
#define TOKEN "CccQ5Gm6k3W58sMCPvDZ"  //Floater2
//Used to adjust the sensors as needed.  This one for example seemed 2 degrees lower than the rest.

//Used to calibrate this sensors tempteture
float calibration = 0;

/*#include <SPI.h>
#define BME_SCK 14
#define BME_MISO 12
#define BME_MOSI 13
#define BME_CS 15*/

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime;


//Dallas
//DSB setup
// GPIO where the DS18B20 is connected to
//const int oneWireBus = 5;
// Setup a oneWire instance to communicate with any OneWire devices
//OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
//DallasTemperature sensors(&oneWire);     


#define WIFI_AP "Brewers_2.4G"
#define WIFI_PASSWORD "4MyPackers"


// DHT
#define DHTPIN 2
#define DHTTYPE DHT22

char thingsboardServer[] = "192.168.1.67";

WiFiClient wifiClient;

// Initialize DHT sensor.
//DHT dht(DHTPIN, DHTTYPE);

ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  Serial.begin(115200);
  
  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  //delayTime = 1000;

  Serial.println();
  delay(10);
  InitWiFi();
  lastSend = 0;
  
  //Dallas
  //sensors.begin();
}

void loop()
{
  Serial.println("starting");
  if ( !tb.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  tb.loop();
}

void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  //float humidity = dht.readHumidity();
  
  // Read temperature as Celsius (the default)
  //float temperature = dht.readTemperature();
  //sensors.requestTemperatures();
  float temperature = ((1.8 * bme.readTemperature() + 32) + calibration);

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.println("Sending data to ThingsBoard:");
  //Serial.print("Humidity: ");
  //Serial.print(humidity);
  //Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *F ");

  tb.sendTelemetryFloat("temperature", temperature);
  tb.sendTelemetryFloat("pressure", bme.readPressure() / 100.0F);
  tb.sendTelemetryFloat("altitude", bme.readAltitude(SEALEVELPRESSURE_HPA));
  tb.sendTelemetryFloat("humidity", bme.readHumidity());
  //tb.sendTelemetryFloat("humidity", humidity);
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( tb.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
