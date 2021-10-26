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
// pt comanda:
#include<sys/utsname.h>
#include<utmp.h>

//pt time conversion:
#include <time.h>

int main()
{
    struct utmp *n;
    setutent();
    n=getutent();

    /*
        trebuie afisat: username, hostname for remote login, time entry was made
    */

    while(n) {
        if(n->ut_type == USER_PROCESS) 
        {
            // conversie
            time_t time_entry = n->ut_tv.tv_sec;  
            struct tm ts;
            char buf[80];

            // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
            ts = *localtime(&time_entry);
            strftime(buf, sizeof(buf), "%Y-%m-%d at %H:%M:%S", &ts);
            printf("Name: %s | Hostname %s | Time Entry: %s\n", n->ut_user, n->ut_host, buf);
        }
        n = getutent();
    }
    return 0;
}