#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <linux/input.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#define SERVER "212.128.171.68"
#define PORT 45554
#define MOUSE_DEV "/dev/input/event5"
#define BUFSIZE 1024

int main() {
    int fd;
    struct input_event ev;
    int dx = 0, dy = 0;

    char buf[BUFSIZE];
    int addrlen=sizeof(struct sockaddr_in);
    long timevar;
    struct sockaddr_in serveraddr_in, mouseaddr_in;

    int s = socket (AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("Unable to create socket\n");
		exit(1);
	}

    memset ((char *)&mouseaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&serveraddr_in, 0, sizeof(struct sockaddr_in));

    serveraddr_in.sin_family = AF_INET;
    serveraddr_in.sin_port = htons(PORT);
    if(inet_pton(AF_INET,SERVER, (struct sockaddr *) &serveraddr_in.sin_addr) == -1){
        perror("Error creando IP servidor");
        exit(1);
    }

    if (connect(s, (struct sockaddr *)&serveraddr_in, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr,"Error conectándose a %s", SERVER);
		exit(1);
	}

    if (getsockname(s, (struct sockaddr *)&mouseaddr_in, &addrlen) == -1) {
		perror("Unable to read socket address\n");
		exit(1);
    }

    time(&timevar);
    printf("Conectado al servidor con IP %s en el puerto %u a las %s",
        SERVER, mouseaddr_in.sin_port, (char *) ctime(&timevar));

    fd = open(MOUSE_DEV, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el dispositivo de ratón.");
        return 1;
    }

    printf("Leyendo desplazamientos del ratón...\n");

    while (1) {
    	if (read(fd, &ev, sizeof(struct input_event)) < sizeof(struct input_event)) {
        	perror("Error al leer evento del ratón");
            close(fd);
            return 1;
    	}

        if (ev.type == EV_REL) {
            if (ev.code == REL_X) {
                dx += ev.value;
            } else if (ev.code == REL_Y) {
                dy += ev.value;
            }
            sprintf(buf,"RAW X: %d | Acumulado X: %d | RAW Y: %d | Acumulado Y: %d\n",
                       ev.value, dx, ev.value, dy);
	    printf("%s", buf);
            send(s,buf,strlen(buf)+1,0);
        }
    }

    close(fd);
    time(&timevar);
	printf("All done at %s", (char *)ctime(&timevar));
    return 0;
}
