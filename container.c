#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <sched.h>

int main(int argc, char * argv[]) {
  printf("nb argument : %d\n", argc);
  pid_t retourFork;
  int tabUnshare[5] = {
    CLONE_NEWPID,
    CLONE_NEWNET,
    CLONE_NEWUTS,
    CLONE_FS,
    CLONE_NEWNS
  };
  //unshare new pid 
  if (unshare(tabUnshare[0]) < 0) {
    perror("Erreur unshare");
    exit(EXIT_FAILURE);
  }
  //fork
  retourFork = fork();
  switch (retourFork) {
  case -1:
    perror("Erreur du fork");
    exit(EXIT_FAILURE);
  case 0:
    printf("le fils unshare %d\n", getpid());
    mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL);
    //unshare ns 
    if (unshare(tabUnshare[4]) < 0) {
      perror("Erreur unshare ns");
      exit(EXIT_FAILURE);
    }
    mount("none", "/tmp", "ext4", MS_BIND, NULL);
 
    mount("none", "/media", "ext4", MS_BIND, NULL);
    
   
  
    //unshare net
    if (unshare(tabUnshare[1]) < 0) {
      perror("Erreur unshare proc net");
      exit(EXIT_FAILURE);
    }
    //usnhare uts
    if (unshare(tabUnshare[2]) < 0) {
      perror("Erreur unshare proc uts");
      exit(EXIT_FAILURE);
    }
    //unshare fs 
    if (unshare(tabUnshare[3]) < 0) {
      perror("Erreur du unshare fs");
      exit(EXIT_FAILURE);
    }
    //montage du dossier proc
    if (mount("proc", "proc", "proc", 0, NULL) < 0) {
      perror("Erreur du mount proc");
      exit(EXIT_FAILURE);
    }

    //chroot
    if (chroot("./") == -1) {
      perror("erreur chroot");
      exit(EXIT_FAILURE);
    }

    char destination[20] = "/bin/";

    char * source;
    //si aucun de parametre alors on exexute le bash par defaut sinon la commande passer en parametere
    if (argv[1] == NULL) {
      printf("ça passe par là !!!\n");
      source = "bash";
    } else {
      printf("ça passe par ici !!!\n");
      source = argv[1];
    }
    strcat(destination, source);
   

    if (execlp(destination, destination, NULL) < 0) {
      perror("erreur de la commande execlp");
      exit(EXIT_FAILURE);
    }

    break;

  default:
    printf("le pere pid=%d et fils pid=%d \n", getpid(), retourFork);
    if (wait(NULL) < 0) {
      perror("Erreur du wait");
      exit(EXIT_FAILURE);
    }
    printf("fin du conteneur");
  }

  return (0);
}

