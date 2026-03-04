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

#define BUFSIZE       256
#define REMOTE_PORT   45554
#define LOCAL_PORT    45454
#define REMOTE_IP     "192.168.1.40"
#define REMOTE_IP2    "212.128.171.68"
#define LOCAL_IP      "127.0.0.1"
#define MOUSE_DEV     "/dev/input/event4"

void * enviarPosRaton(void * arg);

int main() {
  char buf[1];
  pthread_t hilo_Raton;
  long timevar;
  int opt = 1;
  int addrlen = sizeof(struct sockaddr_in);

  int sockRaton  = socket(AF_INET, SOCK_STREAM, 0);
  int sockLocal  = socket(AF_INET, SOCK_STREAM, 0);

  if (sockRaton == -1 || sockLocal == -1) {
    perror("Unable to create socket");
    exit(1);
  }

  if (setsockopt(sockRaton, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1) {
    perror("Error estableciendo el socket remoto sin delay");
    exit(1);
  }
  if (setsockopt(sockLocal, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1) {
    perror("Error estableciendo el socket local sin delay");
    exit(1);
  }

  struct sockaddr_in ratonaddr_in, localaddr_in, vibraraddr_in;
  memset(&ratonaddr_in, 0, sizeof(ratonaddr_in));
  memset(&localaddr_in, 0, sizeof(localaddr_in));
  memset(&localaddr_in, 0, sizeof(vibraraddr_in));

  ratonaddr_in.sin_family = AF_INET;
  ratonaddr_in.sin_port = htons(REMOTE_PORT);
  if (inet_pton(AF_INET, REMOTE_IP, &ratonaddr_in.sin_addr) <= 0) {
    perror("Error creando IP servidor remoto");
    exit(1);
  }

  if (connect(sockRaton, (struct sockaddr *)&ratonaddr_in, sizeof(ratonaddr_in)) == -1) {
    perror("Error conectándose al servidor remoto");
    exit(1);
  }

  if (getsockname(sockRaton, (struct sockaddr *)&localaddr_in, &addrlen) == -1) {
    perror("Unable to read socket address remoto");
    exit(1);
  }

  time(&timevar);
  printf("Conectado al servidor REMOTO %s desde puerto local %u a las %s",
	 REMOTE_IP, ntohs(localaddr_in.sin_port), (char *)ctime(&timevar));

  vibraraddr_in.sin_family = AF_INET;
  vibraraddr_in.sin_port = htons(LOCAL_PORT);
  if (inet_pton(AF_INET, LOCAL_IP, &vibraraddr_in.sin_addr) <= 0) {
    perror("Error creando IP servidor local");
    exit(1);
  }

  if (connect(sockLocal, (struct sockaddr *)&vibraraddr_in, sizeof(vibraraddr_in)) == -1) {
    perror("Error conectándose al servidor local");
    exit(1);
  }

  printf("Conectado al servidor LOCAL %s:%d\n", LOCAL_IP, LOCAL_PORT);

  if (pthread_create(&hilo_Raton, NULL, enviarPosRaton, (void*)(intptr_t)sockRaton) != 0) {
    perror("Error creando hilo de ratón");
    return 1;
  }

  while (1) {
    if (recv(sockRaton, buf, sizeof(buf), 0) <= 0) {
      perror("Error o cierre en recv remoto");
      break;
    }
    printf("Recibido del remoto: %c\n", buf[0]);
    if (send(sockLocal,buf,sizeof(buf), 0) <=0) {
      perror("Error o cierre en send local");
      break;
    }        
  }

    pthread_join(hilo_Raton, NULL);

    time(&timevar);
    printf("\nEjecución terminada a las: %s", (char *)ctime(&timevar));
    close(sockRaton);
    close(sockLocal);
    return 0;
  }

  void * enviarPosRaton(void * arg) {
    int sockRaton =  (int)(intptr_t)arg;

    struct input_event ev;
    int dx = 0, dy = 0;
    char buf[BUFSIZE];

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

	sprintf(buf, "%d/%d\n", dx, dy);
	printf("%s", buf);
	send(sockRaton, buf, strlen(buf), 0);
      }
    }

    close(fd);
    return NULL;
  }
