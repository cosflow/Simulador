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
#define NUMSOCK       2
#define IDREM         0
#define IDLOC         1
#define REMOTE_PORT   45554
#define LOCAL_PORT    45454
//#define REMOTE_IP    "192.168.1.40"
#define REMOTE_IP     "212.128.171.68"
#define LOCAL_IP      "127.0.0.1"
#define MOUSE_DEV     "/dev/input/event4" //cambiar por /dev/input/id/id-raton

void * enviarPosRaton(void * arg);
void die(char *msg){
	perror(msg);
	exit(1);
}

int main() {
	char bufCoord[BUFSIZEC];
	int vibId;
	pthread_t hilo_Raton;
	long timevar;
	int opt = 1, i;
	size_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in addrs[NUMSOCK];
	int sockets[NUMSOCK];
	int ports[NUMSOCK] ={REMOTE_PORT, LOCAL_PORT};
	for (int i = 0 ; i < NUMSOCK ; i++){
		sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (sockets[i] == -1) die("Unable to create socket");
		memset(&addrs[i], 0, sizeof(addrs[i]));
		if (setsockopt(sockets[i], IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1)
	       		die("Error estableciendo el socket remoto sin delay");
		addrs[i].sin_family = AF_INET;
		char * ip;
		int port;
		if (i == IDREM) {
			ip = REMOTE_IP;
			port = REMOTE_PORT;
		}
		else {
			ip = LOCAL_IP;
			port = LOCAL_PORT;
		}
		addrs[i].sin_port = htons(port);
		if (inet_pton(AF_INET, ip,  &addrs[i].sin_addr) <= 0)
			die("Error generando IP");
		if (connect(sockets[i], (struct sockaddr *)&addrs[i], addrlen) == -1)
			die("Error conectándose al servidor");
	}

	time(&timevar);
	printf("Conectado al servidor REMOTO %s a las %s\n", REMOTE_IP, (char *)ctime(&timevar));
	printf("Conectado al servidor LOCAL %s:%d\n", LOCAL_IP, LOCAL_PORT);
	if (pthread_create(&hilo_Raton, NULL, enviarPosRaton, (void*)(intptr_t)sockets[IDREM]) != 0)
		die("Error creando hilo de ratón");

	while (1) {
		if (recv(sockets[IDREM], &vibId, sizeof(vibId), 0) < 0)
			die("Error o cierre en recv remoto");
		int dec = ntohl(vibId);
		printf("%d\n", dec);
		if (send(sockets[IDLOC],&dec,sizeof(dec), 0) < 0)
			die("Error o cierre en send local");
	}
	
	pthread_join(hilo_Raton, NULL);
	time(&timevar);
	printf("\nEjecución terminada a las: %s", (char *)ctime(&timevar));
	for (int i = 0 ; i < NUMSOCK ; i++) close(sockets[i]);
	return 0;
}

void * enviarPosRaton(void * arg) {
	int s =  (int)(intptr_t)arg;
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
			close(fd);
			die("Error al leer evento del ratón");
		}

		if (ev.type == EV_REL) {
			if (ev.code == REL_X) dx += ev.value;
			else if (ev.code == REL_Y) dy += ev.value;
		}

		else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
			snprintf(bufCoord, BUFSIZEC, "%d/%d\n", dx, dy);
			send(s, bufCoord, strlen(bufCoord), 0);
			dx = 0;
			dy = 0;
		}
	}

	close(fd);
	return NULL;
}
