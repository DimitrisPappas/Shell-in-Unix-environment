Σε περίπτωση που δεν λειτουργεί το make
Compile with: gcc -Wall dShell.c -lreadline -o dShell
OR: gcc -Wall dShell.c -lreadline -L/usr/include -o dShell
Πρέπει να υπαρχει η βιβλιοθήκη readline.h
Αν δεν υπάρχει: sudo apt-get install libreadline-dev 

