#using serial communication using serial0
import serial
import time

global ser

def protocol_init(baudrate, port):
    global ser
    ser = serial.Serial(port, baudrate, timeout=1)

def protocol_send(data, length):
    global ser
    if length is None:
        ser.write(data)
    return time.time()

def protocol_receive(data, length):
    global ser
    data = b''
    
    while len(data) < length:
        data += ser.read()
        
    ser.reset_input_buffer()
    return data

def protocol_close():
    global ser
    
    ser.close()


   










    