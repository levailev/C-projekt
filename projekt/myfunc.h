#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 80
#define S_SIZE 100
#define MAX_SECOND 1
char buffer[1024];

typedef struct
	{
		unsigned char RGB[3];
	}RGB;

int Post(char *neptunID, char *message, int NumCh){
    
    NumCh = NumCh + 6*sizeof(neptunID);
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("193.6.135.162");
    server.sin_port = htons(PORT);
    server.sin_family = AF_INET;
    char tmp[S_SIZE];
    int bytes = 0;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    char on = 1;
    
    if(sock < 0){
        fprintf(stderr, "Hiba történt a socket létrehozása közben!\n");
        exit(2);
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

    int connect_error = connect(sock, (struct sockaddr*)&server, sizeof(server));
    if(connect_error < 0){
        fprintf(stderr, RED "Kapcsolat létrehozása közben hiba történt!\n" WHT);
        exit(3);
    }
    
    strcat(buffer, "POST /~vargai/post.php HTTP/1.1\r\n");
    strcat(buffer, "Host: irh.inf.unideb.hu\r\n");
    strcat(buffer, "Content-Length: ");sprintf(tmp, "%d", NumCh);strcat(buffer, tmp);
    strcat(buffer, "\r\n");
    strcat(buffer, "Content-Type: application/x-www-form-urlencoded\r\n\r\n");
    strcat(buffer, "NeptunID=");strcat(buffer, neptunID);
    strcat(buffer, "&PostedText=");
    strcat(buffer, message);
    strcat(buffer, "\r\n");
    
   bytes = send(sock, buffer, strlen(buffer)+1, 0);
   if ( bytes <= 0 ) {
      fprintf(stderr,RED "Az adat küldése során hiba történt.\n" WHT);
      exit(4);
      }
   
   bytes = recv(sock, buffer, 1024, 0);
   if ( bytes < 0 ) {
      fprintf(stderr,RED "Az válasz fogadása során hiba történt.\n" WHT);
      exit(5);
   }

    
    close(sock);
    return 0;
}


char* Unwrap(char* Pbuff, int NumCh){
    
    unsigned char* Data;

    Data = (char*)malloc(sizeof(char)*NumCh);
    if(Data == NULL){
        fprintf(stderr,BRED "Hiba a memóriaterulet lefogalalsa kozben!" WHT);
        exit(1);
    }
    char tmp[3];
    int k = 0;
    #pragma omp paralell for
    for(int i = 0; i < 3*NumCh; i += 3)
    {
        tmp[0] = (Pbuff[i+2] & 3) << 6;
        tmp[1] = (Pbuff[i+1] & 7) << 3;
        tmp[2] = (Pbuff[i] & 7);
        
        char full = tmp[0] | tmp[1] | tmp[2];
         
        if(full < 0){
            char asci[9]  = {'a','e','o','o','o','u','u','u','i'};
            char easci[9] = {-95,-87,-74,-77,-111,-70,-79,-68,-83};
            for(int j = 0;j<9;j++){
                if(easci[j] == full){
                    full = asci[j];
                    Data[k] = full;
                    break;
                }
            }
        }
        else Data[k] = full;
        
        if(!(Data[k] < 127 && Data[k] > 31 || Data[k] < 91 && Data[k] > 64 || Data[k] == ' ' || Data[k] == '.' || Data[k] == ',' || Data[k] == '!')){
            Data[k] = Data[k] << 8;
            continue;
        }
        k++;
    }
    

    alarm(0);
    return Data;
}
void WhatToDo(int sig){
    
    pid_t pid;
    switch (sig){
        case SIGINT:
            if((pid = fork()) == 0){
                printf(BYEL "\nA dekódolást a CTRL+C kombinációval nem tudod megszakítani!\n" WHT);
                kill(getpid(), SIGKILL);
            }
            break;
        case SIGALRM:
            fprintf(stderr, RED "A program túl sokáig futott!\n" WHT);
            exit(6);
            break;
    }
}
char* ReadPixels(int f, int* NumCh)
{  
    char info[54];
    read(f, info, 54*sizeof(char));    
    int tmp = *(int*)&info[6];

    *NumCh = tmp;
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    int size = 3 * width * height;
    unsigned char* data;
    data = (char*)malloc(size);
    if(data == NULL){
        fprintf(stderr, RED "Hiba a memóriaterulet lefogalalsa közben!\n" WHT);
        exit(7);
    }
    
    read(f, data, *NumCh*3);
    
    for(int i = 0; i < size; i += 3)
    {
            unsigned char tmp = data[i];
            data[i] = data[i+2];
            data[i+2] = tmp;
    }  
    close(f);
    return data;
    
}

char* TestArray(int *NumCh){
   
    unsigned char* Data;
    Data = (char*)malloc(9*sizeof(char));

    srand(time(NULL));
    unsigned int RGB[3][3];
    unsigned int mask = 255;
    mask = (mask << 3) & 255 ; //11111000

    for(int j = 0; j < 3 ; j++){
        for(int i = 0; i < 3 ; i++){
                RGB[i][j] = rand()%(255-0+1)+0;
            }
    }
    unsigned char ch;
    for (unsigned char j = 97; j < 100; j++ ){

        ch = j;
        for(int i = 0; i < 3 ; i++){
            if(i == 0)
            {
                ch = ch & 7;
                RGB[j-97][i] = RGB[j-97][i] & mask;
                RGB[j-97][i] = RGB[j-97][i] + ch;
                ch = j;
            }
            if(i == 1)
            {
                ch = (ch >> 3) & 7;
                RGB[j-97][i] =  RGB[j-97][i] & mask;
                RGB[j-97][i] =  RGB[j-97][i] + ch;
                ch = j;
            }
            if(i == 2)
            {
                ch =  (ch >> 6) & 3;
                RGB[j-97][i] = RGB[j-97][i] & (mask + 4);
                RGB[j-97][i] = RGB[j-97][i] + ch;
                ch = j;
            }
        }
    }
    int sor = sizeof(Data) / sizeof(Data[0]);
    int oszlop = sizeof(Data[0])/sor;
    *NumCh = oszlop;
    return Data;
}

int BrowseForOpen(){
    //printf("\n");
    DIR* dir;
    struct dirent* entry;
    struct stat inode;
    char tmp[S_SIZE];
    int f = 0;
    
    while(1){
        dir = opendir(getenv("HOME")); // get env "home"
        printf( UWHT "\n\tAdj meg egy fájl/mappa nevét!\n\n" WHT);
        while((entry = readdir(dir)) != NULL){
            stat((*entry).d_name, &inode);
            switch(inode.st_mode & S_IFMT){
                case S_IFDIR:
                    printf("%s%s%s\t",MAG,(*entry).d_name, WHT);
                    break;
                case S_IFREG: 
                    printf("%s%s%s\t",CYN,(*entry).d_name, WHT);
                    break;
            }
        }
        
        puts("");
        scanf("%s", tmp);
        dir = opendir(".");
        while((entry = readdir(dir)) != NULL){
            stat((*entry).d_name, &inode);

            if(((inode.st_mode & S_IFMT) == S_IFREG)){
                if(!(strcmp((*entry).d_name, tmp))){
                    alarm(1);
                    return open(tmp, O_RDONLY);
                }
            }
        }
        if(chdir(tmp) == -1) printf(BHRED "\tHiba! Nem létező állomány!\n" WHT);
        dir = opendir(".");
        if(dir != NULL) {
            closedir(dir);
        }            
    }
}

