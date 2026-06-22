from serialread import ReadLine
import builtins
from numpy import *
from numpy.linalg import *
import random

farsideSensorLocations = [[1.37, 0.16], [0.16, 0.16], [0.75, 0.75], [0.75, 1.37]]
nearsideSensorLocations = [[0.16, 2.63], [1.37, 2.63], [0.75, 2.04], [0.75, 1.41]]
numOfDimensions = 2
nSensors = 4
tableWidth = 1.584
tableLength = 3.24

def parseSerialLine(line):
	values = line.rstrip().split()
	if not values or values[0] not in ("A", "B", "C", "D"):
		raise ValueError("expected an A, B, C, or D serial line")

	if values[0] in ("A", "B"):
		if len(values) != 5:
			raise ValueError("expected four sensor times")
		return values[0], [float(value) / 1000000 for value in values[1:]]

	if len(values) < 3 or len(values) % 2 == 0:
		raise ValueError("expected sensor ID and time pairs")

	partial = [[int(values[i]), int(values[i+1])] for i in range(1, len(values), 2)]
	firstSensor = 1 if values[0] == "C" else 5
	if builtins.any(sensor < firstSensor or sensor >= firstSensor + nSensors for sensor, _ in partial):
		raise ValueError("sensor ID does not match side")
	return values[0], partial

def calculateSerialLine(line, rng=None):
	lineType, sensorData = parseSerialLine(line)
	if lineType in ("A", "B"):
		pos = nearside(sensorData) if lineType == "A" else farside(sensorData)
		return {
			"side": "near" if lineType == "A" else "far",
			"x": float(pos[0]),
			"y": float(pos[1]),
			"valid": builtins.bool(pos[0] > 0.0 and pos[0] < tableWidth and pos[1] > 0.0 and pos[1] < tableLength),
		}

	rng = rng or random
	closest = builtins.min(sensorData, key=lambda sensor: sensor[1])[0]
	locations = nearsideSensorLocations if lineType == "C" else farsideSensorLocations
	firstSensor = 1 if lineType == "C" else 5
	pos = locations[closest - firstSensor]
	return {
		"side": "near" if lineType == "C" else "far",
		"x": pos[0] + rng.uniform(-0.15, 0.15),
		"y": pos[1] + rng.uniform(-0.15, 0.15),
		"valid": False,
	}

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
			b[iRow,0] = v*(t[i])*(v*v*(t[j])**2-(p[:,j].T*p[:,j]).item()) + \
			(v*(t[i])-v*(t[j]))*(p[:,c].T*p[:,c]).item() + \
			v*(t[j])*((p[:,i].T*p[:,i]).item()-v*v*(t[i])**2)
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
			b[iRow,0] = v*(t[i])*(v*v*(t[j])**2-(p[:,j].T*p[:,j]).item()) + \
			(v*(t[i])-v*(t[j]))*(p[:,c].T*p[:,c]).item() + \
			v*(t[j])*((p[:,i].T*p[:,i]).item()-v*v*(t[i])**2)
			rankA = matrix_rank(A)
			if rankA >= numOfDimensions :
				break
			iRow += 1
		if rankA >= numOfDimensions:
			break

	calculatedLocation = asarray( lstsq(A,b)[0] )[:,0]

	return calculatedLocation

def main():
	import argparse
	import serial
	from pythonosc import udp_client

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
		values = rcv.split()
		print(values)

		if values[0] in ("A", "B", "C", "D"):
			result = calculateSerialLine(rcv)
			print([result["x"], result["y"]])
			address = "/nearside/location" if result["side"] == "near" else "/farside/location"
			message = [result["x"], result["y"], int(result["valid"])]
			client.send_message(address, message)
			client.send_message("/location", message)

if __name__ == "__main__":
	main()
