import struct
import tkinter as tk
from tkinter import ttk
import serial.tools.list_ports
import protocol
import session

STATUS_OKAY = 0
STATUS_ERROR = 1

SESSION_CLOSE = 0
SESSION_GET_TEMP = 1
SESSION_TOGGLE_LED = 2
SESSION_ESTABLISH = 3

def list_serial_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

def create_window(serial_address, select_options):
    def establish():
        if button1.cget('text') == "Close Session":
            if protocol.send(bytes([SESSION_CLOSE])):
                received_bytes = protocol.receive(1)
                if received_bytes[0] == STATUS_OKAY:
                    log_text.insert(tk.END, "Session closed\n")
                    button1.config(text="Establish Session")
                    button2.config(state=tk.DISABLED)
                    button3.config(state=tk.DISABLED)
                    protocol.close()
        else:
            selected_port = select_combobox.get()
            if protocol.init(selected_port):
                #if protocol.send(bytes([SESSION_ESTABLISH])):
                    #received_bytes = protocol.receive(1)
                    if session.establish():
                    #if received_bytes[0] == STATUS_OKAY:
                        log_text.delete(1.0, tk.END)
                        log_text.insert(tk.END, f"Session established on {selected_port}\n")
                        button1.config(text="Close Session")
                        button1.config(state=tk.NORMAL)
                        button2.config(state=tk.NORMAL)
                        button3.config(state=tk.NORMAL)
                    else:
                        log_text.delete(1.0, tk.END)
                        log_text.insert(tk.END, f"Failed to establish session: {received_bytes}\n")
                        button1.config(text="Establish Session")
                        button1.config(state=tk.NORMAL)
                        button2.config(state=tk.DISABLED)
                        button3.config(state=tk.DISABLED)
                # else:
                #     log_text.delete(1.0, tk.END)
                #     log_text.insert(tk.END, "Failed to send establish session command\n")
                #     button1.config(text="Establish Session")
                #     button1.config(state=tk.NORMAL)
                #     button2.config(state=tk.DISABLED)
                #     button3.config(state=tk.DISABLED)
            else:
                log_text.delete(1.0, tk.END)
                log_text.insert(tk.END, "Failed to initialize port\n")
                button1.config(state=tk.DISABLED)
                button2.config(state=tk.DISABLED)
                button3.config(state=tk.DISABLED)

    def get_temp():
        if protocol.send(bytes([SESSION_GET_TEMP])):
            received_bytes = protocol.receive(5)
            if received_bytes[0] == STATUS_OKAY:
                temperature = struct.unpack('f', received_bytes[1:])[0]
                log_text.insert(tk.END, f"Temperature: {temperature:.2f}\n")
            else:
                log_text.insert(tk.END, "Failed to receive temperature.\n")
        else:
            log_text.insert(tk.END, "Failed to send get temperature command.\n")

    def toggle_led():
        if protocol.send(bytes([SESSION_TOGGLE_LED])):
            received_bytes = protocol.receive(2)
            if received_bytes[0] == STATUS_OKAY:
                if received_bytes[1] == 1:
                    log_text.insert(tk.END, "LED Status: ON\n")
                elif received_bytes[1] == 0:
                    log_text.insert(tk.END, "LED Status: OFF\n")
                else:
                    log_text.insert(tk.END, f"Unexpected LED status response: {received_bytes}\n")
            else:
                log_text.insert(tk.END, "Failed to send toggle LED command.\n")

    def on_clear_button_click():
        log_text.delete(1.0, tk.END)

    def update_button_state(event):
        selected_port = select_combobox.get()
        if selected_port == "":
            button1.config(state=tk.DISABLED)
            button2.config(state=tk.DISABLED)
            button3.config(state=tk.DISABLED)
        else:
            button1.config(state=tk.NORMAL)
            log_text.delete(1.0, tk.END)
            log_text.insert(tk.END, "Port selected\n")

    root = tk.Tk()
    root.title("Client")

    root.geometry("800x600")

    log_text = tk.Text(root, bg="black", fg="white", font=("Arial", 10))
    log_text.place(x=20, y=90, width=760, height=480)
    log_text.insert(tk.END, "Please select a port")

    serial_label_text = f"Serial Port: {serial_address}"
    serial_label = tk.Label(root, text=serial_label_text, font=("Arial", 10))
    serial_label.place(x=20, y=10)

    log_label_text = "Log:"
    log_label = tk.Label(root, text=log_label_text, font=("Arial", 10))
    log_label.place(x=20, y=60)

    clear_button = tk.Label(root, text='Clear', font=("Helvetica", 12), fg="blue", cursor="hand2")
    clear_button.place(x=750, y=60)
    clear_button.bind("<Button-1>", lambda event: on_clear_button_click())

    select_combobox = ttk.Combobox(root, values=select_options, state="readonly", width=15)
    select_combobox.set("")
    select_combobox.place(x=110, y=13)
    select_combobox.config(font=("Arial", 12))
    select_combobox.bind("<<ComboboxSelected>>", update_button_state)

    button1 = tk.Button(root, text="Establish Session", command=establish, width=15, height=1, state=tk.DISABLED)
    button1.place(x=270, y=10)
    button1.config(font=("Arial", 10))

    button2 = tk.Button(root, text="Get Temperature", command=get_temp, width=15, height=1, state=tk.DISABLED)
    button2.place(x=450, y=10)
    button2.config(font=("Arial", 10))

    button3 = tk.Button(root, text="Toggle LED", command=toggle_led, width=15, height=1, state=tk.DISABLED)
    button3.place(x=630, y=10)
    button3.config(font=("Arial", 10))

    root.mainloop()

# Example usage with a serial port address and options for the combobox
port_address = ""
combobox_options = list_serial_ports()
create_window(port_address, combobox_options)
