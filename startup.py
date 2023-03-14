# MQTT Callbacks
# pip install callbacks
#  pip install paho-mqtt
# https://mosquitto.org/download/
# mosquitto-2.0.15-install-windows-x64.exe 
import paho.mqtt.client as mqtt
import time
import Globals
import Car


from queue import Queue
q=Queue()

#broker ="mqtt.eclipseprojects.io" 
#broker ="test.mosquitto.org"
broker ="127.0.0.1" 
# from callbacks import *

from getmac import get_mac_address as gma
mac = gma()
print(mac)
ID = mac[-5:]
print (ID)

def reset(payload):
	print ('got reset command: ' + payload)
	PublishMessage( 'status1', 'resetting')
	time.sleep(5)
	PublishMessage('status2', 'bottom')
	time.sleep(5)
	PublishMessage( 'status3', 'top')
	time.sleep(5)
	PublishMessage('status4', 'bottom2')

def PublishMessage(topic, msg):
	topic = ID + '/' + topic
	result = client.publish(topic, msg)
	# result: [0, 1]
	print (result)
	status = result[0]
	time.sleep(.5)
	if status == 0:
		print("Sent " + msg + " to topic " + topic)
	else:
		print("Failed to send message to topic " + topic)
	#msg_count += 1
	print("Topic: " + topic + "   Message: " + msg)

def on_message(client, userdata, msg):
	#callback when a new message is posted on MQTT server
	#print('Received a new  data ', msg.payload.decode('utf-8'))
	#print("message qos=",msg.qos)
	#print("message retain flag=",msg.retain)
	payload = msg.payload.decode('utf-8')
	topic = msg.topic
	
	print('MQTT send: ' + topic.ljust(25," "), payload.ljust(20," "))

def on_connect(client, userdata, flags, rc):
	if rc == 0:
		print("Connected to MQTT Broker!")
	else:
		print("Failed to connect, return code %d\n", rc)	

def on_connect(client, userdata, flags, rc):
	if rc==0:
		print ("Callback: connected ok")
	else:
		print ("bad connection Return code: ". rc)		

def on_disconnect(client, userdata, flags, rc):
	if rc==0:
		print ("Callback: Disconnected. Code: " + str(rc))
		
client = mqtt.Client("elevator")
client.on_connect = on_connect

client.message_callback_add(ID + '/#', on_message)
client.connect(broker)
client.subscribe(ID +'/#')

print ("starting loop as new thread")
client.loop_start()

while True:
	time.sleep(.2)
	while not q.empty():
		msg = q.get()
		if msg is None:
			continue
		print("received from queue",str(msg.payload.decode("utf-8")))		
		payload = msg.payload.decode('utf-8')
		topic = msg.topic
		payload = msg.payload.decode('utf-8')
		topic = msg.topic

	if topic == ID + '/reset':
		reset(payload)
		print ('got reset command: ' + payload)

	elif topic == ID + '/MoveCar':
		steps = int(payload)
		Car.MoveCar(payload)
		print ('got move command: ' + payload)
		PublishMessage(ID + '/move', 'complete')

	elif topic == ID + '/speed':
		print ('got speed command: ' + payload)

	elif topic == ID + '/TopFloor':
		TopFloor = int(payload)
		print ('got sTopFloor command: ' + str(TopFloor))

	elif topic == ID + '/StepWaitTime':
		StepWaitTime = float(payload)
		print ('got sTopFloor command: ' + float(TopFloor))

	elif topic == ID + '/CurrentStepPosition':
		Globals.CurrentStepPosition = int(payload)
		print ('got sTopFloor command: ' + str(Globals.CurrentStepPosition))

	elif topic == ID + '/StopNow':
		if payload == True:
			Globals.StopNow = True
		else:
			Globals.StopNow = False
		print ('got StopNow command: ' + Globals.StopNow)
	