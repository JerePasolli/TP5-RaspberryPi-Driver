import os
import matplotlib.pyplot as plt
import time
import struct
from matplotlib.widgets import Button

# Nombre del archivo en /dev
FILE_PATH = "/dev/gpiodriver"

# Inicializar los datos
x_data = []
y_data = []
running = True

# Configurar la gráfica
plt.ion()  # Modo interactivo
fig, ax = plt.subplots()
plt.subplots_adjust(bottom=0.3)
line, = ax.plot(x_data, y_data, 'r-')  # Línea roja
ax.set_xlabel('Tiempo (S)')
ax.set_ylabel('Tensión (V)')

start_time = time.time()

# Función para manejar el cierre de la ventana
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
    

# Función para leer los datos del archivo
def read_data(file_path):
    try:
        with open(file_path, 'rb') as file:
            value = struct.unpack('i', file.read(4))[0]
            return int(value * 3.3)
    except Exception as e:
        print(f"Error leyendo el archivo: {e}")
        return None
    
# Función para resetear los datos
def reset_data():
    global x_data, y_data, start_time
    x_data = []
    y_data = []
    start_time = time.time()  # Reiniciar el tiempo
    
# Funciones de los botones para cambiar el archivo de lectura
def change_pin(pin):
    select_pin(FILE_PATH, pin)
    reset_data()

select_pin(FILE_PATH, "20")
buttons = []
for i in range(5):
    ax_button = plt.axes([0.1 + i * 0.18, 0.05, 0.15, 0.075])  # Ejes para cada botón
    button = Button(ax_button, f'Pin {i+17}')
    button.on_clicked(lambda event, idx=i: change_pin(str(idx+17)))
    buttons.append(button)
    
# Conectar el evento de cierre de la ventana
fig.canvas.mpl_connect('close_event', on_close)

# Loop para actualizar la gráfica
while running:
    # Leer el dato del archivo
    new_value = read_data(FILE_PATH)
    
    if new_value is not None:
        # Añadir el nuevo valor a los datos
        current_time = time.time()
        elapsed_time = current_time - start_time  # Tiempo en segundos desde el inicio
        x_data.append(elapsed_time)
        y_data.append(new_value)
        
        # Actualizar la línea de la gráfica
        line.set_xdata(x_data)
        line.set_ydata(y_data)
        
        # Ajustar los límites de la gráfica
        ax.relim()
        ax.autoscale_view()
        
        # Redibujar la gráfica
        plt.draw()
        plt.pause(1)  # Pausa de un segundo para actualizar la gráfica
        
    else:
        # Si no se puede leer el archivo, esperar un segundo antes de reintentar
        time.sleep(1)
        
plt.ioff()  # Desactivar el modo interactivo cuando el bucle se detiene
plt.show()  # Mostrar la gráfica al final (en caso de que el bucle se detenga)
