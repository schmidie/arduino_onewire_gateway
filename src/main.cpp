
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

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS D1
// Setup a oneWire instance to communicate with any OneWire devices
OneWire one_wire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&one_wire);

#define HTTP_DEBUG

// TODO:multiple ssids/wlans
const char* json_config = "{"
  "\"host\": \"api-v1.teufelswerk-berlin.de\","
  "\"port\": 443,"
  "\"wifi_ssid\": \"Speedy\","
  "\"wifi_pw\": \"Seifenblase\","
  "\"sha_1\": \"88:49:DD:29:76:84:DF:E3:3E:60:81:5E:7E:4A:D9:88:77:C1:67:9D\""
"}";

HTTPClient client;

enum method {
  POST,
  GET
};
enum unity {
  celsius,
  fahrenheit
};

// Configuration that we'll store on disk
struct Config {
  char host[64];
  int port;
  char sha_1[64];
  char wifi_ssid[64];
  char wifi_pw[64];
} config;

struct Node {
  String mac;
  String pw;
  String email;
  int id;

  struct Sensor {
    String mac; // String representation of addr
    DeviceAddress addr;
    int id;
    std::vector<String> data;
  };
  std::vector<Node::Sensor> sensors;

} node;

struct Token {
  String access_token;
  String client;
  String expiry;
  String uid;
} token;

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

// initialize
void initialize(){
  // get node settings
  node.mac = WiFi.macAddress();
  node.pw = "123123123"; // TODO
  node.email = "test@test.de"; // TODO

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

void client_begin(const char * path){
      client.begin(config.host, config.port, path , config.sha_1); //HTTPS
      client.setReuse(true);
      client.addHeader("Content-Type", "application/json");

}
void setHeaders() {
  if (token.access_token.length()) {
    Serial.print("set headers");
    client.addHeader("access-token", token.access_token);
    client.addHeader("client", token.client);
    client.addHeader("expiry", token.expiry);
    client.addHeader("uid", token.uid);
  }
}

void getHeaders() {
  const char * headerkeys[] = {"access-token","client","expiry","uid"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  client.collectHeaders(headerkeys,headerkeyssize);
}

// TODO: refactor to dont use String
String request(method meth, const char* path, String request_body = "") {

  client_begin(path);
  setHeaders();
  getHeaders();

  int statusCode = -1;
  if (meth == POST) {
    statusCode = client.POST(request_body);
  }
  else if(meth == GET) {
    statusCode = client.GET();
  }
  else {
    client.end();
    Serial.print("HTTP method not supported");
    return "";
  }

  String response = client.getString();;
  Serial.print(response);
  Serial.print("statuscode: " + String(statusCode));

  if(statusCode == 200){
    token.access_token = client.header("access-token");
    token.client = client.header("client");
    token.expiry = client.header("expiry");
    token.uid = client.header("uid");

    client.end();
    return response;

  } else{
    Serial.print("Cannot request " + String(path));
  }
  client.end();
  return "";
}

int get_id(String json_string){
  StaticJsonBuffer<512> jsonBuffer;
  // Parse the root object
  JsonObject &json = jsonBuffer.parseObject(json_string);

  if (!json.success()){
    Serial.println(F("Failed to read json"));
    return -1;
  }
  return json["data"]["id"];
}

bool create_login() {
  String request_body = "{\"email\"=\"" + node.email + "\",\"mac\":\"" + node.mac + "\",\"password\":\"" + node.pw + "\"}";
  String response = request(POST, "/node_auth", request_body);
  if (response.length()){
    node.id = get_id(response);
    return true;
  }
  return false;
}

bool login() {
  String request_body = "{\"mac\":\"" + node.mac + "\",\"password\":\"" + node.pw + "\"}";
  String response = request(POST, "/node_auth/sign_in", request_body);
  if (response.length()){
    node.id = get_id(response);
    return true;
  }
  return false;
}

String get_node_data() {
  String path = "/nodes/" + String(node.id);
  return request(GET, const_cast<char*>(path.c_str()));
}

bool create_sensor(Node& node, Node::Sensor& sensor) {
  String request_body = "{\"mac\":\"" + sensor.mac + "\"}";
  String path = "/nodes/" + String(node.id) + "/sensors/";
  String response = request(POST, const_cast<char*>(path.c_str()), request_body);
  if (response.length()){
    sensor.id = get_id(response);
    return true;
  }
  return false;
}

bool create_sensors(Node& node) {
  for(int i = 0; i < node.sensors.size(); i++){
    create_sensor(node, node.sensors[i]);
  }
}

bool push_sensor_data(String* sensor_data,int data_count, String sensor_mac) {
  // Allocate the memory pool on the stack.
  StaticJsonBuffer<4000> jsonBuffer; //TODO: capacity: arduinojson.org/assistant
  JsonObject& _json = jsonBuffer.createObject();
  JsonArray& _sensors = _json.createNestedArray("sensors");

  JsonObject& _s = _sensors.createNestedObject();
  _s["mac"] = sensor_mac;
  // JsonArray& _sensor_data = _s.createNestedArray("sensor_data");
  // for(int i = 0; i < data_count; i++){
  //   _sensor_data.add(sensor_data[i]);
  // }
  _json.printTo(Serial);
  // String request_body = "{\"mac\":\"" + sensor.mac + "\"}";
  // String path = "/nodes/" + String(node.id) + "/sensor_data/";
  // String response = request(POST, const_cast<char*>(path.c_str()), request_body);

}

// TODO: with created_at
String json_data(float value, String u) {
    // Allocate the memory pool on the stack.
    StaticJsonBuffer<512> jsonBuffer; //TODO: capacity: arduinojson.org/assistant
    JsonObject& json = jsonBuffer.createObject();
    json["value"] = value;
    json["unity"] = u;
    String json_data = "";
    json.printTo(json_data);
    return json_data;
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
    //connect_wlan();

    // if(!login()){
    //   create_login();
    // }
    // create_sensor();
}


void loop() {
  //login();
  //Serial.print(get_node_data());
  Serial.println("start loop");

  Serial.println(node.mac);
  for(int i = 0; i < node.sensors.size(); i++){
    Serial.println(node.sensors[i].mac);
  }
  // request sensor data
  // sensors.requestTemperatures(); // Send the command to get temperature readings
  // float temp = sensors.getTempCByIndex(0);
  // // push sensor data to buffer
  // sensor.data[sensor.data_count] = json_data(temp, "celsius");
  // sensor.data_count ++;
  //
  // push_sensor_data(sensor.data,sensor.data_count, sensor.mac);
  delay(1000);

}
