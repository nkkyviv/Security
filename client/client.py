import protocol
import time

def establish_session(port):
    protocol.protocol_init(115200, port)
    
    request = b"1"
    
    length = len(request)
    print(length)
    
    now =protocol.protocol_send(request, length)
    time.sleep(1)
    
    response_size = 2
    response_data = None
    
    received_data = protocol.protocol_receive(response_data,response_size)
    print(received_data)
    return received_data
    


def get_temperature(port):
    request = b"2"
    
    length = len(request)
    print(length)
    
    now = protocol.protocol_send(request, length)
    
    print("sent_data:", request)
    time.sleep(1)
    
    response_size = 4
    response_data = b''
    
    received_data = protocol.protocol_receive(response_data,response_size)

    return received_data
    
def toggle_led(port):
    request = b"3"
    length = len(request)
    print(length)

    protocol.protocol_send(request, length)
    time.sleep(1)
    
    response_size = 1
    response_data = b''
    
    received_data = protocol.protocol_receive(response_data,response_size)

    print("received_data:", received_data)
    
    return received_data

def close_session(port):
    request = b"4"
    
    length = len(request)
    
    now = protocol.protocol_send(request, length)
    
    time.sleep(1)
    
    response_size = 2
    response_data = None
    
    received_data = protocol.protocol_receive(response_data,response_size)
    
    protocol.protocol_close()
    
    return received_data

 
    