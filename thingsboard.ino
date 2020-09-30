//#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ThingsBoard.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//3rd Bedroom, Master Bedroom, Basement
//3rd Bedroom

//TOKEN - used to determine which device this is.
//#define TOKEN "kEm7Ts2TukJXdzrXwvyj" //3rd bedroom
//#define TOKEN "GtjJi7TtXgoanRbw5Wxp" //Master
#define TOKEN "vujHQAl7x6j5yg8J3TgZ" //Basement


//Used to adjust the sensors as needed.  value of 2 would increase the temp 2
//float calibration = 0; //3rd bedroom
//float calibration = 2.5; //Master
float calibration = 1; //Basement

//DSB setup
// GPIO where the DS18B20 is connected to
const int oneWireBus = 5;
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);     


#define WIFI_AP "Brewers_2.4G"
#define WIFI_PASSWORD "4MyPackers"


// DHT
#define DHTPIN 2
#define DHTTYPE DHT22

char thingsboardServer[] = "192.168.1.111";

WiFiClient wifiClient;

// Initialize DHT sensor.
//DHT dht(DHTPIN, DHTTYPE);

ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  Serial.begin(115200);
  //dht.begin();
  delay(10);
  InitWiFi();
  lastSend = 0;
  sensors.begin();
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
  sensors.requestTemperatures();
  float temperature = (sensors.getTempFByIndex(0) + calibration);
  float minTemp = 50;

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  //Checks to see if the device read a low number.  Seems to happen every so often throwing averages way off
  if (temperature < minTemp) {
    Serial.println("Failed to read from DHT sensor!  Too Low!");
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
