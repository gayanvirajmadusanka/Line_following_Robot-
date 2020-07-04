import serial
import matplotlib.pyplot as plt
from drawnow import *
import time

values = []
clr = 'blue'

plt.ion()
cnt = 0

serial = serial.Serial('COM18', 9600)

def plotValues():
    plt.title('Distance to the wall')
    plt.grid(True)
    plt.ylabel('Values')
    plt.plot(values, clr, label='values')
    
for i in range(0, 100):
    values.append(0)

while True:
    try:

        x = serial.readline().decode('utf-8')
        print(x)
        if(x=='Red'):
        	clr='red'
        	print('Red')
        elif(x=='Green'):
        	clr='green'
        	print("Green")
        elif(x=='White'):
        	clr='brown'
        elif(x=='Black'):
        	clr='black'
        else:
        	vals = int(x)
        	if(vals>50):
        		vals=50
        	#print(vals)
        	values.append(vals)
        	values.pop(0)
        	drawnow(plotValues)	
    except:
        print("\n")
