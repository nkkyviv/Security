#using serial communication using serial0
import serial
import time

global ser

def protocol_init(baudrate, port):
    global ser
    ser = serial.Serial(port, baudrate, timeout=1)

def protocol_send(data):
    global ser
    length=ser.write(data)
    return length
  
def protocol_receive(length):
    global ser
    data = b''
    
    while len(data) < length:
        data += ser.read()
        
    return data

def protocol_close():
    global ser
    
    ser.close()


   










    