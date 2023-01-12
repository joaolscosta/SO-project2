#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../fs/operations.h"
#include "../producer-consumer/producer-consumer.h"
#include "../utils/logging.h"
#include "../utils/utils.h"

sem_t sessionsSem;
pc_queue_t queue;

int registerPub(const char* pipeName, char* boxName) {
  int boxFd = tfs_open(boxName, TFS_O_APPEND | TFS_O_CREAT);
  if ((boxFd) < 0) {
    perror("Error while opening box");
    exit(EXIT_FAILURE);
  }

  // TODO: Verify if the box exists and if there are any other publishers
  if (0) {
    exit(EXIT_FAILURE);
  }

  unlink(pipeName);

  if (mkfifo(pipeName, 0777) < 0) {
    perror("Error while creating fifo");
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "%s\n", pipeName);
  int sessionFd;
  if ((sessionFd = open(pipeName, O_RDONLY)) < 0) {
    perror("Error while opening fifo");
    exit(EXIT_FAILURE);
  }

  char message[MESSAGE_SIZE];
  ssize_t n;

  while ((n = read(sessionFd, message, MESSAGE_SIZE)) > 0) {
    fprintf(stdout, "%s", message);
    //* Reads what is in the fifo

    if (tfs_write(boxFd, message, MESSAGE_SIZE) <= 0) {
      //* Writes it to the file System
      perror("Error while writing in fifo");
      exit(EXIT_FAILURE);
    }

    sleep(1);  //! espera ativa :D
  }

  memset(message, 0, MESSAGE_SIZE);

  tfs_read(boxFd, message, MESSAGE_SIZE);

  printf("%s", message);
  puts("hey");

  return 0;
}

int registerSub(const char* pipeName, const char* boxName) {
  int boxFd = tfs_open(boxName, TFS_O_TRUNC);
  if (boxFd < 0) {
    perror("Error while creating/opening box");
    exit(EXIT_FAILURE);
  }

  if (mkfifo(pipeName, 0640) < 0) {
    perror("Error while creating fifo");
    exit(EXIT_FAILURE);
  }

  int sessionFd;
  if ((sessionFd = open(pipeName, O_WRONLY)) < 0) {
    perror("Error while opening fifo");
    exit(EXIT_FAILURE);
  }

  char message[MESSAGE_SIZE];
  ssize_t n;

  if (tfs_read(boxFd, message, MESSAGE_SIZE) > 0) {
    //* Sends byte Code
    if (write(sessionFd, "10|", 3) < 0) {
      //* Writes it to fifo
      perror("Error while writing in fifo");
      exit(EXIT_FAILURE);
    }
    if (write(sessionFd, message, MESSAGE_SIZE) < 0) {
      //* Writes it to fifo
      perror("Error while writing in fifo");
      exit(EXIT_FAILURE);
    }
  }

  while ((n = tfs_read(boxFd, message, MESSAGE_SIZE)) > 0) {
    //* Reads what is in the file

    if (write(sessionFd, message, MESSAGE_SIZE) < 0) {
      //* Writes it to fifo
      perror("Error while writing in fifo");
      exit(EXIT_FAILURE);
    }

    // sleep(1);  //! espera ativa :D
  }

  close(boxFd);
  close(sessionFd);

  return 0;
}

int createBox(const char* pipeName, const char* boxName) {
  (void)pipeName;
  (void)boxName;
  // TODO: Implement Me
  return -1;
}

int destroyBox(const char* pipeName, const char* boxName) {
  (void)pipeName;
  (void)boxName;
  // TODO: Implement Me
  return -1;
}

int listBoxes() {
  // TODO: Implement Me
  fprintf(stdout, "Isto é uma caixa que levou print uwu");
  return -1;
}

//? Codigo daqui para cima talvez possa ser colocado noutro ficheiro :D

int main(int argc, char** argv) {
  // expected argv:
  // 0 - nome do programa
  // 1 - pipename
  // 2 - max sessions

  if (argc != 3) {
    fprintf(stderr, "usage: mbroker <pipeName> <maxSessions>\n");
    exit(EXIT_FAILURE);
  }

  const char* pipeName = argv[1];
  const size_t maxSessions = (size_t)atoi(argv[2]);
  char buf[MAX_BLOCK_LEN];

  pcq_create(&queue, maxSessions);

  if (sem_init(&sessionsSem, 0, maxSessions) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  // sem_wait(&sessionsSem); //! Usar isto quando não houverem mais sessões

  tfs_init(NULL);

  if (unlink(pipeName) < 0) {
    tfs_destroy();
    exit(EXIT_FAILURE);
  }

  if (mkfifo(pipeName, 0640) < 0) {
    tfs_destroy();
    exit(EXIT_FAILURE);
  }

  int fd = open(pipeName, TFS_O_TRUNC);
  if (fd < 0) {
    tfs_destroy();
    exit(EXIT_FAILURE);
  }

  while (1) {
    // This loop reads the pipe, always expecting new messages

    if ((read(fd, buf, MAX_FILE_NAME)) != 0) {
      //* recebeu uma mensagem
      char* ptr = strtok(buf, "|");
      short code = (short)atoi(ptr);
      char* clientNamedPipe = strtok(NULL, "|");
      char* boxName;

      switch (code) {
        case 1:
          boxName = strtok(NULL, "|");
          registerPub(clientNamedPipe, boxName);
          break;

        case 2:
          boxName = strtok(NULL, "|");
          registerSub(clientNamedPipe, boxName);
          break;

        case 3:
          boxName = strtok(NULL, "|");
          createBox(clientNamedPipe, boxName);
          break;

        case 4:
          //* resposta
          break;

        case 5:
          boxName = strtok(NULL, "|");
          destroyBox(clientNamedPipe, boxName);
          break;

        case 6:
          //* resposta
          break;

        case 7:
          listBoxes();
          break;

        case 8:
          //* resposta
          break;

        case 9:

          break;
        default:
          break;
      }
    }
    sleep(1);  //! Ta em espera ativa aqui uwu
  }

  pcq_destroy(&queue);

  tfs_destroy();

  close(fd);

  return EXIT_FAILURE;
}
