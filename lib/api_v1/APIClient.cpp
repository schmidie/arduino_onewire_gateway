/**
 * APIClient.cpp
 */
#include <APIClient.h>

/**
 * constructor
 */
APIClient::APIClient(){}

/**
 * destructor
 */
APIClient::~APIClient(){}

void APIClient::setHeaders() {
  if (token.access_token.length()) {
    Serial.print("set headers");
    client.addHeader("access-token", token.access_token);
    client.addHeader("client", token.client);
    client.addHeader("expiry", token.expiry);
    client.addHeader("uid", token.uid);
  }
}

bool APIClient::push_sensor_data(){
  String path = "/nodes/" + String(node.id) + "/sensor_data/";
  String response = request(POST, const_cast<char*>(path.c_str()), build_json_data());
  Serial.println(response);
}

void APIClient::getHeaders() {
  const char * headerkeys[] = {"access-token","client","expiry","uid"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  client.collectHeaders(headerkeys,headerkeyssize);
}

void APIClient::client_begin(const char * path){
      //client.begin(config.host, config.port, path , config.sha_1); //HTTPS
      client.setReuse(true);
      client.addHeader("Content-Type", "application/json");

}

// TODO: refactor to dont use String
String APIClient::request(method meth, const char* path, String request_body) {

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

int APIClient::get_id(String json_string){
  StaticJsonBuffer<512> jsonBuffer;
  // Parse the root object
  JsonObject &json = jsonBuffer.parseObject(json_string);

  if (!json.success()){
    Serial.println(F("Failed to read json"));
    return -1;
  }
  return json["data"]["id"];
}

bool APIClient::create_login() {
  String request_body = "{\"email\":\"" + node.email + "\",\"mac\":\"" + node.mac + "\",\"password\":\"" + node.pw + "\"}";
  String response = request(POST, "/node_auth", request_body);
  if (response.length()){
    node.id = get_id(response);
    return true;
  }
  return false;
}

bool APIClient::login() {
  String request_body = "{\"mac\":\"" + node.mac + "\",\"password\":\"" + node.pw + "\"}";
  String response = request(POST, "/node_auth/sign_in", request_body);
  if (response.length()){
    node.id = get_id(response);
    return true;
  }
  return false;
}

String APIClient::get_node_data() {
  String path = "/nodes/" + String(node.id);
  return request(GET, const_cast<char*>(path.c_str()));
}

bool APIClient::create_sensor(Node& node, Node::Sensor& sensor) {
  String request_body = "{\"mac\":\"" + sensor.mac + "\"}";
  String path = "/nodes/" + String(node.id) + "/sensors/";
  String response = request(POST, const_cast<char*>(path.c_str()), request_body);
  if (response.length()){
    sensor.id = get_id(response);
    return true;
  }
  return false;
}

bool APIClient::create_sensors(Node& node) {
  bool retval = true;
  for(int i = 0; i < node.sensors.size(); i++){
    retval = create_sensor(node, node.sensors[i]);
    if(!retval) {
      Serial.println("error: create remote sensor");
    }
  }
  return retval;
}

String APIClient::build_json_data() {
  // Allocate the memory pool on the stack.
  StaticJsonBuffer<1024> jsonBuffer; //TODO: capacity: arduinojson.org/assistant
  JsonObject& _json = jsonBuffer.createObject();
  JsonArray& _sensors = _json.createNestedArray("sensors");

  // get all sensors
  for(int i = 0; i < node.sensors.size(); i++){
    JsonObject& _s = _sensors.createNestedObject();
    _s["mac"] = node.sensors[i].mac;
    JsonArray& _data = _s.createNestedArray("sensor_data");
    // get all data
    for(int j = 0; j < node.sensors[i].data.size(); j++){
      JsonObject& _datum = _data.createNestedObject();
      _datum["value"] = node.sensors[i].data[j].value;
      _datum["unity"] = node.sensors[i].data[j].unity;
    }
    // clear data_buffer
    node.sensors[i].data.clear();
  }

  String json_data = "";
  _json.printTo(json_data);

  return json_data;

}
