/**
 * APIClient.h
 */
#ifndef APIClient_H_
#define APIClient_H_

// json parsing
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <DallasTemperature.h>

HTTPClient client;

enum method {
  POST,
  GET
};
enum unity {
  celsius,
  fahrenheit
};

struct Node {
  String mac;
  String pw;
  String email;
  int id;

  struct Sensor {
    String mac; // String representation of addr
    DeviceAddress addr;
    int id;
    struct Data {
      float value;
      String unity;
    };
    std::vector<Node::Sensor::Data> data;
  };
  std::vector<Node::Sensor> sensors;

} node;

struct Token {
  String access_token;
  String client;
  String expiry;
  String uid;
} token;

// TODO: split this in api_config and wifi_config
// Configuration that we'll store on disk
struct Config {
  char host[64];
  int port;
  char sha_1[64];
  char wifi_ssid[64];
  char wifi_pw[64];
} config;

class APIClient
{
  public:
    APIClient();
    ~APIClient();

    // create/register sensors to API
    bool create_sensors(Node& node);
    // register to API
    bool create_login();
    // login to API and set header-credentials(token,uid,expiry,client)
    bool login();

    // push sensor_data to API
    bool push_sensor_data();


  private:
    String build_json_data();
    bool create_sensor(Node& node, Node::Sensor& sensor);
    void setHeaders();
    void getHeaders();
    void client_begin(const char * path);
    String request(method meth, const char* path, String request_body = "");
    int get_id(String json_string);
    String get_node_data();

};

#endif /* APIClient_H_ */
