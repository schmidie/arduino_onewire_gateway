#include <main.h>
#include <server.h>

// loop cycle for controlling the processor loop
int cycle = 0;
// in which loop_interval we want to push sensor data
int push_interval = 1;
// is node connected to wlan ?
bool wlan = false;

void setup() {
    Serial.begin(9600);

    // Start up the library
    sensors.begin();

    WiFi.mode(WIFI_AP_STA);

    //Start Hotspot
    WiFi.softAP(ssid);

    if (MDNS.begin("energeer")) {
      Serial.println("MDNS responder started");
    }

    // IPAddress myIP = WiFi.softAPIP(); //Get IP address
    // Serial.print("HotSpt IP:");
    // Serial.println(myIP);
    // server.setServerKeyAndCert_P(rsakey, sizeof(rsakey), x509, sizeof(x509));

    server.on("/", handleRoot);
    server.on("/settings", handleSettings);
    server.begin();

    // initialize all config (mac, sensors_mac, password)
    initialize();
    // try to login API
    //try_login();
}

void loop() {
  //Handle client requests
  server.handleClient();

  if(!wlan){
   wlan = connect_wlan();
  }

  // try to login to API, if we are not allready logged in
  if(wlan && !client.logged_in(node)){
    try_login();
  }

  // save sensor_data in buffer (for all sensors on bus)
  get_sensor_data();

  if(client.logged_in(node) && cycle % push_interval == 0){
    // push sensor_data to API
    client.push_sensor_data(node);
    cycle = 0;
  }

  // check if we are still connected
  if(WiFi.status() != WL_CONNECTED){
    wlan = false;
  }
  cycle++;
  delay(1000);
}
