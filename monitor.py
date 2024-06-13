import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

# Ruta del archivo
FILE_PATH = '/proc/gpio'

# Función para leer el valor del archivo
def read_gpio_value(file_path):
    try:
        with open(file_path, 'r') as file:
            value = file.read().strip()
            print(value)
            return int(value * 3.3)
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return 0

# Configuración de la gráfica
fig, ax = plt.subplots()
x_data, y_data = [], []
ln, = plt.plot([], [], 'ro', animated=True)

def init():
    ax.set_xlim(0, 10)  # Ajusta el límite del eje x según sea necesario
    ax.set_ylim(-0.1, 3.3)
    return ln,

def update(frame):
    x_data.append(time.time())
    y_data.append(read_gpio_value(FILE_PATH))

    # Ajustar el eje x para desplazar la ventana de tiempo
    if len(x_data) > 10:
        ax.set_xlim(x_data[-10], x_data[-1])

    ln.set_data(x_data, y_data)
    return ln,

ani = animation.FuncAnimation(fig, update, init_func=init, blit=True, interval=1000)

plt.show()
