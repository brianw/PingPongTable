from serialread import ReadLine
import serial
from numpy import *
from numpy.linalg import *
import argparse
import random
import time
from pythonosc import osc_bundle_builder
from pythonosc import osc_message_builder
from pythonosc import udp_client

farsideSensorLocations = [[1.37, 0.16], [0.16, 0.16], [0.75, 0.75], [0.75, 1.37]]
nearsideSensorLocations = [[0.16, 2.63], [1.37, 2.63], [0.75, 2.04], [0.75, 1.41]]
numOfDimensions = 2
nSensors = 4

def farside(sensorTimes):
	#speed of sound in medium
	#v = 1128
	v = 322
	#v = 613
	

	p = matrix( farsideSensorLocations ).T

	c = argmin(sensorTimes)
	cTime = sensorTimes[c]

	#sensors delta time relative to sensor c
	t = sensorDeltaTimes = [ sensorTime - cTime for sensorTime in sensorTimes ] 

	ijs = list(range(nSensors))
	del ijs[c]

	A = zeros([nSensors-1,numOfDimensions])
	b = zeros([nSensors-1,1])
	iRow = 0
	rankA = 0
	for i in ijs:
		for j in ijs:
			A[iRow,:] = 2*( v*(t[j])*(p[:,i]-p[:,c]).T - v*(t[i])*(p[:,j]-p[:,c]).T )
			b[iRow,0] = v*(t[i])*(v*v*(t[j])**2-p[:,j].T*p[:,j]) + \
			(v*(t[i])-v*(t[j]))*p[:,c].T*p[:,c] + \
			v*(t[j])*(p[:,i].T*p[:,i]-v*v*(t[i])**2)
			rankA = matrix_rank(A)
			if rankA >= numOfDimensions :
				break
			iRow += 1
		if rankA >= numOfDimensions:
			break

	calculatedLocation = asarray( lstsq(A,b)[0] )[:,0]

	return calculatedLocation

def nearside(sensorTimes):
	#speed of sound in medium
	#v = 322
	v = 470

	p = matrix( nearsideSensorLocations ).T

	c = argmin(sensorTimes)
	cTime = sensorTimes[c]

	#sensors delta time relative to sensor c
	t = sensorDeltaTimes = [ sensorTime - cTime for sensorTime in sensorTimes ] 

	ijs = list(range(nSensors))
	del ijs[c]

	A = zeros([nSensors-1,numOfDimensions])
	b = zeros([nSensors-1,1])
	iRow = 0
	rankA = 0
	for i in ijs:
		for j in ijs:
			A[iRow,:] = 2*( v*(t[j])*(p[:,i]-p[:,c]).T - v*(t[i])*(p[:,j]-p[:,c]).T )
			b[iRow,0] = v*(t[i])*(v*v*(t[j])**2-p[:,j].T*p[:,j]) + \
			(v*(t[i])-v*(t[j]))*p[:,c].T*p[:,c] + \
			v*(t[j])*(p[:,i].T*p[:,i]-v*v*(t[i])**2)
			rankA = matrix_rank(A)
			if rankA >= numOfDimensions :
				break
			iRow += 1
		if rankA >= numOfDimensions:
			break

	calculatedLocation = asarray( lstsq(A,b)[0] )[:,0]

	return calculatedLocation

#Sensors = serial.Serial("/dev/serial/by-id/usb-Teensyduino_USB_Serial_3818950-if00", baudrate=38400, bytesize=8, parity='N', stopbits=1, timeout=None)
Sensors = serial.Serial("/dev/TEENSY5", baudrate=38400, bytesize=8, parity='N', stopbits=1, timeout=None)
reader = ReadLine(Sensors)
parser = argparse.ArgumentParser()
parser.add_argument("--ip", default="127.0.0.1", help="The ip of the OSC server")
parser.add_argument("--port", type=int, default=6666, help="The port the OSC server is listening on")
args = parser.parse_args()
client = udp_client.SimpleUDPClient(args.ip, args.port)
client2 = udp_client.SimpleUDPClient(args.ip, 7778)
client2.send_message("/video", 2)

print("Started")

while True:
	rcv = reader.readline().decode('ascii').rstrip()
	values = rcv.split(' ')

	print(values)
	if (values[0] == "A"):
		sensorTimes = [float(values[1])/1000000, int(values[2])/1000000, int(values[3])/1000000, int(values[4])/1000000]
		pos = nearside(sensorTimes)
		print (pos)
		if pos[0] > 0.0 and pos[0] < 1.584 and pos[1] > 0 and pos[1] < 3.24:
			client.send_message("/nearside/location", [pos[0], pos[1], 1])
			client.send_message("/location", [pos[0], pos[1], 1])
		else:
			clip(pos[0], 0, 1.584)
			clip(pos[1], 0, 3.24)
			client.send_message("/nearside/location", [pos[0], pos[1], 0])
			client.send_message("/location", [pos[0], pos[1], 0])

	if (values[0] == "B"):
		sensorTimes = [float(values[1])/1000000, int(values[2])/1000000, int(values[3])/1000000, int(values[4])/1000000]
		pos = farside(sensorTimes)
		print (pos)
		if pos[0] > 0.0 and pos[0] < 1.584 and pos[1] > 0 and pos[1] < 3.24:
			client.send_message("/farside/location", [pos[0], pos[1], 1])
			client.send_message("/location", [pos[0], pos[1], 1])
		else:
			clip(pos[0], 0, 1.584)
			clip(pos[1], 0, 3.24)
			client.send_message("/farside/location", [pos[0], pos[1], 0])
			client.send_message("/location", [pos[0], pos[1], 0])

	if (values[0] == "C"):
		partial = []
		for i in range(1, len(values) - 1, 2):
			partial.append([int(values[i]), int(values[i+1])])
		closest = min(partial, key=lambda x: x[1])[0]
		pos = [nearsideSensorLocations[closest-1][0] + random.uniform(-0.15,0.15), nearsideSensorLocations[closest-1][1] + random.uniform(-0.15,0.15)]
		client.send_message("/nearside/location", [pos[0], pos[1], 0])
		client.send_message("/location", [pos[0], pos[1], 0])

	if (values[0] == "D"):
		partial = []
		for i in range(1, len(values) - 1, 2):
			partial.append([int(values[i]), int(values[i+1])])
		closest = min(partial, key=lambda x: x[1])[0]
		pos = [farsideSensorLocations[closest-5][0] + random.uniform(-0.15,0.15), farsideSensorLocations[closest-5][1] + random.uniform(-0.15,0.15)]
		client.send_message("/farside/location", [pos[0], pos[1], 0])
		client.send_message("/location", [pos[0], pos[1], 0])

