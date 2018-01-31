
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

APIClient apiClient;

// TODO:multiple ssids/wlans
const char* json_config = "{"
  "\"host\": \"api-v1.teufelswerk-berlin.de\","
  "\"port\": 443,"
  "\"wifi_ssid\": \"Speedy\","
  "\"wifi_pw\": \"Seifenblase\","
  "\"sha_1\": \"88:49:DD:29:76:84:DF:E3:3E:60:81:5E:7E:4A:D9:88:77:C1:67:9D\""
"}";



void get_sensors_mac(){
  DeviceAddress mac;

  while ( one_wire.search(mac)) {
    char macStr[24] = { 0 };
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7]);
    // TODO: APICLient method for adding sensors
    Node::Sensor sensor;
    sensor.mac = String(macStr);
    memcpy( sensor.addr, mac, sizeof(DeviceAddress) );
    node.sensors.push_back(sensor);
  }

  one_wire.reset_search();
}

void get_sensor_data(){
  // Send the command to get temperature readings
  sensors.requestTemperatures();

  for(int i = 0; i < node.sensors.size(); i++){
    float temp = sensors.getTempC(node.sensors[i].addr);
    // TODO: APICLient method
    Node::Sensor::Data data;
    data.value = temp;
    data.unity = "celsius";
    node.sensors[i].data.push_back(data);
  }
}

// initialize
void initialize(){
  // get node settings
  node.mac = WiFi.macAddress();
  node.pw = "123123123"; // TODO
  node.email = "node1@energeer.de"; // TODO

  get_sensors_mac();
}

// Loads the configuration from a file
void load_config(const char* json_config) {

  // Allocate the memory pool on the stack.
  StaticJsonBuffer<512> jsonBuffer; //TODO: capacity: arduinojson.org/assistant
  // Parse the root object
  JsonObject &root = jsonBuffer.parseObject(json_config);

  if (!root.success())
    Serial.println(F("Failed to read config, using default configuration"));

  // Copy values from the JsonObject to the Config
  config.port = root["port"] | 443;
  strlcpy(config.host, root["hostname"] | "api-v1.teufelswerk-berlin.de" ,sizeof(config.host));
  strlcpy(config.wifi_ssid, root["wifi_ssid"] | "" ,sizeof(config.wifi_ssid));
  strlcpy(config.wifi_pw, root["wifi_pw"] | "" ,sizeof(config.wifi_pw));
  strlcpy(config.sha_1, root["sha_1"] | "" ,sizeof(config.sha_1));
}

void connect_wlan() {
  WiFi.begin(config.wifi_ssid, config.wifi_pw);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
    // Serial.setDebugOutput(true);
    // put your setup code here, to run once:
    Serial.begin(9600);

    // Start up the library
    sensors.begin();

    // Should load default config if run for the first time
    //Serial.println(F("Loading configuration..."));
    load_config(json_config);

    initialize();
    connect_wlan();

    // // if node not exist on API -> create
    if(!apiClient.login()){
      apiClient.create_login();
    }
    apiClient.login();
    // // if sensors not exist on API -> create
    apiClient.create_sensors(node);
}

void loop() {
  apiClient.login();
  // save sensor_data in buffer (for all sensors on bus)
  get_sensor_data();
  // push sensor_data to API
  apiClient.push_sensor_data();
  delay(3000);
}
