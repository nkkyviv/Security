# Client Requirements

[ReqId:01v01]: It shall be possible to have a list of all the serial ports as a drop down in the GUI.
[ReqId:01v02]: It shall be possible to choose from the list of ports, the port connected to the microcontroller in the GUI.
[ReqId:01v03]: It shall be possible to have a functionality to establish a connection using a button in the Graphical user interface(GUI).
[ReqId:01v04]: It shall be possible to terminate a session using a button in the GUI.
[ReqId:01v05]: It shall be possible to get the core temperature of the microcontroller using a button in the GUI.
[ReqId:01v06]: It shall be possible to toggle the led connected to the microcontroller using a button in the GUI.
[ReqId:01v07]: It shall be possible to view the log state on the text box.
[ReqId:01v08]: It shall be possible to view the log status on the text box of the GUI.
[ReqId:01v09]: It shall be possible to view the result of the request on the text box of the GUI.
[ReqId:01v10]: It shall be possible to clear the log in the text box of the GUI.
[ReqId:01v11]: The button to get the temperature and toggle the led shall be disabled if there is no communication.
[ReqId:01v12]: It shall be possible to change the label in the GUI to "Establish Session" if there is no session.
[ReqId:01v13]: The GUI shall have a functionality that changes the label to "Establish Session" if there is no connection and "Close Session" if there is a connection.

## Server Requirements

[ReqId:01v01]: It shall be possible to have a led connected to the server.
[ReqId:01v02]: It shall be possible to have a serial connection where the server and the client communicate with each other.
[ReqId:01v03]: It shall be possible to generate a temporary random pair of keys (a public key and a private key) for both the client and the server according to the sequence diagram.
[ReqId:01v04]: It shall be possible for the client to send its public key without encryption to the server.
[ReqId:01v05]: It shall be possible for the server encrypts its public key using the client public key and its should be protected using HMAC-256.
[ReqId:01v06]: It shall be possible for the server to send the Cipher to the client securely using AES-256.
[ReqId:01v07]: It shall be possible for the client to decrypt the Cipher with its private key to get the server's public key.
[ReqId:01v08]: It shall be possible for the client to encrypt its public key with the server's key.
[ReqId:01v09]: It shall be possible for the client for the client to send the cipher and calculated HASH to the server.
[ReqId:01v10]: It shall be possible for the server to decrypt the cipher and get the public key of the client.
[ReqId:01v11]: It shall be possible for the client to expire its private and public key.
[ReqId:01v12]: It shall be possible for the client to generate a new set of public and private keys.
[ReqId:01v13]: It shall be possible for thr user to establish a session which is managed by the server.
[ReqId:01v14]: It shall be possible for the client to send a request to the server to establish a session securely.
[ReqId:01v15]: It shall be possible for the server to handle only one session at a time.
[ReqId:01v16]: It shall be possible for the user to end a session.
[ReqId:01v17]: It shall be possible for the client to send a request to the server to terminate a session securely.
[ReqId:01v18]: It shall be possible to implement a logic to end the session between the client and the server after one minute of no communication.
