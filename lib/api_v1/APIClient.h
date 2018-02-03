/**
 * APIClient.h
 */
#ifndef APIClient_H_
#define APIClient_H_

// json parsing
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <DallasTemperature.h>

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

};

struct Token {
  String access_token;
  String client;
  String expiry;
  String uid;
};

// TODO: split this in api_config and wifi_config
// Configuration that we'll store on disk
struct Config {
  char host[64];
  int port;
  char sha_1[64];
  char wifi_ssid[64];
  char wifi_pw[64];
};

class APIClient
{
  public:

    APIClient();
    ~APIClient();

    // register to API
    // the Node structure must be build up before
    // with mac, pw, sensor_mac , etc.
    bool create_login(Node& node);
    // login to API and set header-credentials(token,uid,expiry,client)
    bool login(Node& node);
    // push sensor_data to API (for all sensors of node)
    bool push_sensor_data(Node& node);

  private:
    String build_json_data(Node& node);
    // create/register sensors to API
    bool create_sensors(Node& node);
    bool create_sensor(Node& node, Node::Sensor& sensor);
    void setHeaders();
    void getHeaders();
    void client_begin(const char * path);
    String request(method meth, const char* path, String request_body = "");
    int get_id(String json_string);
    String get_node_data(Node& node);

    // API tokens
    Token token;
    HTTPClient client;
};

#endif /* APIClient_H_ */
