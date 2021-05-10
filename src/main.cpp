#include <Arduino.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFiMulti.h>
#include <EventDispatcher.hpp>
#include <HTTPSClient.hpp>
#include <LittleFS.h>
#include <Timer.hpp>
#include <WiFiManager.hpp>

#define SSID "VM3549886"
#define PASSWORD "mc7RsdnxV4qp"

BearSSL::CertStore certStore;
Timer timer;
EventDispatcher dispatcher;
ESP8266WiFiMulti wifiMulti;

WiFiManager wifiManager(&wifiMulti, &dispatcher, &timer, SSID, PASSWORD);
HTTPSClient httpsClient(&certStore, &wifiManager, &timer);

const char *filename = "ndrdhx4k7kt61.jpeg";
char url[128];
char uploadURL[1600];

void uploadFile() {
  auto file = std::make_shared<File>(LittleFS.open(filename, "r"));

  httpsClient.sendRequest(
      Request::build(HTTPMethod::HTTP_PUT, uploadURL).body(file.get()),
      [file](Response res) {
        file->close();

        if (res.error != nullptr) {
          Serial.printf("err: %s\n", res.error);
          return;
        }

        Serial.printf("Response code: %d\n", res.statusCode);
      });
}

void setup() {
  Serial.begin(115200);
  delay(5000);

  LittleFS.begin();

  int numCerts =
      certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));

  Serial.printf("Number of CA certs read: %d\n", numCerts);

  sprintf(url,
          "https://d73oy8ung7.execute-api.eu-west-1.amazonaws.com/"
          "presigned_url?key=%s",
          filename);

  httpsClient.sendRequest(
      Request::build(HTTPMethod::HTTP_GET, url), [](Response res) {
        if (res.error != nullptr) {
          Serial.printf("err: %s\n", res.error);
          return;
        }

        res.body->readString().toCharArray(uploadURL, sizeof(uploadURL) - 1);

        Serial.print("Response Body: ");
        Serial.println(uploadURL);

        uploadFile();
      });
}

void loop() { timer.tick(); }