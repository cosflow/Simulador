#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <linux/input.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#define BUFSIZEC      16
#define BUFSIZEV      2
#define REMOTE_PORT   45554
#define LOCAL_PORT    45454
//#define REMOTE_IP    "192.168.1.40"
#define REMOTE_IP     "212.128.171.68"
#define LOCAL_IP      "127.0.0.1"
#define MOUSE_DEV     "/dev/input/event5"

void * enviarPosRaton(void * arg);

int main() {
	char bufCoord[BUFSIZEC];
	char bufVib[BUFSIZEV];
	char vibId;
	pthread_t hilo_Raton;
	long timevar;
	int opt = 1;
	int addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in remoteAddr_in, localAddr_in;
	int remoteSocket  = socket(AF_INET, SOCK_STREAM, 0);
	int localSocket  = socket(AF_INET, SOCK_STREAM, 0);

	if (remoteSocket == -1 || localSocket == -1) {
		perror("Unable to create socket");
		exit(1);
	}

	if (setsockopt(remoteSocket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1) {
		perror("Error estableciendo el socket remoto sin delay");
		exit(1);
	}
	if (setsockopt(localSocket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1) {
		perror("Error estableciendo el socket local sin delay");
		exit(1);
	}

	memset(&remoteAddr_in, 0, sizeof(remoteAddr_in));
	memset(&localAddr_in, 0, sizeof(localAddr_in));

	remoteAddr_in.sin_family = AF_INET;
	remoteAddr_in.sin_port = htons(REMOTE_PORT);

	if (inet_pton(AF_INET, REMOTE_IP, &remoteAddr_in.sin_addr) <= 0) {
		perror("Error creando IP servidor remoto");
		exit(1);
	}

	if (connect(remoteSocket, (struct sockaddr *)&remoteAddr_in, sizeof(remoteAddr_in)) == -1) {
		perror("Error conectándose al servidor remoto");
		exit(1);
	}

	time(&timevar);
	printf("Conectado al servidor REMOTO %s a las %s\n", REMOTE_IP, (char *)ctime(&timevar));

	localAddr_in.sin_family = AF_INET;
	localAddr_in.sin_port = htons(LOCAL_PORT);
	if (inet_pton(AF_INET, LOCAL_IP, &localAddr_in.sin_addr) <= 0) {
		perror("Error creando IP servidor local");
		exit(1);
	}

	if (connect(localSocket, (struct sockaddr *)&localAddr_in, sizeof(localAddr_in)) == -1) {
		perror("Error conectándose al servidor local");
		exit(1);
	}

	printf("Conectado al servidor LOCAL %s:%d\n", LOCAL_IP, LOCAL_PORT);
	if (pthread_create(&hilo_Raton, NULL, enviarPosRaton, (void*)(intptr_t)remoteSocket) != 0) {
		perror("Error creando hilo de ratón");
		exit(2);
	}
	while (1) {
		if (recv(remoteSocket, &vibId, sizeof(char), 0) < 0) {
			perror("Error o cierre en recv remoto");
			exit(-1);
		}
		printf("%c\n", vibId);
		if (send(localSocket,&vibId,sizeof(char), 0) < 0) {
			perror("Error o cierre en send local");
			break;
		} 
	}

	pthread_join(hilo_Raton, NULL);
	time(&timevar);
	printf("\nEjecución terminada a las: %s", (char *)ctime(&timevar));
	close(remoteSocket);
	close(localSocket);
	return 0;
}

void * enviarPosRaton(void * arg) {
	int remoteSocket =  (int)(intptr_t)arg;
	struct input_event ev;
	int dx = 0, dy = 0;
	char bufCoord[BUFSIZEC];
	int fd = open(MOUSE_DEV, O_RDONLY);

	if (fd == -1) {
		perror("Error al abrir el dispositivo de ratón.");
		return NULL;
	}
	printf("Leyendo desplazamientos del ratón...\n");
	while (1) {
		if (read(fd, &ev, sizeof(struct input_event)) < (ssize_t)sizeof(struct input_event)) {
			perror("Error al leer evento del ratón");
			close(fd);
			return NULL;
		}

		if (ev.type == EV_REL) {
			if (ev.code == REL_X) {
				dx += ev.value;
			} else if (ev.code == REL_Y) {
				dy += ev.value;
			}

			snprintf(bufCoord, BUFSIZEC, "%d/%d\n", dx, dy);
			send(remoteSocket, bufCoord, strlen(bufCoord), 0);
		}
	}

	close(fd);
	return NULL;
}
