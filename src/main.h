#ifndef MAIN_H_
#define MAIN_H_

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

// Prototypes
bool connect_wlan(int connection_tries = 5);
bool try_login();
void update_wifi(String ssid, String password);
void initialize();
void get_sensors_mac();

// API settings
const String host = "api-v1.teufelswerk-berlin.de";
const int port = 443;
const String sha_1 = "88:49:DD:29:76:84:DF:E3:3E:60:81:5E:7E:4A:D9:88:77:C1:67:9D";

Node node;
APIClient client(host, port, sha_1);

struct WifiConfig{
  String ssid;
  String password;
  std::vector<std::pair<String, String>> networks;
};
static WifiConfig config;

bool try_login(){
  if (config.ssid.length()){
    // if node not exist on API -> create
    if(!client.login(node)){
      client.create_login(node);
    }
  }
  return client.logged_in(node);
}

// TODO: we should save this to EEPROM
void update_wifi(String ssid, String password){
  // save to the remember wifi list
  // std::pair<String,String> wifi = std::make_pair(ssid, password);
  // if(std::find(config.networks.begin(), config.networks.end(), wifi) == config.networks.end()) {
  //   config.networks.push_back(wifi);
  // }
  // set the actual wifi
  config.ssid = ssid;
  config.password = password;
}


void get_sensor_data(){
  // Send the command to get temperature readings
  sensors.requestTemperatures();

  //TODO: save real timestamp
  for(int i = 0; i < node.sensors.size(); i++){
    float temp = sensors.getTempC(node.sensors[i].addr);
    Node::Sensor::Data data;
    data.value = temp;
    data.unity = "celsius";
    node.sensors[i].data.push_back(data);
  }
}

void get_sensors_mac(){
  DeviceAddress mac;

  while ( one_wire.search(mac)) {
    char macStr[24] = { 0 };
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7]);

    Node::Sensor sensor;
    sensor.mac = String(macStr);
    memcpy( sensor.addr, mac, sizeof(DeviceAddress) );
    node.sensors.push_back(sensor);
  }

  one_wire.reset_search();
}

void initialize(){
  // get node settings
  node.mac = WiFi.macAddress();
  node.pw = "123123123"; // TODO
  node.email = node.mac + "@energeer.de";

  // get mac addresses of all sensors
  get_sensors_mac();
}

bool connect_wlan(int connection_tries) {
  if(config.ssid.length()){
    WiFi.begin(config.ssid.c_str(), config.password.c_str());
    Serial.print("Connecting");

    for(int i=0; i < connection_tries; i++)
    {
      delay(500);
      Serial.print(".");
    }

    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    return (WiFi.status() == WL_CONNECTED);
  }
  return false;
}


#endif /* MAIN_H_ */
