import protocol
from mbedtls import pk, hmac, hashlib, cipher

RSA_SIZE = 256
EXPONENT = 65537
SESSION_ID: bytes
SERVER_RSA: pk.RSA
CLIENT_RSA: pk.RSA
HMAC_CS: hmac.Hmac
AES_CS: cipher.AEADCipher
SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"
     

def send(buf: bytes) -> bool:
     HMAC_CS.update(buf)
     buf += HMAC_CS.digest()
     protocol.send(buf)
     return True
     
def recieve(size: int) -> bytes:
     buffer = protocol.receive(size + HMAC_CS.digest_size)
     HMAC_CS.update(buffer[0:size])
     data = b''
     if buffer[size:size + HMAC_CS.digest_size] == HMAC_CS.digest():
          data = buffer[0:size]
     return data

def init(port) -> bool:
     status = False
     if True == protocol.init(port):
          global HMAC_CS
          HMAC_CS = hashlib.sha256()
          HMAC_CS.update(SECRET_KEY)
          HMAC_CS = HMAC_CS.digest()
          HMAC_CS = hmac.new(HMAC_CS, digestmod="SHA256")
          
          global CLIENT_RSA
          CLIENT_RSA = pk.RSA()
          CLIENT_RSA.generate(RSA_SIZE * 8, EXPONENT)
     
          if True == send(CLIENT_RSA.export_public_key()):
               buffer = recieve(2 * RSA_SIZE)
               if len(buffer) > 0:
                    global SERVER_RSA
                    server_key_pub = CLIENT_RSA.decrypt(buffer[0:RSA_SIZE])
                    server_key_pub += CLIENT_RSA.decrypt(buffer[RSA_SIZE:2*RSA_SIZE])
                    SERVER_RSA = pk.RSA().from_DER(server_key_pub)
               
                    del CLIENT_RSA
                    CLIENT_RSA = pk.RSA()
                    CLIENT_RSA.generate(RSA_SIZE * 8, EXPONENT)
               
                    buffer = CLIENT_RSA.export_public_key() + CLIENT_RSA.sign(SECRET_KEY, "SHA256")
                    buffer = SERVER_RSA.encrypt(buffer[0: 184]) + SERVER_RSA.encrypt(buffer[184:368]) + SERVER_RSA.encrypt(buffer[368:550])
                    if True == send(buffer):             
                         buffer = recieve(RSA_SIZE)
                         if len(buffer) > 0:
                             if b"OKAY" == CLIENT_RSA.decrypt():
                                  status = True
          
     return status

def establish() -> bool:
     global CLIENT_RSA
     global SERVER_RSA
     buffer = CLIENT_RSA.sign(SECRET_KEY, "SHA256")
     buffer = SERVER_RSA.encrypt(buffer[0:RSA_SIZE//2]) + SERVER_RSA.encrypt(buffer[RSA_SIZE//2:RSA_SIZE])
     status = False
     if True == send(buffer):
          buffer = recieve(RSA_SIZE)
          if len(buffer) > 0:
               buffer = CLIENT_RSA.decrypt(buffer)
               
               global AES_CS
               global SESSION_ID
               SESSION_ID = buffer[0:8]
               AES_CS = cipher.AES.new(buffer[24:56], cipher.MODE_CBC, buffer[8:24])
               status = True
     return status

def request(req: bytes) -> bytes:
     pass