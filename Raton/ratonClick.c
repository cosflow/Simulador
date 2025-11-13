#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#define MOUSE_DEV "/dev/input/event6"

int main() {
    int fd;
    struct input_event ev;
    int dx = 0, dy = 0;

    fd = open(MOUSE_DEV, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el dispositivo");
        return 1;
    }

    printf("Leyendo eventos del ratón...\n");

    while (1) {
        if (read(fd, &ev, sizeof(struct input_event)) < sizeof(struct input_event)) {
            perror("Error al leer evento del ratón");
            close(fd);
            return 1;
        }

        // Movimiento
        if (ev.type == EV_REL) {
            if (ev.code == REL_X)
                dx += ev.value;
            else if (ev.code == REL_Y)
                dy += ev.value;

            printf("Mov: X=%d (acum %d) | Y=%d (acum %d)\n", ev.value, dx, ev.value, dy);
        }

        // Clics
        else if (ev.type == EV_KEY) {
            if (ev.code == BTN_LEFT)
                printf("Botón IZQUIERDO %s\n", ev.value ? "PRESIONADO" : "SOLTADO");
            else if (ev.code == BTN_RIGHT)
                printf("Botón DERECHO %s\n", ev.value ? "PRESIONADO" : "SOLTADO");
            else if (ev.code == BTN_MIDDLE)
                printf("Botón CENTRAL %s\n", ev.value ? "PRESIONADO" : "SOLTADO");
        }

        // Sincronización (opcional)
        else if (ev.type == EV_SYN) {
            // Fin de paquete de eventos
        }
    }

    close(fd);
    return 0;
}

