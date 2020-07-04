import serial
import time
from itertools import count
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

ser = serial.Serial(port='COM5',baudrate=9600)
plt.style.use('fivethirtyeight')
file = open("sample.txt","w")

x_values = []
y_values = []

index = count()


def animate(i):
        value = ser.readline().decode('utf-8')
        x_values.append(next(index))
        y_values.append(int(value))

        file.write(value)

        plt.cla()
        plt.plot(x_values, y_values)
        plt.xlabel('Time')
        plt.ylabel('Values')
        plt.title('Graph of Values vs. Time')
        time.sleep(0.001)


ani = FuncAnimation(plt.gcf(), animate, 1000)


plt.tight_layout()
plt.show()
file.close()
