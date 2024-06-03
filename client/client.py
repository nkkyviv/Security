import struct
import tkinter as tk
from tkinter import ttk
import serial.tools.list_ports
import protocol
from session import Session

HIGH = "1"
LOW = "0"

class ClientApp:
    def __init__(self, master, select_options):
        self.master = master
        self.session = None
        self.create_widgets(select_options)

    def create_widgets(self, select_options):
        self.master.title("Client")
        self.master.geometry("800x600")

        self.log_text = tk.Text(self.master, bg="black", fg="white", font=("Arial", 12))
        self.log_text.place(x=20, y=90, width=760, height=480)
        self.print_log("Invalid port. Select a serial port")

        serial_label_text = "Serial Port: "
        self.serial_label = tk.Label(self.master, text=serial_label_text, font=("Arial", 12))
        self.serial_label.place(x=20, y=10)

        log_label_text = "Log:"
        self.log_label = tk.Label(self.master, text=log_label_text, font=("Arial", 12))
        self.log_label.place(x=20, y=60)

        self.clear_button = tk.Label(self.master, text='Clear', font=("Helvetica", 12), fg="blue", cursor="hand2")
        self.clear_button.place(x=730, y=60)
        self.clear_button.bind("<Button-1>", lambda event: self.on_clear_button_click())

        self.select_combobox = ttk.Combobox(self.master, values=select_options, state="readonly", width=15)
        self.select_combobox.set("")
        self.select_combobox.place(x=110, y=13)
        self.select_combobox.config(font=("Arial", 12))
        self.select_combobox.bind("<<ComboboxSelected>>", self.update_button_state)

        self.button_session = tk.Button(self.master, text="Establish Session", command=self.establish_session, width=15, height=1, state=tk.DISABLED)
        self.button_session.place(x=270, y=10)
        self.button_session.config(font=("Arial", 10))

        self.getTemp_button = tk.Button(self.master, text="Get Temperature", command=self.get_temp, width=15, height=1, state=tk.DISABLED)
        self.getTemp_button.place(x=450, y=10)
        self.getTemp_button.config(font=("Arial", 10))

        self.toggleLed_button = tk.Button(self.master, text="Toggle LED", command=self.toggle_led, width=15, height=1, state=tk.DISABLED)
        self.toggleLed_button.place(x=630, y=10)
        self.toggleLed_button.config(font=("Arial", 10))

    def establish_session(self):
        success = Session.select()
        if success:
            self.print_log("Session established")
            self.button_session.config(text="Close Session", command=self.close_session)
            self.getTemp_button.config(state=tk.NORMAL)
            self.toggleLed_button.config(state=tk.NORMAL)
        else:
            self.print_log("Failed to establish session")
            self.getTemp_button.config(state=tk.DISABLED)
            self.toggleLed_button.config(state=tk.DISABLED)

    def close_session(self):
        success = True

        if success:
            self.print_log("Session closed")
            self.button_session.config(text="Establish Session", command=self.establish_session)
            self.getTemp_button.config(state=tk.DISABLED)
            self.toggleLed_button.config(state=tk.DISABLED)
        else:
            self.print_log("Failed to send close session command")
            self.getTemp_button.config(state=tk.DISABLED)
            self.toggleLed_button.config(state=tk.DISABLED)

    def get_temp(self):
        if self.session:
            received_bytes = self.session.get_temperature()
            if received_bytes is not None:
                try:
                    temperature = received_bytes[0:].decode("utf-8")
                    message = f"Temperature: {temperature} °C"
                    self.print_log(message)
                except Exception as e:
                    self.clear_text()
                    self.print_log(f"Error: Unable to get temperature. {str(e)}")
            else:
                self.clear_text()
                self.print_log("Error: Unable to get temperature")
        else:
            self.print_log("No active session")

    def toggle_led(self):
        if self.session:
            state = self.session.toggle_led()
            if state is not None:
                try:
                    state_str = state.decode("utf-8").strip('\x00')
                    if state_str == HIGH:
                        self.print_log("LED State: ON")
                    elif state_str == LOW:
                        self.print_log("LED State: OFF")
                    else:
                        self.print_log("Unexpected LED status response")
                except Exception as e:
                    self.print_log(f"Error decoding LED status: {str(e)}")
            else:
                self.print_log("Failed to send toggle LED command")
        else:
            self.print_log("No active session")

    def on_clear_button_click(self):
        self.log_text.delete(1.0, tk.END)

    def update_button_state(self, event):
        port = self.select_combobox.get()
        self.session = Session(port)
        self.print_log(f"Serial port Selected: {port}")
        self.button_session.config(state=tk.NORMAL)
        self.getTemp_button.config(state=tk.DISABLED)
        self.toggleLed_button.config(state=tk.DISABLED)

    def print_log(self, log):
        self.log_text.insert(tk.END, log + '\n')
        self.log_text.see(tk.END)

    def clear_text(self):
        self.log_text.delete(1.0, tk.END)

def list_serial_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

if __name__ == "__main__":
    root = tk.Tk()
    combobox_options = list_serial_ports()
    app = ClientApp(root, combobox_options)
    root.mainloop()
