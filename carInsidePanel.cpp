#include <Arduino.h>
#include <WiFi.h>
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
int numberKeyPresses = 0;
String msg;

int LED = 2;
String LedState = "off";
int keyLockPin = 21;
String keyState = "off";

// variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;
bool buttonPressed = false;

// ######## BUTTONS/LAMPS ##########################################################
String ButtonNames[] = {"F1B", "F2B", "F3B", "F4B", "F5B", "KEY"};
int ButtonPins[] = {22, 18, 4, 32, 27, 21}; // 21 is the key lock
int LampPins[] = {23, 5, 16, 33, 14};
int ButtonPressCount[] = {0, 0, 0, 0, 0, 0};
//====== methods ===========================================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqttCallBack(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  msg = "";
  int onOff = LOW;

  for (int i = 0; i < length; i++)
  {
    msg += (char)message[i];
  }
  int lampID = msg.toInt();
  if (lampID < 0)
  {
    onOff = HIGH;
    lampID = abs(lampID);
  }

  int pin = LampPins[lampID - 1];

  // Serial.println("lamp on - pin: " + pin);
  // Serial.println("onoff: " + onOff);
  digitalWrite(pin, onOff);
  ButtonPressCount[lampID - 1] = 0;
  delay(100);

  msg = "0";
  mqttTopic = deviceID + "/button/" + ButtonNames[lampID - 1];
  mqttClient.publish(mqttTopic.c_str(), msg.c_str());
  delay(100);
}

// ################################################################################
// The IRAM_ATTR attribute places the compiled code Internal RAM (IRAM)
//   instead of slower Flash

void IRAM_ATTR ButtonCallBack()
{
  button_time = millis();
  numberKeyPresses++;
  if (button_time - last_button_time > 150)
  {
    last_button_time = button_time;
    buttonPressed = true;
  }
}
// #########################################################
void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // String clientId = "Nano33-";
    // clientId += String(random(0xffff), HEX);
    bool conn = mqttClient.connect("ESPClient", mqttUser, mqttPassword);
    // delay(1000);

    if (mqttClient.connected())
    {
      Serial.println("connected");
      // ... and resubscribe
      mqttTopic = deviceID + "/lamp";
      Serial.println("mqttTopic..." + mqttTopic);
      mqttClient.subscribe(mqttTopic.c_str());
      Serial.println(" Reconnected subscribed...");
      // delay(1000);
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
// ########### SETUP ####################################################################
// ######################################################################################
void setup()
{
  //  put your setup code here, to run once:
  Serial.println("Starting Setup()");

  Serial.begin(9600);
  // 9600,14,400, 19,200, 38,400, 57,600, and 115,200 baud
  pinMode(LED, OUTPUT);
  pinMode(keyLockPin, INPUT_PULLDOWN);

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
  IPaddress = String(WiFi.localIP());
  Serial.println("IP Address: " + IPaddress);
  long rssi = WiFi.RSSI();

  // ########  Device id settings ###########################################
  deviceID = MACaddress.substring(MACaddress.length() - 5);
  mqttTopic = deviceID + "/lamp";
  mqttClient.subscribe(mqttTopic.c_str());
  Serial.println(" Device ID: " + deviceID);
  Serial.println(" MAC Address: " + deviceID);

  // ######### MQTT SETUP ###########################################################
  // https://github.com/knolleary/pubsubclient/issues/163

  Serial.println("Connecting to MQTT Server...");
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallBack);
  mqttClient.connect("ESPClient", mqttUser, mqttPassword);
  mqttClient.setCallback(mqttCallBack);
  mqttTopic = deviceID + "/lamp";
  mqttClient.subscribe(mqttTopic.c_str());
  msg = " world";
  mqttTopic = deviceID + "/info/Hello/";
  mqttClient.publish(mqttTopic.c_str(), msg.c_str());

  // ############# Configure the ESP32 I/0 pins #############################################

  for (int i = 0; i < sizeof(ButtonPins) / sizeof(int); i++)
  {
    Serial.println("Setting Input Button: " + String(ButtonPins[i]));
    pinMode(ButtonPins[i], INPUT_PULLDOWN);
    attachInterrupt(ButtonPins[i], ButtonCallBack, RISING);
  }

  for (int i = 0; i < sizeof(LampPins) / sizeof(int); i++)
  {
    Serial.println("Setting Lamp Outputs: " + String(LampPins[i]));
    pinMode(LampPins[i], OUTPUT);
    digitalWrite(LampPins[i], HIGH);
  }

  mqttTopic = deviceID + "/info/IPaddress/";
  msg = IPaddress;
  mqttClient.publish(mqttTopic.c_str(), msg.c_str());
  Serial.println("Published IP Address");

  msg = MACaddress;
  mqttTopic = deviceID + "/info/MACaddress/";
  mqttClient.publish(mqttTopic.c_str(), msg.c_str());

  buttonPressed = false;
}
// END OF SETUP

//  ############## LOOP #################################################################
//  #####################################################################################
void loop()
{

  if (!mqttClient.connected())
  {
    reconnect();
  }

  if (buttonPressed)
  {

    // Serial.println("Button was pressed, polling buttons: " + String(numberKeyPresses));
    mqttTopic = deviceID + "/button/";
    if (digitalRead(ButtonPins[0]) == 1)
    {
      mqttTopic += ButtonNames[0];
      ButtonPressCount[0] += 1;
      msg = ButtonPressCount[0];
      mqttClient.publish(mqttTopic.c_str(), msg.c_str());
      // digitalWrite(LampPins[0], LOW);
    }

    if (digitalRead(ButtonPins[1]) == 1)
    {
      mqttTopic += ButtonNames[1];
      ButtonPressCount[1] += 1;
      msg = ButtonPressCount[1];
      mqttClient.publish(mqttTopic.c_str(), msg.c_str());
      // digitalWrite(LampPins[1], LOW);
    }

    if (digitalRead(ButtonPins[2]) == 1)
    {
      mqttTopic += ButtonNames[2];
      ButtonPressCount[2] += 1;
      msg = ButtonPressCount[2];
      mqttClient.publish(mqttTopic.c_str(), msg.c_str());
      // digitalWrite(LampPins[2], LOW);
    }

    if (digitalRead(ButtonPins[3]) == 1)
    {
      mqttTopic += ButtonNames[3];
      ButtonPressCount[3] += 1;
      msg = ButtonPressCount[3];
      mqttClient.publish(mqttTopic.c_str(), msg.c_str());
      // digitalWrite(LampPins[3], LOW);
    }

    if (digitalRead(ButtonPins[4]) == 1)
    {
      mqttTopic += ButtonNames[4];
      ButtonPressCount[4] += 1;
      msg = ButtonPressCount[4];
      mqttClient.publish(mqttTopic.c_str(), msg.c_str());
    }

    if (digitalRead(ButtonPins[5]) == 1)
    {
      mqttTopic += ButtonNames[5];
      ButtonPressCount[5] += 1;
      msg = ButtonPressCount[5];
      mqttClient.publish(mqttTopic.c_str(), msg.c_str());
    }
  }

  buttonPressed = false;
  numberKeyPresses = 0;

  mqttClient.loop();
}
