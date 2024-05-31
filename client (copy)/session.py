import time
import protocol
from mbedtls import pk, hmac, hashlib, cipher

RSA_SIZE = 256
EXPONENT = 65537


SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"

hmac_key = hashlib.sha256()
hmac_key.update(SECRET_KEY)
hmac_key = hmac_key.digest()
hmac_key = hmac.new(hmac_key, digestmod="SHA256")


def send_data(buf: bytes):
     hmac_key.update(buf)
     buf += hmac_key.digest()
     protocol.protocol_send(buf, len(buf))
     print("key hashed:", buf.hex())
     print("Sent")
     
def recieve_data(size: int)->bytes:
     buffer = protocol.protocol_receive(size + hmac_key.digest_size)
     hmac_key.update(buffer[0:size])
     if buffer[size:size + hmac_key.digest_size] != hmac_key.digest():
          print("Hash Error")
     return buffer[0:size]
     

def session_init(port):
     protocol.protocol_init(115200, port)
     client_temp = pk.RSA()
     client_temp.generate(RSA_SIZE*8, EXPONENT)
    
     temp_public_key = client_temp.export_public_key()
    
     send_data(temp_public_key)
     
     buffer = recieve_data(2*RSA_SIZE)
     
     server_key_pub = client_temp.decrypt(buffer[0:RSA_SIZE])
     server_key_pub += client_temp.decrypt(buffer[RSA_SIZE:2*RSA_SIZE])
     server_key= pk.RSA().from_DER(server_key_pub)
     
     del client_temp
     
     client_key = pk.RSA()
     client_key.generate(RSA_SIZE * 8, EXPONENT)
     
     buffer = client_key.export_public_key() + client_key.sign(SECRET_KEY, "SHA256")
     buffer = server_key.encrypt(buffer[0: 184]) + server_key.encrypt(buffer[184:368]) + server_key.encrypt(buffer[368:550])
     send_data(buffer)
     
     buffer = recieve_data(RSA_SIZE)

                    