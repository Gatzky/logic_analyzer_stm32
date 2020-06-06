import serial                                       # serial is for serial communication
import matplotlib.pyplot as plot                    # matplotlib is for draw plot

serialPort = serial.Serial('COM9', 115200)          # open a serial port, COM9, baudrate 115200 same as in stm32


def get_buffer_length():                            # function for get buffer length from uart
    length = 0
    while length < 2:
        temp = serialPort.read()
        temp = int(hex(ord(temp)), 16)              # line for change data from string to int
        length = temp
    return length


def collect_data(buffer, length):                   # function for get sample from uart
    for x in range(0, length):
        temp = serialPort.read()
        temp = int(hex(ord(temp)), 16)              # line for change data from string to int
        buffer[x] = temp
    return buffer


while 1:
    buffer_length = get_buffer_length()             # get buffer length from uart

    result = [0] * buffer_length                    # initialize int table with size of 'buffer_length'
    result = collect_data(result, buffer_length)    # get sample from uart

    plot.plot(result)
    plot.xlabel('samples')
    plot.ylabel('logic state')
    plot.show()
