import protocol
import time

def establish_session(port):
    protocol.protocol_init(115200, port)
    
    request = b"1"

    protocol.protocol_send(request)
    
    response_size = 1
    
    received_data = protocol.protocol_receive(response_size)
    print(received_data)
    return received_data
    
def get_temperature():
    request = b"2"

    protocol.protocol_send(request)
    
    print("sent_data:", request)
    response_size = 5
    
    received_data = protocol.protocol_receive(response_size)
    print(received_data)
    return received_data
    
def toggle_led():
    request = b"3"
 
    protocol.protocol_send(request)
   
    response_size = 1
    
    received_data = protocol.protocol_receive(response_size)

    print("received_data:", received_data)
    print(received_data)
    return received_data

def close_session():
    request = b"4"

    protocol.protocol_send(request)
 
    response_size = 1
    
    received_data = protocol.protocol_receive(response_size)
    
    protocol.protocol_close()
    print(received_data)
    return received_data

 
    