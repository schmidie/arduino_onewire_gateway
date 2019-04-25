// webserver
#include <ESP8266WebServer.h>
//#include <ESP8266WebServerSecure.h>
// multicast DNS for nice local domain name
#include <ESP8266mDNS.h>

// ESP Webserver files (html and certificates)
//#include "server/cert.h"
#include "server/index.h"

#include <main.h>

//ESP8266WebServerSecure server(443);
//Server on port 80
ESP8266WebServer server(80);

//SSID and Password to our ESP-Server
const char* ssid = "ENERGEER";
const char* password = "";

void handleRoot() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", INDEX_PAGE); //Send web page
}

void handleSettings() {

  if (server.arg("ssid")!= ""){

    // TODO: add API-key
    update_wifi(server.arg("ssid"), server.arg("password"));
    update_secret(server.arg("secret"));
 
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", INDEX_PAGE);
  }
  else {
    server.sendHeader("Connection", "close");
    server.send(401, "text/html", INDEX_PAGE);
  }
}

// TODO: do somehow like this
// static const char* result = R"=====(<HTML><p style="align:center">Eingaben wurden gespeichert!</p></HTML>)=====";
// char buffer[sizeof(result)+sizeof(INDEX_PAGE)+16];
// strncpy(buffer, result, sizeof(result));
// strncat(buffer, INDEX_PAGE, sizeof(INDEX_PAGE));
