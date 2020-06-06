import serial                                       # serial is for serial communication
import matplotlib.pyplot as plot                    # matplotlib is for draw plot

serialPort = serial.Serial('COM9', 115200)          # open a serial port, COM9, baudrate 115200 same as in stm32


def get_buffer_length():                            # function for get buffer length from uart
    length = 0
    while length < 2:
        temp = serialPort.read()
        temp = int(hex(ord(temp)), 16)              # line for change data from string to int
        length = temp
    length = 250
    return length


def collect_data(buffer, length):                   # function for get sample from uart
    for x in range(0, length):
        temp = serialPort.read()
        temp = int(hex(ord(temp)), 16)              # line for change data from string to int
        buffer[x] = temp
    return buffer


while 1:
    buffer_length = get_buffer_length()                     # get buffer length from uart

    result_raw = [0] * buffer_length                        # initialize int table with size of 'buffer_length'
    result_raw = collect_data(result_raw, buffer_length)    # get sample from uart
    print("result_raw", result_raw)
    result_value = result_raw[::2]
    print("result_value", result_value)
    result_quanity = result_raw[1::2]
    print("result_quanity", result_quanity)

    result = []

    for i in range(0, len(result_quanity)):
        temp_value = result_value[i]
        for j in range(0, result_quanity[i]):
            result.append(temp_value)

    print("result", result)
    plot.plot(result)
    plot.xlabel('samples')
    plot.ylabel('logic state')
    plot.show()
