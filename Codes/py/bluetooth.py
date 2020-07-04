import serial
import time

ser = serial.Serial('COM5', 9600, timeout=1)
file = open("2nd_method_blue.txt","w")

while 1:
    reading = ser.readline().decode('utf-8')
    print(reading)
    file.write(reading)
   
file.close()
