// DoorController.cpp

#include <Arduino.h>
// #include <WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

const char *ssid = "zoomfrog";
const char *password = "8129839566";

const char *mqttServer = "192.168.2.158";
//const char *mqttServer = "10.81.104.102";
const int mqttPort = 1883;
const char *mqttUser = "user";
const char *mqttPassword = "user";

String IPaddress = "";
String MACaddress;
String deviceID;
String mqttTopic;
int numberKeyPresses = 0;
String msg;
String command;

int LED = 2;
String LedState = "off";
int switchPinOpen = 5;
int switchPinClosed = 4;
int bridgeA = 12;
int bridgeB = 14;

// variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;

//====== methods ===========================================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqttCallBack(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  msg = "";
  for (unsigned int j = 0; j < length; j++)
  {
    msg += (char)message[j];
  }
  Serial.print("Command received: ");
  Serial.println(msg);
  command = msg;
}
void IRAM_ATTR OpenCallBack()
{
  button_time = millis();
  numberKeyPresses++;
  if (button_time - last_button_time > 150)
  {
    last_button_time = button_time;
   Serial.print("Open Switch closed");
  }
}
void IRAM_ATTR CloseCallBack()
{
  button_time = millis();
  numberKeyPresses++;
  if (button_time - last_button_time > 150)
  {
    last_button_time = button_time;
    Serial.print("closed switch closed");
  }
}
void mqttConnect()
{
  // Loop until we are reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");

    Serial.println("Connecting to MQTT Server...");
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.connect("ESPClient", mqttUser, mqttPassword);
    mqttClient.setKeepAlive(600);
    mqttClient.setCallback(mqttCallBack);

    if (mqttClient.connected())
    {
      Serial.println("connected");
      // ... and resubscribe
      mqttTopic = deviceID;
      Serial.println("mqttTopic: " + mqttTopic);
      mqttClient.subscribe(mqttTopic.c_str());
      Serial.println(" Reconnected & subscribed: " + mqttTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// ##### SETUP  SETUP  SETUP  SETUP  SETUP  SETUP #######
// ######################################################################################
void setup()
{
  //  put your setup code here, to run once:
  Serial.println("Starting Setup()");

  Serial.begin(9600);
  // 9600,14,400, 19,200, 38,400, 57,600, and 115,200 baud

  // Configure I/O pins
  pinMode(LED, OUTPUT);
  pinMode(switchPinOpen, INPUT_PULLDOWN_16);
  pinMode(switchPinClosed, INPUT_PULLDOWN_16);

  //Serial.println("Setting limit switch callbacks...");
  // attachInterrupt(switchPinOpen, OpenCallBack, FALLING);
  // attachInterrupt(switchPinClosed, CloseCallBack, FALLING);

  pinMode(bridgeA, OUTPUT);
  pinMode(bridgeB, OUTPUT);

  // ################ WIFI ##############################################################
  Serial.print("WiFi Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.println(String(n) + " wifi network(s) found");

  /*
  for (int i = 0; i < n; i++)
  {
    // Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1,WiFi.SSID(i).c_str(),
    //     WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
  }
  */

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi");
    delay(200);
  }
  Serial.println("Connected to the WiFi network ");
  Serial.println("       SSID: " + String(WiFi.SSID()));
  Serial.println("    Channel: " + String(WiFi.channel()));
  Serial.println("   Strength: " + String(WiFi.RSSI()) + " dbm");
  Serial.println("MAC Address:" + WiFi.macAddress());
  MACaddress = WiFi.macAddress();
  Serial.println("SSID: " + WiFi.SSID());
  IPaddress = WiFi.localIP().toString().c_str();
  Serial.println("IP Address: " + IPaddress);
  // long rssi = WiFi.RSSI();

  // ########  Device id settings ###########################################
  Serial.println(" MAC Address: " + deviceID);
  deviceID = MACaddress.substring(MACaddress.length() - 5);
  Serial.println(" Device ID: " + deviceID);
  mqttClient.subscribe(deviceID.c_str());
  Serial.println(" Setup Subscribed: " + deviceID);

  // ######### MQTT SETUP ###########################################################
  // https://github.com/knolleary/pubsubclient/issues/163

  msg = " world (door)";
  mqttTopic = deviceID + "/info/Hello/";
  mqttClient.publish(mqttTopic.c_str(), msg.c_str());

  // zero volts across motor to turn off
  digitalWrite(bridgeA, LOW);
  digitalWrite(bridgeB, LOW);

  mqttTopic = deviceID + "/info/IPaddress/";
  msg = IPaddress;
  mqttClient.publish(mqttTopic.c_str(), msg.c_str());
  Serial.println("Published IP Address");

  msg = MACaddress;
  mqttTopic = deviceID + "/info/MACaddress/";
  mqttClient.publish(mqttTopic.c_str(), msg.c_str());
}
// END OF SETUP

//  ############## LOOP #################################################################
//  #####################################################################################
void loop()
{
  if (!mqttClient.connected())
  {
    mqttConnect();
  }
  delay(2000);
  command = "open";
  Serial.println(command);
  Serial.print(digitalRead(switchPinOpen));
  Serial.println(digitalRead(switchPinClosed));

  if (command == "open")
  {
    mqttTopic = deviceID + "/status";
    mqttClient.publish(mqttTopic.c_str(), command.c_str());
    Serial.println("Running motor CW");
    digitalWrite(bridgeA, HIGH);
    digitalWrite(bridgeB, LOW);
    while (command != "open")
    {
      if (digitalRead(switchPinOpen) == 0)
      {
        digitalWrite(bridgeA, LOW);
        digitalWrite(bridgeB, LOW);
        break;
      }
    }
  }

  command = "close";
  if (command == "close")
  {
    mqttTopic = deviceID + "/status";
    mqttClient.publish(mqttTopic.c_str(), command.c_str());
    Serial.println("Running motor CCW");
    digitalWrite(bridgeA, LOW);
    digitalWrite(bridgeB, HIGH);
    while (command != "stop")
    {
      if (digitalRead(switchPinClosed) == 0)
      {
        digitalWrite(bridgeA, LOW);
        digitalWrite(bridgeB, LOW);
        break;
      }
    }

    if (command == "stop")
    {
      mqttTopic = deviceID + "/status";
      mqttClient.publish(mqttTopic.c_str(), command.c_str());
      Serial.println("Running motor stop");
      digitalWrite(bridgeA, LOW);
      digitalWrite(bridgeB, LOW);
    }

    command = "";
    mqttClient.loop();
  }
}

/*
; PlatformIO Project Configuration File
;
;   Build options: build flags, source filter
;   Upload options: custom upload port, speed and extra flags
;   Library options: dependencies, extra library storages
;   Advanced options: extra scripting
;
; Please visit documentation for the other options and examples
; https://docs.platformio.org/page/projectconf.html

[env:esp12e]
monitor_speed = 9600
platform = espressif8266
board = esp12e
framework = arduino
lib_deps =
  khoih-prog/ESP_WifiManager@^1.12.1
  knolleary/PubSubClient@^2.8
*/

; PlatformIO Project Configuration File
;
;   Build options: build flags, source filter
;   Upload options: custom upload port, speed and extra flags
;   Library options: dependencies, extra library storages
;   Advanced options: extra scripting
;
; Please visit documentation for the other options and examples
; https://docs.platformio.org/page/projectconf.html

[env:esp12e]
monitor_speed = 9600
platform = espressif8266
board = esp12e
framework = arduino
lib_deps = 
	khoih-prog/ESP_WifiManager@^1.12.1
	knolleary/PubSubClient@^2.8
*/
