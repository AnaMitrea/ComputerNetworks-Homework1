#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_CS "C-S_FIFO"
#define FIFO_SC "S-C_FIFO"


int main()
{
    char cs[300];  // ce comanda se scrie catre server
    char sc[300];  // ce raspuns se scrie catre client
    int num1;
    int num2;
    int fd1;  // fd pt C-S_FIFO
    int fd2;  // fd pt S-C_FIFO

    fd1 = open(FIFO_CS, O_WRONLY);  // se deschide fifo Cient-Server in modul scriere
    fd2 = open(FIFO_SC, O_RDONLY);  // se deschide fifo SERVER-CLIENT in modul citire
    printf("[C] Introduceti comanda...\n");
    printf("[C] 1) login : username     [Atentie: Limitare lungime username de 30 de caractere!] \n");
    printf("[C] 2) get-proc-info \n");
    printf("[C] 3) get-logged-users \n");
    printf("[C] 4) logout \n");
    printf("[C] 5) quit \n\n");

    while (gets(cs), !feof(stdin)) 
    {
        // QUIT -oprirea directa a programului
        if(strcmp(cs, "quit") == 0)  // introducere comanda quit
        {
            if ((num1 = write(fd1, "quit", strlen("quit"))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                perror("[C] Problema la scriere in FIFO! \n");
            return 0;
        }

        /* Trimitere comanda catre server*/
        int cs_length = strlen(cs);  //lungimea comenzii ce se va trimite
        char char_length[3];  // lungimea comenzii scrisa ca sir de ch
        sprintf(char_length,"%d",cs_length);  // conversia int->char array
        
        //trimit cati bytes are comanda scrisa de user pt ca serverul sa ii citeasca
        if ((num1 = write(fd1, char_length, strlen(char_length))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
            perror("[C] Problema la scriere in FIFO! \n");
        
        sleep(1);   // astept ca serverul sa primeasca lungimea comenzii
        
        //trimit comanda
        if ((num1 = write(fd1, cs, strlen(cs))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
            perror("[C] Problema la scriere in FIFO! \n");


        /* Primire raspuns de la server*/

        if ((num2 = read(fd2, sc, 3)) == -1)  // se citeste ce s-a scris in fifo si num contine cati bytes s-au citit
            perror("[C] Eroare la citirea din FIFO!");
        else
        {
            sc[num2] = '\0';  //am primit cati bytes trebuie sa citeasca clientul
            int nr_bytes;
            sscanf(sc, "%d", &nr_bytes);  //conversie char array to int

            if ((num2 = read(fd2, sc, nr_bytes)) == -1)  // se citeste ce comanda s-a scris in fifo si num contine cati bytes s-au citit
                perror("[C] Eroare la citirea din FIFO!");
            else
            {
                sc[num2] = '\0';
                printf("[C] %s \n", sc);
            }
        }
    }
}

/*

--------------------------------
if ((num = write(fd1, cs, strlen(cs))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
    perror("[C] Problema la scriere in FIFO!");
else
    printf("[C->S] S-au scris in FIFO %d bytes\n", num);
-------------------------------
*/