#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>

#define ARDUINOJSON_USE_DOUBLE 0
#include <ArduinoJson.h>

#include "uptime_formatter.h"

const int buttonPin = 15;             // the number of the pushbutton pin
const float calibration = 0.2794;     // rain quantity for each tick in mm

int buttonState = 0;                  // variable for reading the pushbutton status
unsigned int tickCount = 0;
unsigned long lastDebounceTime = 0;   // the last time the input pin was toggled
unsigned long debounceDelay = 300;    // the debounce time; increase if the output flickers

#ifndef STASSID
#define STASSID "dd-wrt"
#define STAPSK  "Cristi01"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

ESP8266WebServer httpServer(80);

void handleRainQTY() {
  /*
  String json = "{\"data\":{\"rainqty\":";
  json += String(tickCount * calibration, 4);
  json += "}}";
  httpServer.send(200, "application/json", json);
*/
  
  JsonDocument doc;
  String output;
  
  doc["data"]["rainqty"] = tickCount * calibration;
  serializeJsonPretty(doc, output);
  httpServer.send(200, "application/json", output);
  tickCount = 0;
}

void reset() {
  ESP.restart();
}

void uptime() {
  Serial.println("up " + uptime_formatter::getUptime());
  httpServer.send(200, "text/plain", uptime_formatter::getUptime());
}

void setup() {
  pinMode(buttonPin, INPUT);          // initialize the pushbutton pin as an input
  pinMode(LED_BUILTIN, OUTPUT);       // Initialize the LED_BUILTIN pin as an output

  Serial.begin(115200);
  Serial.println();
  Serial.println("RainQTY Start");

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  httpServer.on("/", []() {
    httpServer.send(200, "text/plain", "System alive");
  });

  httpServer.on("/rainqty", handleRainQTY);

  httpServer.on("/reboot", reset);

  httpServer.on("/uptime", uptime);

  ElegantOTA.begin(&httpServer);
  httpServer.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
    if (buttonState == HIGH) {
      lastDebounceTime = millis();
      digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
      tickCount += 1;
      Serial.print((tickCount * calibration), 4);
      Serial.print("\n");
    } else {
      digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    }
  }

  httpServer.handleClient();
  ElegantOTA.loop();
}