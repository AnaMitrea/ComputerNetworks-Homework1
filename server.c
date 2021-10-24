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
                    if(logat == 0)
                    {
                        printf("[S] Eroare: Userul nelogat vrea sa se delogheze! \n");
                    }
                    else
                    {
                        logat = 0;
                        printf("[S] User-ul a fost delogat \n");
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


                        if(logat == 1)
                        {
                            printf("[S] Userul \"%s\" este deja logat! \n", user);

                            // daca sunt logat deja, inseamna ca nu ma mai loghez din nou.
                            
                            
                        }
                        else
                        {
                            //inseamna ca nu sunt logat => caut daca exista username

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
                                        break;
                                    }
                                }
                                if(logat == 0)
                                    if(strlen(user) > 0)
                                        printf("Userul \"%s\" nu a fost gasit in lista! \n",user);
                                    else
                                        printf("Introduceti un username valabil \n");
                            }
                        }
                    }
                    else
                    {
                        printf("[S] Comanda introdusa gresit! Introduceti din nou comanda \n");
                    }
                }  
            }  
        }
    } while (num1 > 0);
}