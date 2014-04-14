Remote Reservation System
=========================
Sistema di prenotazione posti remoto.

#Specifica
Realizzazione di un sistema di prenotazione posti per una sala
cinematografica. Un processo su una macchina server gestisce una mappa di
posti per una sala cinematografica. Ciascun posto e' caratterizzato da un
numero di fila, un numero di poltrona ed un FLAG indicante se il posto
e' gia' stato prenotato o meno.
Il server accetta e processa le richieste di prenotazione
di posti da uno o piu' client (residenti, in generale, su macchine diverse).
Un client deve fornire ad un utente le seguenti funzioni:

1. Visualizzare la mappa dei posti in modo da individuare quelli ancora
disponibili.
2. Inviare al server l'elenco dei posti che si intende prenotare (ciascun
posto da prenotare viene ancora identificato tramite numero di fila e numero di
poltrona).
3. Attendere dal server la conferma di effettuata prenotazione ed un codice di prenotazione.
4. Disdire una prenotazione per cui si possiede un codice.

Si precisa che lo studente e' tenuto a realizzare sia il client che il
server.

Il server deve poter gestire le richieste dei client in modo concorrente.

#Installazione
###Scaricare il codice sorgente
Posizionarsi nella cartella dove si vuole scaricare il codice e utilizzare wget
``` Bash
wget -c "https://github.com/ael-code/remote-reservation-system/archive/semaphores.zip" -O "remote-reservation-system.zip"
```
Decomprimere l'archivio
``` Bash
unzip remote-reservation-system.zip
```
Spostarsi nella cartella dove si trova il codice sorgente
``` Bash
cd remote-reservation-system-semaphores
```
###Compilazione
Il processo di compilazione e' stato facilitato tramite l'utilizzo di un Makefile. E' sufficente invocare
``` Bash
make
```
Verra' creata una cartella "bin" contenente i due 
eseguibili e i file intermedi utili alla compilazione 
(con estensione '.o')

#Esecuzione
##Server
##Client
