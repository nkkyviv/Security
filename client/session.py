from mbedtls import pk, hmac, hashlib, cipher
import protocol
import time

class Session:
    state = None
    RSA_SIZE = 256
    EXPONENT = 65537
    SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"
    SESSION_TEMPERATURE = 0x02
    SESSION_TOGGLE_LED = 0x03
    SESSION_OKAY = 0x01
    SESSION_CLOSE = 0xFF
    TIMEOUT_DURATION = 60
    
    def __init__(self, port):
        self.SESSION_ID = b''
        self.SERVER_RSA = None
        self.aes = None 
        self.HMAC_CS = hashlib.sha256()
        self.HMAC_CS.update(self.SECRET_KEY)
        self.HMAC_CS = self.HMAC_CS.digest()
        self.HMAC_CS = hmac.new(self.HMAC_CS, digestmod="SHA256")
        protocol.init(port)
        self.CLIENT_RSA = pk.RSA()
        self.CLIENT_RSA.generate(self.RSA_SIZE * 8, self.EXPONENT)
        self.exchange_keys()
        Session.state = port
        self.last_activity_time = time.time()
        
    def select():
        Session.state
        if Session.state != None:
            last_activity_time = time.time()
            Session.state = True
        else:
            Session.state = False
        return Session.state
    
    def get_temperature(self):
        if self.is_timed_out():
            return None
        self.update_activity_time()
        recieved = self.request(int(self.SESSION_TEMPERATURE))
        return recieved
    
    def toggle_led(self):
        if self.is_timed_out():
            return None
        self.update_activity_time()
        rec = self.request(int(self.SESSION_TOGGLE_LED))
        return rec
        
    def exchange_keys(self):
        if True == self.send(self.CLIENT_RSA.export_public_key()):

            buffer = self.receive(2 * self.RSA_SIZE)
            server_key_pub = self.CLIENT_RSA.decrypt(buffer[0:self.RSA_SIZE])
            server_key_pub += self.CLIENT_RSA.decrypt(buffer[self.RSA_SIZE:2*self.RSA_SIZE])
            self.SERVER_RSA = pk.RSA().from_DER(server_key_pub)

            del self.CLIENT_RSA
            self.CLIENT_RSA = pk.RSA()
            self.CLIENT_RSA.generate(self.RSA_SIZE * 8, self.EXPONENT)

            buffer = self.CLIENT_RSA.export_public_key() + self.CLIENT_RSA.sign(self.SECRET_KEY, "SHA256")
            buffer = self.SERVER_RSA.encrypt(buffer[0: 184]) + self.SERVER_RSA.encrypt(buffer[184:368]) + self.SERVER_RSA.encrypt(buffer[368:550])
            if True == self.send(buffer):
                buffer = self.receive(self.RSA_SIZE)

                if b"OKAY" == self.CLIENT_RSA.decrypt(buffer):
                    self.establish()

    def send(self, buf: bytes) -> bool:
        self.HMAC_CS.update(buf)
        buf += self.HMAC_CS.digest()
        return protocol.send(buf)

    def receive(self, size: int) -> bytes:
        buffer = protocol.receive(size + self.HMAC_CS.digest_size)
        self.HMAC_CS.update(buffer[0:size])
        if buffer[size:size + self.HMAC_CS.digest_size] == self.HMAC_CS.digest():
            return buffer[0:size]
        else:
            return b''

    def establish(self):
        try:
            buffer = self.CLIENT_RSA.sign(self.SECRET_KEY, "SHA256")
            buffer = self.SERVER_RSA.encrypt(buffer[0:self.RSA_SIZE//2]) + self.SERVER_RSA.encrypt(buffer[self.RSA_SIZE//2:self.RSA_SIZE])
            self.send(buffer)
            
            buffer = self.receive(self.RSA_SIZE)  
            buffer = self.CLIENT_RSA.decrypt(buffer)
            self.SESSION_ID = buffer[0:8]
            self.aes = cipher.AES.new(buffer[24:56], cipher.MODE_CBC, buffer[8:24])
        except Exception as e:
            protocol.close()
            print(e)
        
    def request(self, message) -> bytes:
        request = bytes([message]) 
        buffer = request + self.SESSION_ID
        plen = cipher.AES.block_size - (len(buffer) % cipher.AES.block_size)
        buffer = self.aes.encrypt(buffer + bytes([len(buffer)] * plen))
        if True == self.send(buffer):

            buffer = self.receive(cipher.AES.block_size)
            buffer = self.aes.decrypt(buffer)

            if buffer[0] == self.SESSION_OKAY:
                return buffer[1:6]
            else:
                return None
            
    def is_timed_out(self):
        return time.time() - self.last_activity_time > self.TIMEOUT_DURATION

    def update_activity_time(self):
        self.last_activity_time = time.time()
        


