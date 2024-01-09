import tkinter as tk
from tkinter import ttk
import client


class Client:
    def __init__(self, window):
        self.window = window
        window.title("Client")
        window.geometry("800x600")
        window.resizable(False, False)

        self.create_widgets()

    def create_widgets(self):
        self.create_buttons()
        self.text_box()

    def create_buttons(self):
        btn_session = tk.Button(self.window, text="Establish Session", command=self.establish_session, width=15, height=1)
        btn_session.place(x=270, y=10)
        self.session_button = btn_session

        btn_temp = tk.Button(self.window, text="Get Temperature", command=self.get_temperature, width=15, height=1)
        btn_temp.place(x=450, y=10)
        btn_temp.config(state=tk.DISABLED)
        self.btn_temp = btn_temp

        btn_led = tk.Button(self.window, text="Toggle LED", command=self.toggle_led, width=15, height=1)
        btn_led.place(x=630, y=10)
        btn_led.config(state=tk.DISABLED)
        self.btn_led = btn_led

        clear_label = tk.Label(self.window, text='Clear', font=("Helvetica", 12), fg="blue", cursor="hand2")
        clear_label.place(x=730, y=49)
        clear_label.bind("<Button-1>", lambda event: self.clear_text())

        lbl = tk.Label(window, text="Serial Port :", font=("Helvetica", 12))
        lbl.place(x=20, y=13)

        lbl_log = tk.Label(window, text="Log :", font=("Helvetica", 12))
        lbl_log.place(x=20, y=40)

        options = ['/dev/ttyUSB0', '/dev/ttyUSB1']
        self.combo = ttk.Combobox(window, state="readonly", width=15, height=4, value=options)
        self.combo.place(x=110, y=13)
        self.combo.bind("<<ComboboxSelected>>", self.display_selection)

    def establish_session(self):
            
        selected_port = self.combo.get()
        log_message = f"Selected Serial Port: {selected_port}"
        
        establish = client.establish_session(selected_port)
        
        if establish:
            self.print_log("Session established")
            self.btn_temp.config(state=tk.NORMAL)
            self.btn_led.config(state=tk.NORMAL)
            self.session_button.config(text="Close Session", command=self.close_session)
        else:
            self.print_log("Session not established")
            self.btn_temp.config(state=tk.DISABLED)
            self.btn_led.config(state=tk.DISABLED)

    def close_session(self):
        selected_port = self.combo.get()
        log_message = f"Selected Serial Port: {selected_port}"
        
        close = client.close_session(selected_port)
        
        if close:
            self.print_log("Session closed")
            self.btn_temp.config(state=tk.DISABLED)
            self.btn_led.config(state=tk.DISABLED)
            self.session_button.config(text="Establish Session", command=self.establish_session)
        else:
            self.print_log("Session not closed")
            self.btn_temp.config(state=tk.DISABLED)
            self.btn_led.config(state=tk.DISABLED)

    def get_temperature(self):
        selected_port = self.combo.get()
        log_message = f"Selected Serial Port: {selected_port}"
        
        temperature = client.get_temperature(selected_port)
        

        if temperature is not None:
            log_message = f"Temperature: {temperature}"
            self.print_log(log_message)
        else:
            self.print_log("Error: Unable to get temperature")
        

    def toggle_led(self):
        pass

    def text_box(self):
        frame = tk.Frame(self.window, width=780, height=510, bg="black")
        frame.place(x=10, y=70)

        self.log_text = tk.StringVar()
        self.label1 = tk.Label(master=frame, textvariable=self.log_text, bg="black", fg="white", font=("Courier", 10))
        self.label1.place(x=0, y=0)  # Place label at (0, 0) 
        

    def clear_text(self):
        self.log_text.set("")

    def print_log(self, log):
        current_text = self.log_text.get()
        new_text = current_text + "\n" + log
        self.log_text.set(new_text)
        
        
    def display_selection(self, event):
        selected_port = self.combo.get()
        log_message = f"Selected Serial Port: {selected_port}"
        self.print_log(log_message)
        
        

if __name__ == "__main__":
    window = tk.Tk()
    application = Client(window)
    window.mainloop()
