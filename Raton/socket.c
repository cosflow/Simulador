#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
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
#define REMOTE_IP     "212.128.171.68"
#define LOCAL_IP      "127.0.0.1"
#define MOUSE_DEV     "/dev/input/event4"

void * enviarPosRaton(void * arg);
void die(char *msg){
	perror(msg);
	exit(1);
}

int main() {
	signal(SIGPIPE, SIG_IGN);
	pthread_t hilo_Raton;
	int opt = 1;
	size_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in addrs[NUMSOCK];
	int sockets[NUMSOCK];
	const char *ips[] = {REMOTE_IP, LOCAL_IP};
	const int ports[] = {REMOTE_PORT, LOCAL_PORT};
	for (int i = 0 ; i < NUMSOCK ; i++){
		sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (sockets[i] == -1) die("Unable to create socket");
		memset(&addrs[i], 0, sizeof(addrs[i]));
		if (setsockopt(sockets[i], IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1)
	       	die("Error estableciendo socket sin delay");
		addrs[i].sin_family = AF_INET;
		addrs[i].sin_port = htons(ports[i]);
		if (inet_pton(AF_INET, ips[i],  &addrs[i].sin_addr) <= 0)
			die("Error generando IP");
		if (connect(sockets[i], (struct sockaddr *)&addrs[i], addrlen) == -1)
			die("Error conectándose al servidor");
	}

	printf("Conectados\n");
	/*
	if (pthread_create(&hilo_Raton, NULL, enviarPosRaton, (void*)(intptr_t)sockets[IDREM]) != 0)
		die("Error creando hilo de ratón");
*/	
	int msg;
	while (recv(sockets[IDREM], &msg, sizeof(msg), 0) > 0) {
		printf("%d\n", ntohl(msg));
		if (send(sockets[IDLOC],&msg,sizeof(msg), 0) <= 0)
			die("Error o cierre en send local");
	}
	
//	pthread_join(hilo_Raton, NULL);
	printf("\nEjecución terminada\n");
	for (int i = 0 ; i < NUMSOCK ; i++) if (sockets[i] >= 0) close(sockets[i]);
	return 0;
}

void * enviarPosRaton(void * arg) {
	int s =  (int)(intptr_t)arg;
	struct input_event ev;
	int dx = 0, dy = 0;
	char bufCoord[BUFSIZEC];
	int fd = open(MOUSE_DEV, O_RDONLY);

	if (fd == -1) die("Error al abrir el dispositivo de ratón.");
	printf("Leyendo desplazamientos del ratón...\n");
	while (!(read(fd, &ev, sizeof(struct input_event)) < (ssize_t)sizeof(struct input_event))) {
		if (ev.type == EV_REL) {
			if (ev.code == REL_X) dx += ev.value;
			else if (ev.code == REL_Y) dy += ev.value;
		}
		else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
			if (dx || dy) {
				snprintf(bufCoord, BUFSIZEC, "%d/%d\n", dx, dy);
				if (send(s, bufCoord, strlen(bufCoord), 0) <= 0) die("Error send raton");
				dx = 0;
				dy = 0;
			}
		}
	}
	close(fd);
	return NULL;
}
