
#include <Ethernet.h>
#include <SPI.h>

// wifi & http
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

// json parsing
#include <ArduinoJson.h>

// DS18B20 sensor
#include <OneWire.h>
#include <DallasTemperature.h>

#include <Arduino.h>

#include <APIClient.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS D1
// Setup a oneWire instance to communicate with any OneWire devices
OneWire one_wire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&one_wire);

#define HTTP_DEBUG


void setup() {

}

void loop() {

}
