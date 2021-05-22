#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "myfunc.h"
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>

int f;

int main( int argc, char *argv[])  {
    
        if(argc == 1){
            f = BrowseForOpen();
        }
        else{
            if((f = open(argv[1], O_RDONLY)) != -1 ){
                alarm(1);
            }
            else if(!strcmp("--version", argv[1])) {
                fprintf(stdout,BWHT "\t2.0\n\t2021.02.20\n\tLévai Levente\n" WHT);
                return 0;
            }
            else if(!strcmp("--help", argv[1])) {
                fprintf(stdout,BWHT "\t --version: Program verziószáma, elkészültének dátuma, fejlesztő neve.\n" WHT);
                fprintf(stdout,BWHT "\t --help: Tájékoztató a programhoz\n"WHT);
                fprintf(stdout,BWHT"\t Más esetben a megadott argumentumok közül az első kerül képfájlnévként feldolgozásra\n"WHT);
                return 0;
            }
            else{
                 fprintf(stderr,BRED "Hiba! Adj meg létező képfáljnevet vagy kapcsolót(--help)\n" WHT);
                 exit(7);
            }
        }
        
        char neptunID[6] = "FSCAS1";
        int count = 0;
        signal(SIGALRM, WhatToDo);
        signal(SIGINT, WhatToDo);
        char* pixels = ReadPixels(f ,&count);
        char* decoded = Unwrap(pixels, count);
        
        //if(!(Post(neptunID, decoded, count))) printf(BGRN"\n\tA szöveg sikeresen elküldve!\n\n"WHT);
        printf(BGRN "%s\n" WHT, decoded);
        free(decoded);
        return 0;
}
