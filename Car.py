# pip install callbacks
# pip install paho-mqtt
# https://mosquitto.org/download/
# mosquitto-2.0.15-install-windows-x64.exe 
import paho.mqtt.client as mqtt
import time
import Globals
import StepperDriver as sd
# from callbacks import *

#broker ="mqtt.eclipseprojects.io" 
#broker ="test.mosquitto.org"
broker ="10.81.104.102" 

import queue
q = queue.Queue()

topic =""
stepsMax = 0 

from getmac import get_mac_address as gma
mac = gma()
print(mac)
ID = mac[-5:] 
print (ID)
def PublishMessage(topic, msg):
	topic = ID + 'Car/'   + topic
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
		PublishMessage('event', 'MQTT connected')
	else:
		print("Failed to connect, return code %d\n", rc)	

def on_disconnect(client, userdata, flags, rc):
	if rc==0: print ("Callback: Disconnected. Code: " + str(rc))
		
client = mqtt.Client("elevator")
client.on_connect = on_connect

client.message_callback_add(ID + '/#', on_message)
client.connect(broker)
client.subscribe(ID +'/#')

# Initial Start Default Settings
Globals.StepWaitTime = .0015
Globals.StopNow = False
Globals.CarHeight = 0

PublishMessage('StepWaitTime', str(Globals.StepWaitTime))
PublishMessage('CarHeight', str(Globals.CarHeight))
PublishMessage('StopNow', str(Globals.StopNow))

# q.empty		# Clear the queue before loop

sd.SetUpGPIO()

print ("starting loop as new thread")

client.loop_start()
while True:
	#time.sleep(.1)
	if not q.empty():
		PublishMessage('Queue Size: ', str(q.qsize()))
		# print ('queue size: ', q.qsize())
		msg = q.get()
		#print ('q max: ', q.maxsize)
		if msg is None:
			continue
		print("received from queue ",str(msg.payload.decode("utf-8")))		
		payload = msg.payload.decode('utf-8')
		topic = msg.topic
		print (topic)
		print(ID + '/MoveCar')
		if topic == ID + '/MoveCar':
			steps = sd.MoveMotor(int(payload))
			PublishMessage('StepsMoved', str(steps))
			
		elif topic == (ID + '/StepWaitTime'):
			Globals.StepWaitTime = float(payload)
			PublishMessage('StepWaitTime', str(Globals.StepWaitTime))
			PublishMessage('event', 'Changed Lift Speed: ' + str(Globals.StepWaitTime))

		elif topic == (ID + '/StopNow'):
			if payload == 'True':
				Globals.StopNow = True
			else:
				Globals.StopNow = False
				
