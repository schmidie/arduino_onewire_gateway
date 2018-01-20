#include <Arduino.h>
#include <Ethernet.h>
#include <SPI.h>
//#include "RestClient.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SD.h>

#define HTTP_DEBUG

const char* json_config = "{"
  "\"host\": \"api-v1.teufelswerk-berlin.de\","
  "\"port\": 443,"
  "\"wifi_ssid\": \"Speedy\","
  "\"wifi_pw\": \"Seifenblase\","
  "\"sha_1\": \"88:49:DD:29:76:84:DF:E3:3E:60:81:5E:7E:4A:D9:88:77:C1:67:9D\""
"}";

// RestClient client = RestClient("api-v1.teufelswerk-berlin.de", 443, 1);
HTTPClient client;

enum method {
  POST,
  GET
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
} node;
struct Sensor {
  String mac;
} sensor;

struct Token {
  String access_token;
  String client;
  String expiry;
  String uid;
} token;

bool parse_json(JsonObject* json, const char* str){

  // Allocate the memory pool on the stack.
  // Don't forget to change the capacity to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<512> jsonBuffer;
  // Parse the root object
  json = &jsonBuffer.parseObject(str);
  if (!json->success())
    return false;
  return true;
}

// Loads the configuration from a file
void load_config(const char* json_config) {

  // Allocate the memory pool on the stack.
  // Don't forget to change the capacity to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<512> jsonBuffer;

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

  node.mac = "a1b2c3d4e5f6g7h8"; // TODO
  node.pw = "123123123"; // TODO
  node.email = "test@test.de"; // TODO

  sensor.mac = "i1j2k3l4m5n6o7p8"; // TODO

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
      client.addHeader("Content-Type", "application/json");

}
void setHeaders() {
  if (token.access_token.length()) {
    Serial.print("set headers");
    client.addHeader("access_token", token.access_token);
    client.addHeader("client", token.client);
    client.addHeader("expiry", token.expiry);
    client.addHeader("uid", token.uid);
  }
}
// TODO: refactor to dont use String
String request(method meth, const char* path, String request_body = "") {
  setHeaders();

  client_begin(path);
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
    token.access_token = client.header("access_token");
    token.client = client.header("client");
    token.expiry = client.header("expiry");
    token.uid = client.header("uid");

    Serial.print(token.access_token);

    return response;
  } else{
    Serial.print("Cannot request " + String(path));
  }
  client.end();
  return "";
}


bool create_login() {
  String request_body = "{\"email\"=\"" + node.email + "\",\"mac\":\"" + node.mac + "\",\"password\":\"" + node.pw + "\"}";
  String response = request(POST, "/node_auth", request_body);
  if (response.length()){
    node.id = client.header("id").toInt();
    client.end();
    return true;
  }
  client.end();
  return false;
}

bool login() {
  String request_body = "{\"mac\":\"" + node.mac + "\",\"password\":\"" + node.pw + "\"}";
  String response = request(POST, "/node_auth/sign_in", request_body);
  if (response.length()){
    node.id = client.header("id").toInt();

    client.end();
    return true;
  }

  client.end();
  return false;
}

String get_node_data() {
  String request_body = "{\"mac\":\"" + node.mac + "\",\"password\":\"" + node.pw + "\"}";
  String path = "/nodes/" + String(node.id);
  String response = request(GET, const_cast<char*>(path.c_str()));

  client.end();
  return response;
}



void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);

    // Should load default config if run for the first time
    Serial.println(F("Loading configuration..."));
    load_config(json_config);

    connect_wlan();

    // client.setContentType("application/json");

    if(!login()){
      create_login();
    }
}


void loop() {
  login();
  Serial.print(get_node_data());
  delay(1000);

}
