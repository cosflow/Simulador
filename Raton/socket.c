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

#define SERVER "212.128.171.68"
#define PORT 45554
#define MOUSE_DEV "/dev/input/event6"
#define pyRuta "../Vibrar/vibrar.py"
#define BUFSIZE 256

void * enviarPosRaton(void * arg);

int main() {
  pthread_t hilo_Raton;
  pthread_t hilo_Vibrar;
  int addrlen=sizeof(struct sockaddr_in);
  long timevar;
  struct sockaddr_in serveraddr_in, mouseaddr_in;
  int opt=1;
  
  int s = socket (AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("Unable to create socket\n");
    exit(1);
  }
  if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY,&opt,sizeof(opt)) == -1) {
    perror("Error estableciendo el socket sin delay\n");
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

  if(pthread_create(&hilo_Raton, NULL, enviarPosRaton, (void*)(intptr_t)s) != 0){
      perror("Error creando hilo de ratón");
      return 1;
  }

  pthread_join(hilo_Raton,NULL);

  time(&timevar);
  printf("All done at %s", (char *)ctime(&timevar));
  close(s);
  return 0;
}

void * enviarPosRaton(void * arg){
  
  int s = (int)(intptr_t)arg;
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
    if (read(fd, &ev, sizeof(struct input_event)) < sizeof(struct input_event)) {
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
      sprintf(buf,"%d/%d\n",dx, dy);
      send(s,buf,strlen(buf)+1,0);
    }
  }

  close(fd);
  return NULL;
}

