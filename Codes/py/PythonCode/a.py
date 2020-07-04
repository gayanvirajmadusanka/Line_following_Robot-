import serial
#import matplotlib.pyplot as plt
#from drawnow import *

values = []

#plt.ion()
cnt = 0

print('Start com')
serialArduino = serial.Serial('COM5', 9600)
print('Serial open')
#def plotValues():
#    plt.title('Serial values from Arduino')
#    plt.grid(True)
#    plt.ylabel('Values')
#    plt.plot(values, 'rx-', label='values')
#    plt.legend(loc= 'upper right')

#values = range(0,100)
for i in range(0, 100):
    values.append(0)

while True:
    #try:
            
        #for i in range(0,200):
     valueRead = serialArduino.readline()#.decode('utf-8')
           #print(valueRead.decode('utf-8'))
            #print(valueRead)
     valueInInt = int(valueRead)
     print(valueInInt)
            #values.append(valueInInt)
            #values.pop(0)
            #drawnow(plotValues)
    #except:
    #    print("\n")
