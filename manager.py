# MQTT Callbacks
# pip install callbacks
# pip install paho-mqtt
# https://mosquitto.org/download/
# mosquitto-2.0.15-install-windows-x64.exe 
import paho.mqtt.client as mqtt
import time

# from callbacks import *

from queue import Queue
q=Queue()

topic =""

#broker ="mqtt.eclipseprojects.io" 
#broker ="test.mosquitto.org"
broker ="127.0.0.1" 

ID = '78:fdout'

def PublishMessage(topic, msg):
	topic = ID + 'out/'   + topic
	result = client.publish(topic, msg)
	# result: [0, 1]
	print (result)
	status = result[0]
	if status == 0:
		1==1
		# print("Sent " + msg + " to topic " + topic)
	else:
		print("Failed to send message to topic " + topic)
		print("Topic: " + topic + "   Message: " + msg)

def on_message(client, userdata, msg):
	#callback when a new message is posted at MQTT server
	#print("message qos=",msg.qos)
	#print("message retain flag=",msg.retain)
	#PublishMessage('event', 'message retain flag: ' + str(msg.retain))
	#payload = msg.payload.decode('utf-8')
	#topic = msg.topic
	q.put(msg)

def on_connect(client, userdata, flags, rc):
	if rc == 0:
		print("Connected to MQTT Broker!")
		#PublishMessage('event', 'MQTT connected')
	else:
		print("Failed to connect, return code %d\n", rc)	

def on_disconnect(client, userdata, flags, rc):
	if rc==0: print ("Callback: Disconnected. Code: " + str(rc))
		
client = mqtt.Client("elevator")
client.on_connect = on_connect

client.message_callback_add(ID + '/#', on_message)
client.connect(broker)
client.subscribe(ID +'/#')

q.empty		# Clear the queue before loop
print ("starting loop as new thread")

client.loop_start()
while True:
	#time.sleep(.1)
	if not q.empty():
		msg = q.get()
		if msg is None:
			continue

		payload = msg.payload.decode('utf-8')
		topic = msg.topic
		print ( 'Topic: ', topic, ' payload: ' , payload )
	