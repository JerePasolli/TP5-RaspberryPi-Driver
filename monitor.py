import matplotlib.pyplot as plt
import time
import struct
from matplotlib.widgets import TextBox
import threading

FILE_PATH = "/dev/gpiodriver"

x_data = []
y_data = []
running = True
window_size = 40

# Configurar la gráfica
plt.ion() 
fig, ax = plt.subplots()
plt.subplots_adjust(bottom=0.3)
line, = ax.plot(x_data, y_data, 'r-')
ax.set_xlabel('Tiempo (S)')
ax.set_ylabel('Tensión (V)')
ax.set_xlim(0, window_size/2)
ax.set_ylim(-0.1, 3.5)

start_time = time.time()

def on_close(event):
    global running
    running = False

def select_pin(file_path, pin):
    try:
        with open(file_path, 'w') as file:
            file.write(pin)
    except Exception as e:
        print(f"Error escribiendo el archivo: {e}")
        return None
    

def read_data(file_path):
    try:
        with open(file_path, 'rb') as file:
            value = struct.unpack('i', file.read(4))[0]
            return int(value * 3.3)
    except Exception as e:
        print(f"Error leyendo el archivo: {e}")
        return None
    
def reset_data():
    global x_data, y_data, start_time
    x_data = []
    y_data = []
    start_time = time.time()
    

def change_pin(pin):
    if pin == "":
        return
    number = int(pin)
    try:
        if 0 <= number <= 21:
            select_pin(FILE_PATH, pin)
            print(f"Cambiado a pin {pin}")
            text_box.label.set_color('green')
            reset_data()
        else:
            print("Número de pin fuera de rango. Ingrese un número entre 0 y 21")
            text_box.label.set_color('red')
    except ValueError:
        print("Entrada inválida. Por favor, ingrese un número.")
        text_box.label.set_color('red')
    finally:
        timer = threading.Timer(2, lambda: text_box.label.set_color(original_color))
        timer.start()
        text_box.set_val('')

print("Seleccionado pin 20 por defecto")

axbox = plt.axes([0.3, 0.15, 0.4, 0.05]) 
text_box = TextBox(axbox, 'Seleccionar Pin (0-21): ')
text_box.on_submit(change_pin)
original_color = 'black'
    
fig.canvas.mpl_connect('close_event', on_close)

while running:
    new_value = read_data(FILE_PATH)
    
    if new_value is not None:
        current_time = time.time()
        elapsed_time = current_time - start_time
        x_data.append(elapsed_time)
        y_data.append(new_value)
        
        if len(x_data) > window_size:
            x_data = x_data[-window_size:]
            y_data = y_data[-window_size:]
        
        line.set_xdata(x_data)
        line.set_ydata(y_data)
        
        if len(x_data) >= window_size:
            ax.set_xlim(elapsed_time - window_size/2, elapsed_time)
        else:
            ax.set_xlim(0, window_size/2)
        
        ax.relim()
        ax.autoscale_view(scaley=True)
        
        plt.draw()
        plt.pause(0.5)
        
    else:
        time.sleep(0.5)
        
plt.ioff()
plt.show()
