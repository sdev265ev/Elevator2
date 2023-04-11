// DoorController.cpp

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
string command = "stop";

int LED = 2;
String LedState = "off";
int switchPinOpen = 21;
int switchPinClosed = 21;
int bridgeA = 21;
int bridgeB = 21;
bool LimitClose = false;
bool LimitOpen = false;

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
	for (int i = 0; i < length; i++)
	{
		msg += (char)message[i];
	}
	Serial.println("Command received: ", msg);
	command = msg;
}

void IRAM_ATTR OpenCallBack()
{
	button_time = millis();
	numberKeyPresses++;
	if (button_time - last_button_time > 150)
	{
		last_button_time = button_time;
		LimitOpen = true;
	}
}
void IRAM_ATTR CloseCallBack()
{
	button_time = millis();
	numberKeyPresses++;
	if (button_time - last_button_time > 150)
	{
		last_button_time = button_time;
		LimitClose = true;
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

	// Configure I/O pins
	pinMode(LED, OUTPUT);
	pinMode(switchPinUP, INPUT_PULLDOWN);
	pinMode(switchPinDW, INPUT_PULLDOWN);
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
	IPaddress = String(WiFi.localIP());
	Serial.println("IP Address: " + IPaddress);
	long rssi = WiFi.RSSI();

	// ########  Device id settings ###########################################
	deviceID = MACaddress.substring(MACaddress.length() - 5);
	mqttTopic = deviceID + "/door";
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
	mqttTopic = deviceID + "/door";
	mqttClient.subscribe(mqttTopic.c_str());
	msg = " world";
	mqttTopic = deviceID + "/info/Hello/";
	mqttClient.publish(mqttTopic.c_str(), msg.c_str());

	// ############# Configure I/0 pins #############################################

	Serial.println("Setting limit switch callbacks...");
	attachInterrupt(switchPinOpen, OpenCallBack, RISING);
	attachInterrupt(switchPinClosed, CloseCallBack, RISING);
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
		reconnect();
	}

	if (command = 'open' && !LimitOpen)
	{
		digitalWrite(bridgeA, HIGH);
		digitalWrite(bridgeB, LOW);
		msg = 'open';
		mqttTopic = 'status';
		mqttClient.publish(mqttTopic.c_str(), msg.c_str());
	}
	if (command = 'close' && !LimitClose)
	{
		digitalWrite(bridgeA, LOW);
		digitalWrite(bridgeB, HIGH);
		msg = 'closed';
		mqttTopic = 'status';
		mqttClient.publish(mqttTopic.c_str(), msg.c_str());
	}
	if (command = 'stop')
	{
		if (!LimitClose && !LimitOpen)
		{
			digitalWrite(bridgeA, HIGH);
			digitalWrite(bridgeB, HIGH);
			msg = 'Schrödinger';
			mqttTopic = 'status';
			mqttClient.publish(mqttTopic.c_str(), msg.c_str());
		}
	}
	msg = "";
	mqttClient.loop();
}

/*
#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Message arrived on topic: ");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(500);
  Serial.print("Message arrived on topic: ");
}
*/
