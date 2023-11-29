import matplotlib.pyplot as plt
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.animation import FuncAnimation
import serial
import threading
from conversion import convert_impulses2_xy


root = tk.Tk()
root.title("Line Follower")

fig, ax = plt.subplots()
line, = ax.plot([], [], lw=2)
line2, = ax.plot([], [], lw=2)

x = [0]
y = [0]
alpha = 0
r = 1.453
L = 7.512
total_rotations = 600

recieved_data = []
data_to_send = []

# power_on = False

class CustomIterator:
    def __init__(self):
        self.current_frame = 0
    def __iter__(self):
        return self
    def __next__(self):
        return self.current_frame


def update_plot(i):
    global alpha
    if recieved_data:
        imp_L, imp_R = recieved_data.pop(0)
        x_mov, y_mov, alpha = convert_impulses2_xy(imp_R, imp_L, r, L, total_rotations, alpha)
        x.append(x[-1] + x_mov)
        y.append(y[-1] + y_mov)

    line.set_data(x, y)
    ax.relim()
    ax.autoscale_view()


# def toggle():
#     global power_on
#
#     if power_button.config('relief')[-1] == 'sunken':
#         power_button.config(relief="raised")
#         power_label.config(text="Robot OFF")
#         power_on = False
#         data_to_send.append("power=off")
#     else:
#         power_button.config(relief="sunken")
#         power_label.config(text="power=on")
#         power_on = True
#         data_to_send.append("power=on")


def calibrate():
    data_to_send.append("calibrate \n")


def start():
    data_to_send.append("start \n")


def track_reset():
    global x
    global y
    global alpha
    alpha = 0
    x.clear()
    y.clear()
    x.append(0)
    y.append(0)


def getPID():
    data_to_send.append("getPID \n")


def writePID():
    try:
        kP = pid_kP.get()
        kI = pid_kI.get()
        kD = pid_kD.get()
        data_to_send.append("writePID " + kP + " " + kI + " " + kD + " \n")
    except:
        print("Niepoprawne dane")


def command_send():
    command = command_box.get(1.0, "end-1c")
    command += " \n"
    if command:
        data_to_send.append(command)
        command_box.delete('1.0', tk.END)


def setup_connection():
    port="COM8"
    s = serial.Serial(port, 9600)  # Start communications with the bluetooth unit
    return s


s = setup_connection()


def recieve_data(*args):
    global recieved_data
    global s
    global data_to_send
    if (s.inWaiting() > 0):

        data = s.readline()
        if data:
            if data[-1] == " ":
                data = data[:-1]

            recieved = data.decode()

            if recieved[0] == "r":
                recieved = recieved[1:]
                splited = recieved.split(" ")
                element= [int(i) for i in splited]
                recieved_data.append(element)
            else:
                print("Odczyt: ", data)

            if recieved[0] == "k":
                splited = recieved.split(" ")
                pid_vP.set(float(splited[1]))
                pid_vI.set(float(splited[3]))
                pid_vD.set(float(splited[5]))




    if data_to_send:
        d_t_s = str.encode(data_to_send.pop(0))
        s.write(d_t_s)
        print("Wysyłanie: ", d_t_s)
    threading.Timer(0.01, recieve_data).start()


canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

iterator = CustomIterator()
ani = FuncAnimation(fig, update_plot, frames=iterator, interval=1)

button_frame = tk.Frame(root)
button_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

# power_label = tk.Label(root, text = "Robot OFF")
# power_label.pack()
#
# power_button = tk.Button(root, text="Drive!", command=toggle, relief="raised")
# power_button.pack()

power_button = tk.Button(root, text="Calibrate", command=calibrate)
power_button.pack()

power_button = tk.Button(root, text="Start", command=start)
power_button.pack(pady=(10, 0))

track_reset = tk.Button(root, text="Track reset", command=track_reset)
track_reset.pack(pady=(25, 0))

pid_label = tk.Label(root, text = "Parametry PID")
pid_label.pack(pady=(30, 0))

pid_label_P = tk.Label(root, text = "P")
pid_label_P.pack()
pid_vP = tk.DoubleVar(value = 1.0)
pid_kP = tk.Spinbox(root, from_= 0, to = 100, width=8, increment=0.1, textvariable=pid_vP)
pid_kP.pack()

pid_label_I = tk.Label(root, text = "I")
pid_label_I.pack(pady=(10, 0))
pid_vI = tk.DoubleVar(value = 1.0)
pid_kI = tk.Spinbox(root, from_= 0, to = 100, width=8, increment=0.1, textvariable=pid_vI)
pid_kI.pack()

pid_label_D = tk.Label(root, text = "D")
pid_label_D.pack(pady=(10, 0))
pid_vD = tk.DoubleVar(value = 1.0)
pid_kD = tk.Spinbox(root, from_= 0, to = 100, width=8, increment=0.1, textvariable=pid_vD)
pid_kD.pack()

power_button = tk.Button(root, text="Pobierz wartości", command=getPID)
power_button.pack(pady=(30, 0))

power_button = tk.Button(root, text="Wyślij wartości", command=writePID)
power_button.pack(pady=(10, 0))

command_box_label = tk.Label(root, text = "CommandBox")
command_box_label.pack(pady=(30, 0))

command_box = tk.Text(root, height=1, width=16)
command_box.pack()

command_box_label_button = tk.Button(root, text="Send command", command=command_send)
command_box_label_button.pack()





recieve_data()
root.mainloop()
# while True:
#     root.update()


s.close()