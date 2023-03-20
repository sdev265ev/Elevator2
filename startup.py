# MQTT Callbacks
# pip install callbacks
#  pip install paho-mqtt
# https://mosquitto.org/download/
# mosquitto-2.0.15-install-windows-x64.exe 

#broker ="mqtt.eclipseprojects.io" 
broker ="test.mosquitto.org"
#broker ="192.168.2.158" 


import paho.mqtt.client as mqtt
import time
from callbacks import *

def PublishMessage(topic, msg):
	topic = 'elevEV/' + topic
	result = client.publish(topic, msg)
	# result: [0, 1]
	print (result)
	status = result[0]
	if status == 0:
		print(f"Send '{msg}' to topic '{topic}'")
	else:
		print(f"Failed to send message to topic {topic}")
	#msg_count += 1
	print("Topic: " + topic + "   Message: " + msg)

def on_message_elevator(client, userdata, msg):
	#callback when a new message is posted on MQTT server
	#print('Received a new  data ', msg.payload.decode('utf-8'))
	#print("message qos=",msg.qos)
	#print("message retain flag=",msg.retain)
	payload = msg.payload.decode('utf-8')
	topic = msg.topic
	print('MQTT sub: ' + topic.ljust(25," "), payload.ljust(20," "))

	# Check for topics and commands to control the elevator
	#system, ID, topic = topic.split('/')
	#topic = topic.lower()

def on_connect(client, userdata, flags, rc):
	if rc == 0:
		print("Connected to MQTT Broker!")
	else:
		print("Failed to connect, return code %d\n", rc)	
		
client = mqtt.Client("elevator")
client.on_connect = on_connect

#client.message_callback_add('elevEV/#', on_message_elevator)
client.connect(broker)
client.subscribe('elevEV/#')

print ("starting loop as new thread")
client.loop_start()

while True:
	#PublishMessage('holdcardoor', 'holdaaaa')
	client.publish(lib.SYSTEMID() + '/' + 'holdCarDoor', 'holdaa')
	time.sleep(3)




def on_message_house(client, userdata, msg):
	#print('Received a new  data ', msg.payload.decode('utf-8'))
	payload = msg.payload.decode('utf-8')
	topic = msg.topic
	print(topic, payload)
	#print("message qos=",msg.qos)
	#print("message retain flag=",msg.retain)
	
def on_log(client, userdata, level, buf):
	print ("log: " + buf)

def on_connect(client, userdata, flags, rc):
	if rc==0:
		print ("Callback: connected ok")
	else:
		print ("bad connection Return code: ". rc)

def on_disconnect(client, userdata, flags, rc):
	if rc==0:
		print ("Callback: Disconnected. Code: " + str(rc))
