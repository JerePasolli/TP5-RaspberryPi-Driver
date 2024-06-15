# TP5 - RaspberryPi-Driver - Sistemas de Computación 2024

## Integrantes:

- Nestor Jeremías Pasolli
- Alex Agustín Hernando
- Tomás Moyano

## Consigna

Un "driver" es aquel que conduce, administra, controla, dirige, monitorea la entidad bajo su mando. Un "bus driver" hace eso con un "bus". De manera similar, un "device driver" hace eso con un dispositivo. Un dispositivo puede ser cualquier periférico conectado a una computadora, por ejemplo, un mouse, un teclado, una pantalla/monitor, un disco duro, una cámara, un reloj, etc., cualquier cosa.

Un "driver" puede ser una persona o sistemas automáticos, posiblemente monitoreados por otra persona. Del mismo modo, el "device driver" podría ser una pieza de software u otro periférico/dispositivo, posiblemente controlado por un software. Sin embargo, si se trata de otro periférico/dispositivo, se denomina "device controller" en el lenguaje común. Y por "driver" solo nos referimos a un "software driver". Un "device controller" es un dispositivo en sí mismo y, por lo tanto, muchas veces también necesita un "driver", comúnmente conocido como "bus driver".

Los ejemplos generales de "device controller" incluyen controladores de disco duro, controladores de pantalla, controladores de audio para los dispositivos correspondientes. Ejemplos más técnicos serían los controladores para los protocolos de hardware, como un controlador IDE, un controlador PCI, un controlador USB, un controlador SPI, un controlador I2C, etc. 

En el desarrollo de esta clase veremos estas sutiles diferencias y aprenderemos a construir un "driver" de caracteres.

Para superar este TP tendrán que diseñar y construir un CDD que permita sensar dos señales externas con un periodo de UN segundo. Luego una aplicación a nivel de usuario deberá leer UNA de las dos señales y graficarla en función del tiempo. La aplicación tambien debe poder indicarle al CDD cuál de las dos señales leer. Las correcciones de escalas de las mediciones, de ser necesario, se harán a nivel de usuario. Los gráficos de la señal deben indicar el tipo de señal que se
está sensando, unidades en abcisas y tiempo en ordenadas. Cuando se cambie de señal el gráfico se debe "resetear" y acomodar a la nueva medición.

Se recomienda utilizar una Raspberry Pi para desarrollar este TP.

## Desarrollo

Para comunicarse con el dispositivo de hardware (el sensor), implementaremos un controlador para él. En Linux, esto generalmente se implementa como un módulo del núcleo cargable. Un módulo del núcleo de Linux es una pieza de código del núcleo que puede cargarse en el núcleo durante el tiempo de ejecución utilizando, por ejemplo, los comandos "modprobe" o "insmod". Esto significa que el código del módulo del núcleo no necesita estar compilado en el núcleo de Linux desde el principio, sino que puede cargarse y retirarse dinámicamente más tarde. Esto proporciona una gran flexibilidad para extender la funcionalidad del núcleo y, por lo tanto, del sistema.

La razón por la que se necesita un módulo del núcleo de Linux es debido a que un módulo del núcleo de Linux se ejecuta en el espacio del núcleo, en contraste con las aplicaciones habituales que se ejecutan en el espacio de usuario. Mientras que una aplicación en el espacio de usuario tiene muchas restricciones por motivos de seguridad y estabilidad y, por ejemplo, solo puede acceder a su propio espacio de direcciones virtuales, el código que se ejecuta en el espacio del núcleo tiene privilegios especiales. Un módulo del núcleo de Linux puede acceder al hardware y puede, por ejemplo, registrar interrupciones. Para medir tiempos exactos, las interrupciones son esenciales. Medir intervalos de tiempo en el espacio de usuario está siempre bajo la influencia del planificador (scheduler) y puede llevar algún tiempo hasta que el proceso en el espacio de usuario vuelva a ser programado, lo que influirá en la medición.

Sin embargo, como el tío Ben le dijo al joven Peter Parker: "Con gran poder viene una gran responsabilidad", y mientras que tener un error en una aplicación en el espacio de usuario solo lleva al fallo de un único proceso, los errores en el espacio del núcleo pueden hacer que todo el sistema falle. Por esta razón, solo queremos implementar una funcionalidad mínima en el módulo del núcleo de Linux. En este ejemplo, solo queremos poder iniciar una medición y leer el valor medido del sensor. Las aplicaciones individuales de usuario, como un algoritmo de localización y mapeo, deben mantenerse fuera del núcleo y es mejor implementarlas en el espacio de usuario, por ejemplo, en C++, Python, etc.

Esto nos lleva a la pregunta de cómo un módulo del núcleo de Linux que se ejecuta en el espacio del núcleo puede interactuar con una aplicación en el espacio de usuario escrita en cualquier lenguaje de programación. Siguiendo la idea de UNIX de que "todo es un archivo", un módulo del núcleo de Linux puede registrar un dispositivo de carácter y exponer su funcionalidad a través de la interfaz del sistema de archivos. En este ejemplo, registraremos un dispositivo de carácter bajo "/dev/hc-sr04". Los procesos en el espacio de usuario pueden usar las llamadas del sistema de Linux (open, close, read, write, seek, etc.) para comunicarse con el núcleo.

La siguiente imagen muestra una visión general completa. Implementaremos la parte del controlador de dispositivo de Linux. Esta parte expone el dispositivo de carácter "/dev/hc-sr04". La aplicación de usuario puede interactuar con el módulo del núcleo mediante llamadas del sistema como open, read, write.