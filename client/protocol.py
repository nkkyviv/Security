import serial

SERIAL_BAUDRATE = 115200

class Protocol:
    ser = None
    def __init__(self,port, baudrate=SERIAL_BAUDRATE):
        Protocol.ser = serial.Serial(port, baudrate)
        print("Connected to " + port)

    def send_data(buf: bytes): 
        return Protocol.ser.write(buf)

    def receive_data(size: int) -> bytes:
        return Protocol.ser.read(size)
        
    def close(self):
          Protocol.ser.close()