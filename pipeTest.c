#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    // https://www.geeksforgeeks.org/pass-the-value-from-child-process-to-parent-process/
    int fd[2];

    pipe(fd);
    pid_t Pid = fork();
  
    if(Pid > 0) // tatal
    {
        wait(NULL); //asteapta raspunsul din copil
        
        close(0);  // inchid capatul standard input

        close(fd[1]); // inchid capat de scriere a pipe-ului
    
        dup(fd[0]); // duplicating fd[0] with standard input 0


        unsigned int lungime = 0;
        read(fd[0], &lungime, sizeof(lungime));

        char arr[lungime + 1];
        read(fd[0], arr, sizeof(arr));


        printf("%s \n", arr); // afisare sir trimit de copil
    } 
    else 
    if(Pid == 0) // copil
    {
        char arr[] = "alabala portocala";

        close(fd[0]); // inchidere capat citire pipe
        
        close(1);   // inchidere capat scriere std

        dup(fd[1]); // duplicating fd[0] with standard output 1


        unsigned int lungime = strlen(arr);
        write(1, &lungime, sizeof(lungime));  //trimit mai intai la tata strlen(mesaj)
        sleep(1);
        write(1, arr, sizeof(arr));

        exit(1);  // inchid copilul ca sa nu fie proces zombie
    } 
    else 
    {
        perror("error\n"); //fork()
    }
}