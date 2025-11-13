#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define MOUSE_DEV "/dev/input/event5"  // Asegúrate de que el dispositivo sea el correcto

int main() {
    int fd;
    struct input_event ev;
    int dx = 0, dy = 0;  // Variables para acumular el desplazamiento

    // Abrir el dispositivo de entrada del ratón
    fd = open(MOUSE_DEV, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el dispositivo");
        return 1;
    }

    printf("Leyendo desplazamientos del ratón...\n");

    while (1) {
        // Leer los eventos del ratón
        if (read(fd, &ev, sizeof(struct input_event)) < sizeof(struct input_event)) {
            perror("Error al leer evento del ratón");
            close(fd);
            return 1;
        }

        // Filtrar los eventos de desplazamiento (EV_REL)
        if (ev.type == EV_REL) {
            if (ev.code == REL_X) {
                dx += ev.value;  // Acumular el desplazamiento en X
            } else if (ev.code == REL_Y) {
                dy += ev.value;  // Acumular el desplazamiento en Y
            }

            // Imprimir los desplazamientos crudos y acumulados en una sola línea
            printf("RAW X: %d | Acumulado X: %d | RAW Y: %d | Acumulado Y: %d\n", 
                   ev.value, dx, ev.value, dy);
        }
    }

    // Cerrar el dispositivo cuando termine
    close(fd);
    return 0;
}
