#using serial communication using serial0
import serial
import time

global ser

def protocol_init(baudrate, port):
    global ser
    ser = serial.Serial(port, baudrate, timeout=1)

def protocol_send(data, length=None):
    global ser
    if length is None:
        length = len(data)
    ser.write(data)
    return time.time()

def protocol_receive(data, length):
    global ser
    data = bytearray()
    
    while len(data) < length:
        data += ser.read(length - len(data))
    return data

# def protocol_receive(ser, data, length):
#     data_list = []
    
#     while len(data_list) < length:
#         data_list.append(ser.read())
#     return data_list


   










    