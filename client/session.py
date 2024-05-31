import protocol
from mbedtls import pk, hmac, hashlib, cipher

class Session:
    RSA_SIZE = 256
    EXPONENT = 65537
    SESSION_ID: bytes
    SERVER_RSA: pk.RSA
    CLIENT_RSA: pk.RSA
    HMAC_CS: hmac.Hmac
    AES_CS: cipher.AEADCipher
    SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"
    state = None

    def __init__(self, port):
        self.port = port
        self.HMAC_CS = hashlib.sha256()
        self.HMAC_CS.update(self.SECRET_KEY)
        self.HMAC_CS = self.HMAC_CS.digest()
        self.HMAC_CS = hmac.new(self.HMAC_CS, digestmod="SHA256")
        self.comm = protocol.Protocol(self.port)
        
        self.CLIENT_RSA = pk.RSA()
        self.CLIENT_RSA.generate(self.RSA_SIZE * 8, self.EXPONENT)
        self.exchange_data()
        Session.state = port

    @staticmethod
    def establish():
        Session.state
        if Session.state !=None:
             Session.state = True
        else:
             Session.state = False
        return Session.state

    def send(self, buf: bytes) -> bool:
        self.HMAC_CS.update(buf)
        buf += self.HMAC_CS.digest()
        return self.comm.send_data(buf)
          
    def receive(self, size: int) -> bytes:
        buffer = self.comm.receive_data(size + self.HMAC_CS.digest_size)
        self.HMAC_CS.update(buffer[:size])
        data = b''
        if buffer[size:size + self.HMAC_CS.digest_size] == self.HMAC_CS.digest():
            data = buffer[:size]
        return data

    def exchange_data(self):
        status = False
        self.send(self.CLIENT_RSA.export_public_key())
        buffer = self.receive(2 * self.RSA_SIZE)
        if len(buffer) > 0:
            server_key_pub = self.CLIENT_RSA.decrypt(buffer[:self.RSA_SIZE])
            server_key_pub += self.CLIENT_RSA.decrypt(buffer[self.RSA_SIZE:2*self.RSA_SIZE])
            self.SERVER_RSA = pk.RSA().from_DER(server_key_pub)
                
            del self.CLIENT_RSA
            self.CLIENT_RSA = pk.RSA()
            self.CLIENT_RSA.generate(self.RSA_SIZE * 8, self.EXPONENT)
                
            buffer = self.CLIENT_RSA.export_public_key() + self.CLIENT_RSA.sign(self.SECRET_KEY, "SHA256")
            buffer = self.SERVER_RSA.encrypt(buffer[:184]) + self.SERVER_RSA.encrypt(buffer[184:368]) + self.SERVER_RSA.encrypt(buffer[368:550])
            if self.send(buffer):
                buffer = self.receive(self.RSA_SIZE)
                if len(buffer) > 0:
                    if b"OKAY" == self.CLIENT_RSA.decrypt(buffer):
                        status = True
                        
                        buffer = self.CLIENT_RSA.sign(self.SECRET_KEY, "SHA256")
                        buffer = self.SERVER_RSA.encrypt(buffer[:self.RSA_SIZE//2]) + self.SERVER_RSA.encrypt(buffer[self.RSA_SIZE//2:self.RSA_SIZE])
                        
                        if self.send(buffer):
                            buffer = self.receive(self.RSA_SIZE)
                        if len(buffer) > 0:
                            buffer = self.CLIENT_RSA.decrypt(buffer)

                            self.SESSION_ID = buffer[0:8]
                            self.AES_CS = cipher.AES.new(buffer[24:56], cipher.MODE_CBC, buffer[8:24])
                            status = True
        return status

    def get_temperature(self):
        rec = self.request(b'\x10').decode("utf-8")
        return rec
   
    def toggle_led(self):
         rec_state = self.request(b'\x20').decode("utf-8")
         if rec_state == "25251":
              rec_state = "ON"
         else:
              rec_state = "OFF"
         return rec_state
        

    def request(self, req: bytes) -> bytes:
        # Encrypt the request using the established AES key
        buffer = req + self.SESSION_ID
        len = self.AES_CS.block_size - (len(buffer) % self.AES_CS.block_size)
        buffer = self.AES_CS.encrypt(buffer + bytes([len(buffer)] * len))
        self.send(buffer)

        # Receive the encrypted response
        buffer = self.receive(self.AES_CS.block_size)  # Assuming the response is 16 bytes
        buffer = self.AES_CS.decrypt(buffer)

        # Remove padding
        return buffer


