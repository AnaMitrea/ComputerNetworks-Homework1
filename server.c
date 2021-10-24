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
#define USERS_LIST "usersDB.txt"

int main()
{
    char cs[300];  // ce se scrie catre server
    char sc[300];  // ce se scrie catre client
    int num1;
    int num2;
    int fd1;  // fd pt C-S_FIFO
    int fd2;  // fd pt S-C_FIFO
    int logat = 0;   // 0 nu sunt logat, 1 sunt logat deja
    
    mknod(FIFO_CS, S_IFIFO | 0666, 0); // se creeaza fisierul fifo 1    client-->server
    mknod(FIFO_SC, S_IFIFO | 0666, 0); // se creeaza fisierul fifo 2    server-->client


    printf("[S] - SERVER DESCHIS -\n");
    fd1 = open(FIFO_CS, O_RDONLY); // se deschide fifo CLIENT-SERVER in modul citire
    fd2 = open(FIFO_SC, O_WRONLY); // se deschide fifo SERVER-CLIENT in modul scriere
    printf("[S] User conectat...\n");

    do {
        if ((num1 = read(fd1, cs, 3)) == -1)  // se citeste ce s-a scris in fifo si num contine cati bytes s-au citit
            perror("[S] Eroare la citirea din FIFO!");
        else
        {
            cs[num1] = '\0';  //am primit cati bytes trebuie sa citeasca serverul
            int nr_bytes;
            sscanf(cs, "%d", &nr_bytes);  //conversie char array to int

            if ((num1 = read(fd1, cs, nr_bytes)) == -1)  // se citeste ce comanda s-a scris in fifo si num contine cati bytes s-au citit
                perror("[S] Eroare la citirea din FIFO!");
            else
            {
                cs[num1] = '\0';

                /* PRELUCRARE COMENZI PRIMITE DE LA CLIENT : */

                // COMANDA QUIT
                if(strcmp(cs,"quit") == 0)
                {
                    printf("[S] S-a apelat functia \"quit\"! \n");
                    return 0;
                }
                else
                // COMANDA LOGOUT
                if(strcmp(cs,"logout") == 0)
                {
                    printf("[S] S-a apelat functia \"logout\"! \n");
                    if(logat == 0)  //nu e logat
                    {
                        printf("[S] Eroare: Userul nelogat vrea sa se delogheze! \n");
                      
                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                        if ((num2 = write(fd2, "54", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                        
                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                        
                        // trimit mesajul
                        char msg[300] = "Trebuie sa fiti logati pentru a folosi comanda logout!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                    }
                    else
                    {
                        logat = 0;
                        printf("[S] User-ul a fost delogat \n");
                        
                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                        if ((num2 = write(fd2, "30", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                        
                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                        
                        //trimit mesajul
                        char msg[300] = "Ati fost delogat de pe server!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                    }
                }
                else
                {
                    // COMANDA LOGIN
                    char* ptr = strstr(cs, "login : ");
                    if(ptr != NULL)  //daca comanda e corecta
                    {
                        printf("[S] ---Apelare functie \"login : username\"!--- \n");

                        // aflare username
                        int userlength = strlen(cs) - 8 + 1;
                        char user[userlength];
                        strcpy(user,cs + 8);

                        if(logat == 1) // daca sunt logat deja, inseamna ca nu ma mai loghez din nou.
                        {
                            printf("[S] Userul \"%s\" este deja logat! \n", user);

                            // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                            if ((num2 = write(fd2, "30", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                perror("[S] Problema la scriere in FIFO! \n");
                            
                            sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                            
                            // trimit mesajul
                            char msg[300] = "Sunteti deja logati pe server!";
                            if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                perror("[S] Problema la scriere in FIFO! \n");                            
                        }
                        else //inseamna ca nu sunt logat => caut daca exista username
                        {
                            // prelucrare username
                            FILE* fis = fopen(USERS_LIST,"r"); // deschid lista de useri in modul read pt a face cautarea
                            if(fis == NULL)  //eroare open fisier
                            {
                                printf("[S] Problema la deschiderea fisierului cu usernames! \n");
                            }
                            else
                            {
                                printf("[S] Verificare username... \n");

                                const unsigned int maxlength = 30;  //maxim 30 de caractere pe fiecare linie
                                char name[maxlength];

                                while(fgets(name, maxlength, fis)) //citire linie cu linie din fisier 
                                {
                                    // caut username-ul in fisier
                                    name[strlen(name) - 1] = '\0';
                                    if(strcmp(name,user) == 0) // exista username ul in fisier 
                                    {
                                        printf("[S] User-ul \"%s\" s-a conectat la server! \n", name);
                                        logat = 1;
                                        fclose(fis);

                                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                                        if ((num2 = write(fd2, "22", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                            perror("[S] Problema la scriere in FIFO! \n");
                                        
                                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                                        
                                        // trimit mesajul
                                        char msg[300] = "V-ati logat cu succes!";
                                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                            perror("[S] Problema la scriere in FIFO! \n");  
                                        break;
                                    }
                                }
                                if(logat == 0)
                                {
                                    if(strlen(user) > 0) // user valid dar nu exista in lista
                                    {
                                        printf("[S] Userul \"%s\" nu a fost gasit in lista! \n",user);

                                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                                        if ((num2 = write(fd2, "28", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                            perror("[S] Problema la scriere in FIFO! \n");
                                        
                                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                                        
                                        // trimit mesajul
                                        char msg[300] = "Username-ul nu a fost gasit!";
                                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                            perror("[S] Problema la scriere in FIFO! \n"); 
                                    }
                                    else
                                    if(strlen(user) > 30 || strlen(user) <= 0) // username invalid
                                    {
                                        printf("[S] User-ul a introdus un username invalid \n");

                                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                                        if ((num2 = write(fd2, "116", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                            perror("[S] Problema la scriere in FIFO! \n");
                                        
                                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                                        
                                        // trimit mesajul
                                        char msg[300] = "Ati introdus un username invalid! Lungimea admisa pentru username este de minim 1 caracter si maxim 30 de caractere.";
                                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                            perror("[S] Problema la scriere in FIFO! \n"); 
                                    }
                                }     
                            }
                        }
                    }
                    else // comanda gresita
                    {
                        printf("[S] Comanda introdusa gresit. \n");

                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                        if ((num2 = write(fd2, "32", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                        
                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                        
                        // trimit mesajul
                        char msg[300] = "Ati introdus o comanda invalida!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n"); 
                    }
                }  
            }  
        }
    } while (num1 > 0);
}