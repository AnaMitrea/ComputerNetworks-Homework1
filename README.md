# Proiect-Retele-de-calculatoare

https://profs.info.uaic.ro/~computernetworks/cursullaboratorul.php


RO:
Dezvoltati doua aplicatii (denumite "client" si "server") ce comunica intre ele pe baza unui protocol care are urmatoarele specificatii:

- comunicarea se face prin executia de comenzi citite de la tastatura in client si executate in procesele copil create de server;
- comenzile sunt siruri de caractere delimitate de new line;
- raspunsurile sunt siruri de octeti prefixate de lungimea raspunsului;
- rezultatul obtinut in urma executiei oricarei comenzi va fi afisat de client;
- procesele copil din server nu comunica direct cu clientul, ci doar cu procesul parinte;
- protocolul minimal cuprinde comenzile: 
      - "login : username" - a carei existenta este validata prin utilizarea unui fisier de configurare, care contine toti utilizatorii care au acces la functionalitati. Executia comenzii va fi realizata intr-un proces copil din server;
      - "get-logged-users" - afiseaza informatii (username, hostname for remote login, time entry was made) despre utilizatorii autentificati pe sistemul de operare (vezi "man 5 utmp" si "man 3 getutent"). Aceasta comanda nu va putea fi executata daca utilizatorul nu este autentificat in aplicatie. Executia comenzii va fi realizata intr-un proces copil din server;
      - "get-proc-info : pid" - afiseaza informatii (name, state, ppid, uid, vmsize) despre procesul indicat (sursa informatii: fisierul /proc/<pid>/status). Aceasta comanda nu va putea fi executata daca utilizatorul nu este autentificat in aplicatie. Executia comenzii va fi realizata intr-un proces copil din server;
      - "logout";
      - "quit".
- in implementarea comenzilor nu se va utiliza nicio functie din familia "exec()" (sau alta similara, de ex. popen(), system()...) in vederea executiei unor comenzi de sistem ce ofera functionalitatile respective;
- comunicarea intre procese se va face folosind cel putin o data fiecare din urmatoarele mecanisme ce permit comunicarea: pipe-uri, fifo-uri si socketpair.


Observatii:
- termen de predare: laboratorul din saptamana 5;
- orice incercare de frauda, in functie de gravitate, va conduce la propunerea pentru exmatriculare a studentului in cauza sau la punctaj negativ.





ENG:
  Develop two applications (named "client" and "server") which communicate between them using a protocol having the following specifications:

- the communication is made by executing commands read from the keyboard in the client app and executed in child processes in the server app;
- the commands are strings bounded by a new line;
- the responses are series of bytes prefixed by the length of the response;
- the result obtained from the execution of any command will be displayed on screen by the client app;
- the child processes in the server app do not communicate directly with the client app, only with the parent process;
- the minimal protocol includes the following commands:
    - "login : username" - whose existence is validated by using a configuration file, which contains all the users that have access to the functionalities. This command's execution will be made in a child process in the server app;
    - "get-logged-users" - displays information (username, hostname for remote login, time entry was made) about all users that are logged in the operating system (see "man 5 utmp" and "man 3 getutent"). This command should not execute if the user is not authenticated in the application. This command's execution will be made in a child process in the server app;
    - "get-proc-info : pid" - displays information (name, state, ppid, uid, vmsize) about the process whose pid is specified (source of information: the file /proc/<pid>/status). This command should not execute if the user is not authenticated in the application. This command's execution will be made in a child process in the server app;
    - "logout";
    - "quit".
- no function in the "exec()" family (or other similar, like popen(), system()...) will be used to execute some system commands that offer the same functionalities;
- the communication among processes will be done using all of the following communication mechanisms: pipes, fifos, and socketpairs.


Observations:
- deadline: the 5th week laboratory
- any fraud attempt, depending on severity, will lead to the student's expelling proposal or a negative score.
