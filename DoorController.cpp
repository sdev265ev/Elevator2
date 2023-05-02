/*
DoorController.cpp
ESP8266 microcontroller
Controls passenger door opening and closing
Accepts mqtt commands: open, close, stop, speed
Reports to mqtt door status (position): opened, opening, closed
Reports the current speed 0-1000
*/

#include <Arduino.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

const char *ssid = "sdev265";
const char *password = "sdev265$";

const char *mqttServer = "10.81.104.102";
const int mqttPort = 1883;
const char *mqttUser = "";
const char *mqttPassword = "";

String IPaddress = "";
String MACaddress;
String deviceID;
String mqttTopic;
int counter = 0;
int numberKeyPresses = 0;
String ss;
String msg;
String command;
int count = 0;
int limit;
String doorStatus = "";
int LED = 2;
String LedState = "off";
int switchPinOpen = 5;
int switchPinClosed = 4;
int bridgeA = 12;
int bridgeB = 14;
int speed = 100;
//=================================================================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void ConfigureIO()
{
  // put your setup code here, to run once:

  // Configure I/O pins
  pinMode(LED, OUTPUT);
  pinMode(switchPinOpen, INPUT_PULLDOWN_16);
  pinMode(switchPinClosed, INPUT_PULLDOWN_16);
  // Modes: LOW, HIGH, CHANGE, FALLING, RISING
  pinMode(bridgeA, OUTPUT);
  pinMode(bridgeB, OUTPUT);
  // zero volts across motor to turn off
  digitalWrite(bridgeA, LOW);
  digitalWrite(bridgeB, LOW);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("I/O configured");
  // 9600, 14400, 19200, 38400, 57600, and 115200 baud
}

void mqttCallBack(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  msg = "";
  for (unsigned int j = 0; j < length; j++)
  {
    msg += (char)message[j];
  }

  if (deviceID + "/command" == topic)
  {
    Serial.print("Command received: ");
    Serial.println(msg);
    command = msg;
  }
  if (deviceID + "/speed" == topic)
  {
    Serial.println("Speed received: " + msg);

    speed = msg.toInt();
    if (speed == 0)
    {
      speed = 5;
    }
  }
}

void mqttPublish(String topic, String msg)
{
  topic = "elev/" + topic;
  mqttClient.publish(topic.c_str(), msg.c_str());
}

void mqttConnect()
{
  // Loop until we are reconnected
  Serial.println("now in mqttConnect");
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqttCallBack); // callback executed when message received
    mqttClient.connect("ESPClient", mqttUser, mqttPassword);

    // mqttClient.setKeepAlive(600);

    if (mqttClient.connected())
    {
      Serial.println("mqtt connected");

      Serial.println(" Device ID: " + deviceID);

      mqttTopic = deviceID + "/#";
      mqttClient.subscribe(mqttTopic.c_str());
      Serial.println("subscribed to: " + mqttTopic);

      Serial.println("MQTT callback set");
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

void setup()
{
  ConfigureIO();
  
  //=========== WIFI ==================================================
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(10);
  Serial.println("");
  Serial.println("Scan start... ");

  /*
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++)
    {
      Serial.print(n);
      Serial.println(" network(s) found");
      Serial.println(WiFi.SSID(i));

      Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
    }
    Serial.println();
  */
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println("Connected to the WiFi network ");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  MACaddress = WiFi.macAddress();
  deviceID =  MACaddress.substring(MACaddress.length() - 5) ;
 
  mqttConnect();
}

//===============================================================


//*************************** LOOP ***********************************
void loop()
{
  // put your main code here, to run repeatedly:

  if (!mqttClient.connected())
  {
    mqttConnect();
  }

  // delay(50);
  limit = 1;
  // Serial.print("command: " + command);
  // Serial.print("Speed: " + String(speed));
  // Serial.print(digitalRead(switchPinOpen) + " : ");
  // Serial.println(digitalRead(switchPinClosed));
  mqttTopic = deviceID + "/status";

  digitalWrite(bridgeA, LOW);
  digitalWrite(bridgeB, LOW);
  count += 1;
  if (command == "open")
  {
    mqttPublish(deviceID + "/status", "opening");
    Serial.println("Running motor CW");
    digitalWrite(bridgeA, HIGH);
    digitalWrite(bridgeB, LOW);
    while (limit == 1 && command == "open")
    {
      digitalWrite(bridgeA, HIGH);
      digitalWrite(bridgeB, LOW);
      delay(speed);
      digitalWrite(bridgeA, LOW);
      digitalWrite(bridgeB, LOW);
      delay(100 - speed);

      limit = digitalRead(switchPinOpen);
      mqttPublish(deviceID + "/status", "opened");
      mqttClient.loop();
      // Serial.println(limit);
    }
  }

  if (command == "close")
  {
    mqttPublish(deviceID + "/status", "closing");
    Serial.println("Running motor CCW");
    digitalWrite(bridgeA, LOW);
    digitalWrite(bridgeB, HIGH);
    Serial.println("Running motor CCW wait for switch close");
    while (limit == 1 && command == "close")
    {
      
      limit = digitalRead(switchPinClosed);
      mqttPublish(deviceID + "/status", "closed");
      mqttClient.loop();
    }

    if (command == "stop")
    {
      mqttTopic = deviceID + "/status";
      doorStatus = "stalled";
      mqttClient.publish(mqttTopic.c_str(), doorStatus.c_str());
      Serial.println("Running motor stop");
      digitalWrite(bridgeA, LOW);
      digitalWrite(bridgeB, LOW);
    }
  }
  command = "";
  digitalWrite(bridgeA, LOW);
  digitalWrite(bridgeB, LOW);

  mqttClient.loop();
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

[env:nodemcuv2]
monitor_speed = 9600
platform = espressif8266
board = nodemcuv2
framework = arduino
lib_deps = 
	tzapu/WiFiManager@^0.16.0
	knolleary/PubSubClient@^2.8
	marvinroger/AsyncMqttClient@^0.9.0
*/
