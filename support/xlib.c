/*
 * xlib.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <xlib.h>
#include <util.h>
#include <resparser.h>


/* errquit()
 * Stampa il relativo messaggio d'errore ed esce.
 * @msg: messaggio da stampare
 */
void errquit(char *msg)
{
	fprintf(stderr, "%s: %s.\n", msg, strerror(errno));
	exit(EXIT_FAILURE);
}


/* usage()
 * Mostra il modo corretto di inserire i parametri per l'avvio del proxy.
 */
void usage(char *progname)
{
	fprintf(stdout, "Usage:  %s [options] ... \n", progname);
	fprintf(stdout, "Where 'options' are:\n");
	fprintf(stdout, " -h    --help             Visualizza l'elenco dei comandi disponibili\n");
	fprintf(stdout, " -n    --no-caching       Disattiva la memoria cache interna all'applicazione\n");
	fprintf(stdout, " -l L  --log=<logfile>    Registrazione delle richieste/risposte in un file di LOG\n");
	fprintf(stdout, " -3    --cache-level3     Attiva il 1° 2° e 3° livello di caching\n");
	fprintf(stdout, " -k    --print-cache      Stampa il contenuto della memoria cache, visualizzando le risorse\n");
	fprintf(stdout, " -m N  --maxclient=<N>   Numero max. di clients che possono connettersi al Proxy\n");
	fprintf(stdout, " -p N  --port=<N>         Numero di porta del Proxy (default: 55554)\n");
	fprintf(stdout, "\n");
	exit(EXIT_SUCCESS);
}


/* server_connect()
 * Connessione con il server.
 */
int server_connect(struct server_t *sid)
{
	int serversd;
	struct sockaddr_in server;

	/* creazione socket remoto */
	if ((serversd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return (-1);

	/* inizializzazione della struttura del server */
	memset((void*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(sid->port);
	server.sin_addr.s_addr = inet_addr(sid->address);

	/* Connessione con il server */
	if ((connect(serversd, (struct sockaddr*)&server, sizeof(server))) < 0)
		return (-1);

	return (serversd);
}


/* data_receive_from_server()
 * Riceve i dati dal server e li memorizza all'interno di un buffer.
 * @RETURN:  n (dove n > 0) numero di byte ricevuti
 *           0 i dati sono arrivati ma non sono completi
 *          -1 timeout scaduto (server bloccato)
 *          -2 impossibile connettersi al server
 */
int data_receive_from_server(struct server_t *sid, char *request, char *response) 
{
	ssize_t nread;
	int  cont = 0;
	char *ptr = response;
	long int start, end;
	int  length;
	int  dataLength;
	int  serversd;
	char data[MAXLENDATA];
	
	memset((void*)response, 0x00, MAXLENRESP);
	
	/* connessione con il server */
	if ((serversd = server_connect(sid)) < 0)
		return (-2);
	
	/* inivio la richiesta al server */
	data_send(serversd, request);

	/* ottengo il tempo di inizio, per rimanere in attesa
	 * non piu' di 10 secondi */
	time(&start);
	
	while (TRUE)
	{
		/* ricevo la risposta dal server */	
		nread = recv(serversd, ptr, MAXLENRESP, MSG_DONTWAIT|MSG_NOSIGNAL);

		if (errno == EINTR)
			continue;

		if (nread == 0)
		{
			/* parsing della risposta del server */
			getResContent(response, data);
			length = getResLen(response);
			dataLength = strlen(data);
		
			if (length == dataLength)	/* ho ottenuto tutti i dati */
				return (cont);
			else								/* non ho ottenuto tutti i dati */
				return (0);
		}
		else if (nread > 0)
		{
			cont += nread;
			ptr += nread;
		}
		else
		{
			time(&end);
			if ((end-start) >= INACTIVITY_TIMEOUT_SECONDS)
				return (-1);
		}
	}
	close(serversd);
	return (cont);
}


/* data_receive_from_client()
 * Riceve i dati dal client e li memorizza all'interno di un buffer.
 * @return: numero di byte letti
 */
int data_receive_from_client(int sd, char *buffer, int size) 
{
	ssize_t nread;
	memset((void*)buffer, 0x00, MAXLENREQ);

	if (((nread = recv(sd, buffer, size, MSG_NOSIGNAL)) < 0))
		errquit("PROXY[data_receive_from_client]: read()");

	return (nread);
}


/* data_send()
 * Invia i dati all'interno del 'buffer' al socket descriptor 'sd'.
 */
int data_send(int sd, char *buffer)
{
	ssize_t nwrite;
	
	if ((nwrite = send(sd, buffer, strlen(buffer), MSG_NOSIGNAL) < 0)) {
		errquit(" PROXY[data_send]: send()");
	}
	
	return (nwrite);
}


/* set_socket_nonblock()
 * Imposta il socket come non bloccante.
 */
void set_socket_nonblock(int *sd)
{
	int flags;

	/* ottengo le informazioni di default del socket */
	if ((flags = fcntl((*sd), F_GETFL, 0)) < 0)
		errquit("PROXY[main]: fcntl(): F_GETFL");

	/* aggiungo il flag NONBLOCK a quelli di default */
	flags |= O_NONBLOCK;

	/* imposto il socket come bloccante */
	if ((fcntl((*sd), F_SETFL, flags)) < 0)
		errquit("PROXY[main]: fcntl(): F_SETFL");
}


/* set_socket_block()
 * Imposta il socket come bloccante.
 */
void set_socket_block(int *sd)
{
	int flags;

	/* ottengo le informazioni di default del socket */
	if ((flags = fcntl((*sd), F_GETFL, 0)) < 0)
		errquit("PROXY[main]: fcntl(): F_GETFL");

	/* aggiungo il flag NONBLOCK a quelli di default */
	flags &= (~O_NONBLOCK); 
	
	/* imposto il socket come bloccante */
	if ((fcntl((*sd), F_SETFL, flags)) < 0)
		errquit("PROXY[main]: fcntl(): F_SETFL");
}


/* request_resource_server()
 * Chiede una risorsa al server.
 */
int request_resource_to_server(struct server_t *sid, char *REF, char *response)
{
	char request[MAXLENREQ];
	
	/* creo la stringa di richiesta */
	sprintf(request, "GET %s\n\n", REF);

	/* ricevo la risposta */
	if ((data_receive_from_server(sid, request, response)) > 0)
		return (1);
	
	return (0);
}
