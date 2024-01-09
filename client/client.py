import protocol
import time
import struct

def establish_session(port):
    protocol.protocol_init(115200, port)
    
    request = b"establish_session"
    
    length = len(request)
    
    now =protocol.protocol_send(request, length)
    time.sleep(1)
    
    response_size = 4
    response_data = None
    
    received_data = protocol.protocol_receive(response_data,response_size)
    
    return received_data
    


def get_temperature(port):
    request = b"get_temp"
    
    length = len(request)
    
    now = protocol.protocol_send(request, length)
    
    print("sent_data:", request)
    time.sleep(1)
    
    response_size = 4
    response_data = bytearray()
    
    received_data = protocol.protocol_receive(response_data,response_size)
    
    print("received temp:", received_data) 
    
    temperature = struct.unpack('f', received_data)[0]
    
    
    return temperature
    

def close_session(port):
    request = b"close_session"
    
    length = len(request)
    
    now = protocol.protocol_send(request, length)
    
    time.sleep(1)
    
    response_size = 4
    response_data = None
    
    received_data = protocol.protocol_receive(response_data,response_size)
    
    
    return received_data
 
    