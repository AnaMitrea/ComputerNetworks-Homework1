#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//pt director:
#include <dirent.h>
#include <errno.h>

int main()
{
    
    char cmd[300];

    // https://stackoverflow.com/questions/12510874/how-can-i-check-if-a-directory-exists    

    char proc_dir[6] = "/proc";
    char pid_value[10]; //valoarea pidului
    char path[20];

    DIR* dir = opendir(proc_dir);

    if (dir) /* Directory exists. */
    {
        printf("Directorul cu numele \"%s\" exista! \n", proc_dir);

        printf("Introduceti pid-ul! pid = ");
        scanf("%s",pid_value);
        printf("S-a scris pid-ul \"%s\"!\n", pid_value);

        //construire calea directorului /proc/<pid>
        path[0] = '\0';
        strcat(path,proc_dir);
        strcat(path,"/");
        strcat(path,pid_value);

        printf("Path-ul pidului in director este: \"%s\"  \n", path);        

        //verific daca directorul /proc/<pid> exista
        DIR* pid_dir = opendir(path);
        if(pid_dir)
        {
            printf("Directorul (cu pid) cu numele \"%s\" exista! \n", path);

            char status_file[30]; //calea /proc/<pid>/status

            status_file[0] = '\0';

            strcat(status_file,path);
            strcat(status_file,"/status");

            printf("Path-ul catre status este: \"%s\"  \n", status_file);

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

            printf("Informatiile despre pidul %s sunt: \n%s", pid_value, information);
        }
        else
            if (ENOENT == errno) /* Directory does not exist. */
            { 
                printf("Directorul (cu pid) numele \"%s\" nu exista! \n", path);
            } 
            else /* opendir() failed for some other reason. */
            {
                printf("Eroare la deschiderea directorului (cu pid) \"%s\" \n", path);
            }

        closedir(dir);  //inchidere director /proc
    }
    else    
        if (ENOENT == errno) /* Directory does not exist. */
        { 
            printf("Directorul cu numele \"%s\" nu exista! \n", proc_dir);
        } 
        else /* opendir() failed for some other reason. */
        {
            printf("Eroare la deschiderea directorului \"%s\" \n", proc_dir);
        }
}
