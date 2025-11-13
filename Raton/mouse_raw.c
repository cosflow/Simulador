#include <stdio.h>
#include <stdlib.h>
#include <libinput.h>
#include <unistd.h>
#define MOUSE_DEV "/dev/input/event7"

// Parámetros del ratón
#define DPI 800  // DPI del ratón (800 puntos por pulgada)
#define CM_PER_UNIT (2.54 / DPI)  // Factor de conversión de unidades a centímetros

int main() {
    struct libinput *li;
    struct libinput_event *event;
    struct libinput_device *device;
    const char *device_path = MOUSE_DEV;
    
    // Crear contexto libinput utilizando la interfaz predeterminada
    li = libinput_path_create_context(&libinput_path_interface, NULL);
    if (!li) {
        fprintf(stderr, "No se pudo crear contexto libinput. Asegúrate de tener instalada la librería libinput correctamente.\n");
        exit(1);
    }

    // Intentar añadir el dispositivo de entrada (ratón)
    device = libinput_path_add_device(li, device_path);
    if (!device) {
        fprintf(stderr, "No se pudo abrir el dispositivo %s. Verifica que el dispositivo exista y tenga permisos correctos.\n", device_path);
        libinput_unref(li);
        exit(1);
    }

    printf("Leyendo eventos del ratón...\n");

    int dx = 0, dy = 0;

    while (1) {
        // Procesar los eventos del ratón
        libinput_dispatch(li);

        // Leer los eventos de entrada
        while ((event = libinput_get_event(li))) {
            switch (libinput_event_get_type(event)) {
                case LIBINPUT_EVENT_POINTER_MOTION:
                    // Obtener el desplazamiento en los ejes X y Y
                    double x = libinput_event_pointer_get_dx(event);
                    double y = libinput_event_pointer_get_dy(event);

                    // Acumular el desplazamiento
                    dx += (int)x;
                    dy += (int)y;

                    // Imprimir los desplazamientos
                    printf("RAW X: %.2f, Y: %.2f | Acumulado: (%d, %d)\n", x, y, dx, dy);
                    break;

                default:
                    // Ignorar otros tipos de eventos
                    break;
            }
            libinput_event_destroy(event);  // Destruir el evento después de procesarlo
        }

        usleep(1000);  // 1ms para evitar consumir recursos innecesarios
    }

    // Liberar el contexto libinput cuando termine
    libinput_unref(li);
    return 0;
}
