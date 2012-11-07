/*
 * log.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include <log.h>
#include <xlib.h>

/* log_init()
 * Scrive la data/ora corrente nel file di LOG
 * in modo da avere i LOG delle richieste per ogni connessione.
 */
void log_init(char *logfile, boolean logging)
{
 if (logging)
 {
	int  fd;
	int  flags;
	char buffer[BUFFSIZE];
	time_t now;
	mode_t mode;
	struct tm *info;
	
	time(&now);
	info = localtime(&now);
	
	flags = O_CREAT|O_WRONLY|O_APPEND|O_DSYNC;
	mode  = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
	
	if ((fd = open(logfile, flags, mode)) < 0)
		errquit("PROXY[log_init] open()");

	sprintf(buffer, "\nStart logging on %s====\n", asctime(info));
	if (write(fd, buffer, strlen(buffer)) < 0)
		errquit("PROXY[log_init] write()");
			
	close(fd);
 }
}


/* log_request()
 * Registra una richiesta del client.
 */
void log_request(char *logfile, char *request, char *addr, int port, boolean logging)
{
 if (logging)
 {
	int fd;
	int flags;
	char buffer[BUFFSIZE];
	char newreq[MAXLENREQ];
	mode_t mode; 
	
	flags = O_CREAT|O_WRONLY|O_APPEND|O_DSYNC;
	mode  = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
	
	strncpy(newreq, request, strlen(request)-2);

	if ((fd = open(logfile, flags, mode)) < 0)
		errquit("PROXY[log_request] open()");

	sprintf(buffer, "\nRequest from client %s on port %d\n -> %s\n", addr, port, newreq);
	if (write(fd, buffer, strlen(buffer)) < 0)
		errquit("PROXY[log_request] write()");
		
	close(fd);
 }
}


/* log_cache()
 * Inserisce nel logfile un'informazione che indica la presenza
 * o meno della risorsa in cache.
 */
void log_cache(char *logfile, char *resource, boolean exists, boolean logging) 
{
 if (logging)
 {
	int fd;
	int flags;
	char buffer[BUFFSIZE];
	mode_t mode; 

	flags = O_CREAT|O_WRONLY|O_APPEND|O_DSYNC;
	mode  = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

	if ((fd = open(logfile, flags, mode)) < 0)
		errquit("PROXY[log_request] open()");

	if (exists == TRUE) {
		sprintf(buffer, " -> Resource %s found in cache\n", resource);
		if (write(fd, buffer, strlen(buffer)) < 0)
			errquit("PROXY[log_cache] write()");
	}
	else if (exists == FALSE) {
		sprintf(buffer, " -> Resource %s not found in cache\n", resource);
		if (write(fd, buffer, strlen(buffer)) < 0)
			errquit("PROXY[log_cache] write()");
	}
	close(fd);
 }
}


/* log_response()
 * Inserisce nel logfile la risposta del server
 */
void log_response(char *logfile, char *msg, char *resource, char *saddr, char *caddr, boolean logging)
{
 if (logging)
 {
	int fd;
	int flags;
	char buffer[BUFFSIZE];
	char response[100];
	mode_t mode;
	
	memset((void*)buffer, 0x00, BUFFSIZE);

	flags = O_CREAT|O_WRONLY|O_APPEND|O_DSYNC;
	mode  = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

	if ((fd = open(logfile, flags, mode)) < 0)
		errquit("PROXY[log_request] open()");

	strcpy(response, msg);
	sprintf(buffer, "\nResponse from server %s to client %s\n -> Sending resource '%s'\n -> Response: [%s]\n", saddr, caddr, resource, response);

	if (write(fd, buffer, strlen(buffer)) < 0)
		errquit("PROXY[log_response] write()");	
	
	close(fd);
 }
}
