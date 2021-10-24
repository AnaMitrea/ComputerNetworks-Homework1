#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

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
    char *ptr;
    
    mknod(FIFO_CS, S_IFIFO | 0666, 0); // se creeaza fisierul fifo 1    client-->server
    mknod(FIFO_SC, S_IFIFO | 0666, 0); // se creeaza fisierul fifo 2    server-->client


    printf("[S] - SERVER DESCHIS -\n");
    fd1 = open(FIFO_CS, O_RDONLY); // se deschide fifo CLIENT-SERVER in modul citire
    fd2 = open(FIFO_SC, O_WRONLY); // se deschide fifo SERVER-CLIENT in modul scriere
    printf("[S] User conectat la server...\n");

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
// COMANDA GET-PROC-INFO : PID     -----------TREBUIE ADAUGAT PIPE---------------
                else
                if ((ptr = strstr(cs, "get-proc-info : ")) != NULL)   // get_proc_info : pid
                {
                    printf("[S] S-a apelat functia \"get_proc_info : <pid>\"!\n");
                    if(logat == 1)
                    {
                        char proc_dir[6] = "/proc";
                        char pid_value[10]; 
                        strcpy(pid_value, cs+16); //valoarea pidului
                        char path[20];

                        /*  
                            https://stackoverflow.com/questions/12510874/how-can-i-check-if-a-directory-exists
                            idee adaptata
                        */

                        DIR* dir = opendir(proc_dir);

                        if (dir) /* Directorul exista */
                        {
                            printf("[S] Directorul cu numele \"%s\" exista! \n", proc_dir);
                            printf("[S] S-a scris pid-ul \"%s\"!\n", pid_value);

                            //construire calea directorului /proc/<pid>
                            path[0] = '\0';
                            strcat(path,proc_dir);
                            strcat(path,"/");
                            strcat(path,pid_value);

                            printf("[S] Path-ul pidului in director este: \"%s\"  \n", path);        

                            //verific daca directorul /proc/<pid> exista
                            DIR* pid_dir = opendir(path);
                            if(pid_dir)
                            {
                                printf("[S] Directorul /proc/<pid> cu numele \"%s\" exista! \n", path);

                                char status_file[30]; //calea /proc/<pid>/status

                                status_file[0] = '\0';

                                strcat(status_file,path);
                                strcat(status_file,"/status");

                                printf("[S] Path-ul catre status este: \"%s\"  \n", status_file);

                                // cautare name, state, ppid, uid si vmsize in fisierul /proc/<pid>/status  :
                                FILE *fptr = fopen(status_file, "r");

                                if (fptr == NULL)
                                {
                                    printf("Eroare la deschiderea fisierului: %s \n", status_file);
                                    return 1;
                                }
                                
                                char line[300];
                                char information[300];
                                information[0]='\0';

                                // Cautare informatiei linie cu linie
                                while (fgets(line, 300, fptr))
                                {
                                    if(strncmp(line, "Name:", 5) == 0)
                                    {
                                        strcat(information, line);
                                    }
                                    else
                                    if(strncmp(line, "State:", 6) == 0)
                                    {
                                        strcat(information, line);
                                    }
                                    else
                                    if(strncmp(line, "PPid:", 5) == 0)
                                    {
                                        strcat(information, line);
                                    }
                                    else
                                    if(strncmp(line, "Uid:", 4) == 0)
                                    {
                                        strcat(information, line);
                                    }
                                    else
                                    if(strncmp(line, "VmSize:", 7) == 0)
                                    {
                                        strcat(information, line);
                                    }
                                }
                                
                                fclose(fptr);  // inchidere fisier /proc/<pid>/status

                                closedir(pid_dir);  //inchidere director /proc/<pid>

                                int lung = strlen(information);

                                //conversie int lung in char lung[]
                                char buf[3];
                                buf[0] = '\0';
                                
                                sprintf(buf, "%d", lung);

                                // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                                if ((num2 = write(fd2, buf, 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                                
                                sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                                
                                // trimit mesajul
                                if ((num2 = write(fd2, information, strlen(information))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                            }
                            else
                            if (ENOENT == errno) /* Directorul pid_dir nu exista */
                            { 
                                printf("[S] Directorul (cu pid) numele \"%s\" nu exista! \n", path);
                                
                                // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                                if ((num2 = write(fd2, "26", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                                
                                sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                                
                                // trimit mesajul
                                char msg[300] = "Pid-ul introdus nu exista!";
                                if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                            } 
                            else /* eroare la opendir()*/
                            {
                                printf("Eroare la deschiderea directorului (cu pid) \"%s\" \n", path);
                            }
                            closedir(dir);  //inchidere director /proc
                        }
                        else 
                        {   
                            if (ENOENT == errno) /* Directory does not exist. */
                            { 
                                printf("Directorul cu numele \"%s\" nu exista! \n", proc_dir);
                            } 
                            else /* opendir() failed for some other reason. */
                            {
                                printf("Eroare la deschiderea directorului \"%s\" \n", proc_dir);
                            }
                        }
                    }
                    else  //inseamna ca nu-s logat, deci nu pot folosi comanda
                    {
                       printf("[S] User-ul nelogat nu poate apela functia \"get_proc_info : pid\"\n");

                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                        if ((num2 = write(fd2, "47", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                        
                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                        
                        // trimit mesajul
                        char msg[300] = "Nu puteti apela functia daca nu sunteti logati!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                    }
                }
// COMANDA LOGOUT
                else
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
// COMANDA LOGIN   - !!!!!!!!!!!!!!!!!MODIFICA PENTRU CA EXECUTIA SA FIE FACUTA IN COPIL!!!!!!!!!!!!!!!!!!!!!!!!!!
                if((ptr = strstr(cs, "login : ")) != NULL)
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
                    
                        // folosesc socketuri aici !!!!!!!!!!!!!!!!!!!!!!!1



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
                else // comanda introdusa este gresita
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
    } while (num1 > 0);
}