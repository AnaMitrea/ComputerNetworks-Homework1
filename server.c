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
#include <sys/socket.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <time.h>


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

                    if ((num2 = write(fd2, "48", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                        
                    sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                    
                    // trimit mesajul
                    if ((num2 = write(fd2, "Aplicatia a fost inchisa folosind comanda quit", strlen("Aplicatia a fost inchisa folosind comanda quit"))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                        perror("[S] Problema la scriere in FIFO! \n");

                    return 0;
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
                        char msg[] = "Trebuie sa fiti logati pentru a folosi comanda logout!";
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
                        char msg[] = "Ati fost delogat de pe server!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                    }
                }
                else
// COMANDA LOGIN -> comunicare tata-copil prin socket
                if((ptr = strstr(cs, "login : ")) != NULL)
                {
                    printf("[S] S-a apelat functia \"login : username\"! \n");

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
                        char msg[] = "Sunteti deja logati pe server!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");                            
                    }
                    else //inseamna ca nu sunt logat => caut daca exista username
                    {
                        /*
                            https://profs.info.uaic.ro/~computernetworks/files/NetEx/S4/socketpair.c
                            idee adaptata
                        */

                        printf("[S] Verificare username: \"%s\"... \n", user);
                        
                        int sockp[2], child; 

                        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                        { 
                            perror("[S] Eroare la socketpair!\n"); 
                            exit(1); 
                        }

                        if ((child = fork()) == -1) 
                            perror("[S] Eroare la fork() la login!\n"); 
                        else 
                        if (child)   //parinte 
                        {  
                            close(sockp[0]); //inchidere capat write

                            char raspuns[3];

                            // primire raspuns da/nu la logare
                            // primesc mereu 2 octeti -> am doar un read din socket
                            if (read(sockp[1], raspuns, 3) < 0)  //cati bytes trimite copilul 
                                perror("[S] Eroare citire in parinte la socket!\n");

                            raspuns[strlen(raspuns)] = '\0';
                            if(strcmp(raspuns,"DA") == 0)
                            {
                                printf("[S] Userul \"%s\" s-a logat cu succes! \n", user);
                                logat = 1;

                                if ((num2 = write(fd2, "22", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                            
                                sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                            
                                // trimit mesajul
                                if ((num2 = write(fd2, "V-ati logat cu succes!", strlen("V-ati logat cu succes!"))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                            }
                            else
                            {
                                logat = 0;
                                printf("[S] Userul \"%s\" nu a fost logat. Username inexistent.\n", user);
                                
                                if ((num2 = write(fd2, "32", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                            
                                sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                            
                                // trimit mesajul
                                if ((num2 = write(fd2, "Nu exista username-ul introdus!", strlen("Nu exista username-ul introdus!"))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                    perror("[S] Problema la scriere in FIFO! \n");
                            }

                            wait(NULL);
                            close(sockp[1]);  //inchidere capat read
                        } 
                        else     //copil
                        { 
                            close(sockp[1]); //inchidere capat read

                            char raspuns_login[300];
                            
                            // prelucrare username
                            FILE* fis = fopen(USERS_LIST,"r"); // deschid lista de useri in modul read pt a face cautarea
                            if(fis == NULL)  //eroare open fisier
                            {
                                perror("[S] Problema la deschiderea fisierului cu usernames! \n");
                            }
                            else  // s-a deschis fisierul cu succes -> caut username-ul line by line
                            {
                                const unsigned int maxlength = 30;  //maxim 30 de caractere pe fiecare linie
                                char name[maxlength];

                                while(fgets(name, maxlength, fis)) //citire linie cu linie din fisier 
                                {
                                    // caut username-ul in fisier
                                    name[strlen(name) - 1] = '\0';
                                    if(strcmp(name,user) == 0) // exista username ul in fisier => se scrie "V-ati logat cu succes!"
                                    {

                                        // transmit 2 octeti cu mesaj da
                                        if (write(sockp[0], "DA", sizeof("DA")) < 0)   // cati bytes va citi tatal
                                            perror("[S] Eroare write la socket in copil!\n");

                                        fclose(fis);
                                        exit(1);
                                    }
                                }


                                if (write(sockp[0], "NU", sizeof("NU")) < 0)   // cati bytes va citi tatal
                                    perror("[S] Eroare write la socket in copil!\n");
                            }
                            
                            close(sockp[0]);  //inchidere capat write
                            exit(1);
                        } 
                    }
                }
// COMANDA GET-PROC-INFO : PID   -> comunicare tata-copil prin pipe  
                else
                if ((ptr = strstr(cs, "get-proc-info : ")) != NULL)   // get_proc_info : pid
                {
                    printf("[S] S-a apelat functia \"get_proc_info : <pid>\"!\n");
                    if(logat == 1)  // utilizator logat
                    {
                        int fileD[2];  // 0-input, 1-output
                        pipe(fileD);  //
                        pid_t Pid = fork();

                        if(Pid > 0)  // procesul TATA
                        {
                            wait(NULL); //asteapta raspunsul din copil
                            
                            close(0);  // inchid capatul standard input

                            close(fileD[1]); // inchid capat de scriere a pipe-ului
                        
                            dup(fileD[0]); // duplic fd[0] cu standard input 0


                            //primire mesaj de la copil

                            char lungime[3];
                            lungime[0] = '\0';
                            read(fileD[0], lungime, sizeof(lungime));  //citesc in char[] cati bytes are raspunsul functiei

                            int nr_bytes = 0;
                            sscanf(lungime, "%d", &nr_bytes);  //conversie char array to int

                            char raspuns[nr_bytes + 1];
                            read(fileD[0], raspuns, sizeof(raspuns));


                            // trimitere mesaj de la tata catre client

                            // trimit la client cati bytes are mesajul scris de server
                            if ((num2 = write(fd2, lungime, 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                perror("[S] Problema la scriere in FIFO! \n");
                            
                            sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                            
                            // trimit la client mesajul
                            if ((num2 = write(fd2, raspuns, strlen(raspuns))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                perror("[S] Problema la scriere in FIFO! \n");
                        }
                        else
                        if(Pid == 0) // procesul COPIL
                        {
                            close(fileD[0]); // inchidere capat citire pipe
                            close(1);   // inchidere capat scriere std
                            dup(fileD[1]); // duplic fd[0] cu standard output 1

                            
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
                                //construire calea directorului /proc/<pid>
                                path[0] = '\0';
                                strcat(path,proc_dir);
                                strcat(path,"/");
                                strcat(path,pid_value);     

                                //verific daca directorul /proc/<pid> exista
                                DIR* pid_dir = opendir(path);
                                if(pid_dir)  // fisierul exista
                                {
                                    char status_file[30]; //construiesc calea /proc/<pid>/status

                                    status_file[0] = '\0';

                                    strcat(status_file,path);
                                    strcat(status_file,"/status");

                                    // cautare name, state, ppid, uid si vmsize in fisierul /proc/<pid>/status  :
                                    FILE *fptr = fopen(status_file, "r");

                                    if (fptr == NULL)
                                    {
                                        perror("Eroare la deschiderea fisierului /proc/<pid>/status \n");
                                        exit(1);
                                    }
                                    
                                    char line[300];
                                    char information[1500];
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
                                    
                                    //in information[] am raspunsul functiei
                                    int lung = strlen(information);

                                    //conversie int lung in char lung[]
                                    char buf[3];
                                    buf[0] = '\0';
                                    
                                    sprintf(buf, "%d", lung);

                                // TRIMITERE RASPUNS CATRE TATA
                                    write(1, buf, sizeof(buf));  //trimit mai intai la tata strlen(mesaj)
                                    sleep(1);
                                    write(1, information, sizeof(information));
                                }
                                else
                                if (ENOENT == errno) /* Directorul pid_dir nu exista */
                                {                                
                                    // trimit cati bytes are mesajul scris de copil pt ca, tatal sa ii citeasca
                                    char msg[] = "Pid-ul introdus nu exista!";

                                    write(1, "27", 3);  //trimit mai intai la tata strlen(mesaj)
                                    sleep(1);
                                    write(1, msg, sizeof(msg));
                                } 
                                else /* eroare la opendir()*/
                                {
                                    perror("Eroare la deschiderea directorului /proc/<pid>!\n");
                                }
                                closedir(dir);  //inchidere director /proc
                            }
                            else 
                            {   
                                if (ENOENT == errno) /* Directorul nu exista */
                                { 
                                    perror("Directorul /proc nu exista! \n");
                                } 
                                else /* opendir() failed for some other reason. */
                                {
                                    perror("Eroare la deschiderea directorului /proc! \n");
                                }
                            }
                            exit(1);  // inchidere copil pt a nu deveni zombie
                        }
                        else
                        {
                            perror("Eroare la fork()! \n");
                        }
                    }
                    else  //inseamna ca nu-s logat, deci nu pot folosi comanda
                    {
                        printf("[S] User-ul nelogat nu poate apela functia \"get_proc_info : pid\".\n");

                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                        if ((num2 = write(fd2, "47", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                        
                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                        
                        // trimit mesajul
                        char msg[] = "Nu puteti apela functia daca nu sunteti logati!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                    }
                }
// COMANDA GET-LOGGED-USERS
                else
                if(strcmp(cs,"get-logged-users") == 0)
                {
                    printf("[S] S-a apelat functia \"get-logged-users\"! \n");

                    if(logat == 1)
                    {
                        /*
                            idee adaptata dupa:
                            https://stackoverflow.com/questions/31472040/program-to-display-all-logged-in-users-via-c-program-on-ubuntu
                            si
                            https://www.epochconverter.com/programming/c
                        */
                       
                        int sockp[2], child; 

                        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                        { 
                            perror("[S] Eroare la socketpair!\n"); 
                            exit(1); 
                        }

                        if ((child = fork()) == -1) 
                            perror("[S] Eroare la fork() la login!\n"); 
                        else 
                        if (child)   //parinte 
                        {
                            close(sockp[0]); //inchidere capat write


                            char nr_octeti_char[3];  //cati bytes trimite copilul (char[])                          
                            
                            if (read(sockp[1], nr_octeti_char, 3) < 0)  //cati bytes trimite copilul 
                                perror("[S] Eroare citire in parinte la socket!\n");
                            sleep(1);

                            int nr_octeti_int;
                            sscanf(nr_octeti_char,"%d", &nr_octeti_int);

                            char raspuns[nr_octeti_int]; // informatia de la functie

                            if (read(sockp[1], raspuns, sizeof(raspuns)) < 0)  //cati bytes trimite copilul 
                                perror("[S] Eroare citire in parinte la socket!\n");
                            
                            raspuns[strlen(raspuns)]='\0';

                            printf("[S] %s \n", raspuns);


                            if ((num2 = write(fd2, nr_octeti_char, 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                            
                            sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                        
                            // trimit mesajul
                            if ((num2 = write(fd2, raspuns, strlen(raspuns))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                                perror("[S] Problema la scriere in FIFO! \n");


                            wait(NULL);
                            close(sockp[1]);  //inchidere capat read
                        }
                        else  //copil
                        {
                            close(sockp[1]); //inchidere capat read

                            char raspuns[300];

                            struct utmp *n;
                            setutent();  // file pointer la inceput
                            n = getutent(); // se citeste o linie din file

                            while(n) //citire linie cu linie pana se da de un user process
                            {
                                if(n->ut_type == USER_PROCESS)
                                {
                                    // conversie secunde (de la 1 ian 1970 pana la data de logare pe server) in data curenta
                                    time_t time_entry = n->ut_tv.tv_sec;  
                                    struct tm ts;  // strucg cu date calendaristice
                                    
                                    char data[30];

                                    // Formatare in: "yyyy-mm-dd hh:mm:ss"
                                    ts = *localtime(&time_entry);   // pune valoarea time_entry (secunde) in structura ts
                                    strftime(data, sizeof(data), "%Y-%m-%d at %H:%M:%S", &ts); // face conversia de format

                                    strcpy(raspuns,"Name: ");
                                    strcat(raspuns, n->ut_user);
                                    strcat(raspuns,"| Hostname: ");
                                    strcat(raspuns, n->ut_host);
                                    strcat(raspuns, "| Time Entry: ");
                                    strcat(raspuns, data);

                                    int len = strlen(raspuns);
                                    char buf[3];
                                    buf[0] = '\0';
                                    sprintf(buf, "%d", len);  // conversie int to char array

                                    
                                    if (write(sockp[0], buf, sizeof(buf)) < 0)   // cati bytes va citi tatal
                                        perror("[S] Eroare write la socket in copil!\n");
                                    sleep(1);
                                    if (write(sockp[0], raspuns, sizeof(raspuns)) < 0)
                                        perror("[S] Eroare write la socket in copil!\n");
                                }
                                n = getutent();
                            }

                            close(sockp[0]);  //inchidere capat write
                            exit(1);
                        }
                    }
                    else  // nu se poate folosi functia daca user-ul nu este logat
                    {
                        printf("[S] User-ul nelogat nu poate apela functia \"get-logged-users\".\n");

                        // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                        if ((num2 = write(fd2, "60", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                        
                        sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                        
                        // trimit mesajul
                        char msg[] = "Nu puteti folosi \"get-logged-users\" daca nu sunteti logati!";
                        if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                            perror("[S] Problema la scriere in FIFO! \n");
                    }
                }
// COMENZI INVALIDE 
                else 
                {
                    printf("[S] User-ul a introdus o comanda gresita. \n");

                    // trimit cati bytes are mesajul scris de server pt ca, CLIENTUL sa ii citeasca
                    if ((num2 = write(fd2, "32", 3)) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                        perror("[S] Problema la scriere in FIFO! \n");
                    
                    sleep(1);   // astept CLIENTUL sa primeasca lungimea mesajului
                    
                    // trimit mesajul
                    char msg[] = "Ati introdus o comanda invalida!";
                    if ((num2 = write(fd2, msg, strlen(msg))) == -1) // se scrie in fifo si in num am cati bytes s-au scris
                        perror("[S] Problema la scriere in FIFO! \n"); 
                }  
            }  
        }
    } while (num1 > 0);
}