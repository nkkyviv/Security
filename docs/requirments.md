# Client Requirments

[ReqId:01v01]: It shall be possible to have a list of all the serial ports as a drop down on the GUI.
[ReqId:01v02]: It shall be possible to choose from the list the port connected to the microcontroller in the GUI.
[ReqId:01v03]: It shall be possible to have a functionality to establish a connection using a button in the Graphical user interface(GUI).
[ReqId:01v04]: It shall be possible to terminate a session using a button on the GUI.
[ReqId:01v05]: It shall be possible to get the temperature of the microcontroller using a button in the GUI.
[ReqId:01v06]: It shall be possible to toggle the led connected to the microcontroller using a button in the GUI.
[ReqId:01v07]: It shall be possible to view the log state on the text box.
[ReqId:01v08]: It shall be possible to view the log status on the text box of the GUI.
[ReqId:01v09]: It shall be possible to view the result of the request on the text box of the GUI.
[ReqId:01v10]: It shall be possible to clear the log in the text box of the GUI.
[ReqId:01v11]: The button to get the temperature and toggle the led shall be disabled if there is no communication.
[ReqId:01v12]: It shall be possible to change the label in the GUI to "Establish Session" if there is no session.
[ReqId:01v13]: The GUI shall have a functionality that changes the label to "Establish Session" if there is no connection and "Close Session" if there is a connection.

## Server Requirments

[ReqId:01v01]: It shall be possible to have a led connected to the server.
[ReqId:01v02]: It shall be possible to have a serial connection where the server and the client communicate with each other.
[ReqId:01v03]: It shall be possible to have a security protocol and application (HMAC-SHA256) to ensure the integrity and authencity of the messages and data in both the client and the server.
[ReqId:01v04]: It shall be possible to safely have a secure communication between the server and client using an asymmetric algorithm (RSA-2048).
[ReqId:01v05]: It shall be possible for the server to handle only one session at a time.
[ReqId:01v06]: It shall be possible to implement a logic to end the session between the client and the server after one minute of no communication.
