import tkinter as tk
from tkinter import ttk


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

        btn_temp = tk.Button(self.window, text="Get Temperature", command=self.get_temperature, width=15, height=1)
        btn_temp.place(x=450, y=10)
        btn_temp.config(state=tk.DISABLED)

        btn_led = tk.Button(self.window, text="Toggle LED", command=self.toggle_led, width=15, height=1)
        btn_led.place(x=630, y=10)
        btn_led.config(state=tk.DISABLED)

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
        pass

    def close_session(self):
        pass

    def get_temperature(self):
        pass

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
