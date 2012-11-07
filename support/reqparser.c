/*
 * resparser.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xlib.h>
#include <reqparser.h>

/* isGET()
 * La funzione verifica se un messaggio e' di tipo GET,
 * ed in quel caso restituisce 'TRUE', altrimenti FALSE.
 */
boolean isGET(char *request)
{
	if (((strncmp(request, "GET", strlen("GET"))) == 0))
		return (TRUE);
	
	return (FALSE);
}


/* isINF()
 * La funzione verifica se un messaggio e' di tipo
 * INF, ed in quel caso restituisce 'TRUE', altrimenti
 * FALSE.
 */
boolean isINF(char *request)
{
	if (((strncmp(request, "INF", strlen("INF"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* getReqType()
 * La funzione memorizza all'interno del parametro 'type'
 * il tipo di richiesta (GET/INF).
 */
void getReqType(char *request, char *type)
{
	if (isGET(request))
		strcpy(type, "GET");
	else if (isINF(request))
		strcpy(type, "INF");
	else
		strcpy(type, NOT_VALID);
}


/* getReqAddress()
 * Estrae la risorsa che si vuole scaricare.
 * @request: stringa che identifica il tipo di richiesta del client.
 * @address: buffer in cui memorizzare l'indirizzo
 */
int getReqAddress(char *request, char *address)
{
	int  i = 0;
	int  j = 0;
	char type[4];

	/* ottengo il tipo di richiesta: GET o INF e lo memorizzo all'interno
	 * della variabile 'type' */
	getReqType(request, type);
	
	memset((void*)address, 0x00, ADDRLEN);
	
	/* La richiesta non e' corretta in quanto non e' ne' una richiesta 
	 * di tipo GET ne' di tipo INF. */
	if (strcmp(type, NOT_VALID) == 0) {
		return (0);
	}
	
	/* i parte da 4 per trascurare i primi caratteri che compongono il tipo
	 * di richiesta GET o INF */
	for (i=4; i<strlen(request)-1; i++) {

		/* quando incontro il carattere \n significa che l'indirizzo della risorsa
		 * e' terminato per cui chiudo il ciclo for */
		if (request[i] == '\n')
			break;
		
		address[j] = request[i];
		j++;
	}
	
	address[j] = '\0';
	return (1);
}


/* getReqRange()
 * Estrae il range dalla richiesta del server.
 * @return: 1 per indicare che il parsing e' avvenuto con successo,
 *         -1 per indicare che la richiesta e' priva di range
 */
int getReqRange(char *request, int *start, int *end)
{
	int i = 0;
	int e = 0;
	int s = 0;

	char tmp_start[10];
	char tmp_end[10];
	
	char *ptr = strstr(request, "Range ");
	
	boolean OK_END = FALSE;
	boolean OK_START = TRUE;
	boolean NO_END_NUMBER = FALSE;
	
	/* la richiesta non contiene range */
	if (ptr == NULL)
		return (-1);

	for (i=strlen("Range "); i<strlen(ptr); i++) {
		if (ptr[i] == '\n')
			break;
		
		/* quando incontro il carattere '-' significa che ho copiato
		 * la prima parte e rimane da copiare la seconda parte del
		 * range. */
		if (ptr[i] == '-') {
			i++;	/* incremento per trascurare il carattere '-' */
			
			OK_START = FALSE;

			/* se il range e' nella forma Range x- allora non serve estrarre la fine
			 * del range */
			if (ptr[i] == '\n') {
				OK_END = FALSE;
				NO_END_NUMBER = TRUE; /* indico che il range è nella forma Range x-\n */
				break;
			}
			else
				OK_END = TRUE;
		}
		
		if (OK_START == TRUE) {
			tmp_start[s] = ptr[i];
			s++;
		}
		
		if (OK_END == TRUE) {
			tmp_end[e] = ptr[i];
			e++;
		}
	}
	tmp_start[s] = '\0';
	tmp_end[e] = '\0';
	
	*start = atoi(tmp_start);
	
	if (NO_END_NUMBER == TRUE)
		*end = -1;
	else
		*end = atoi(tmp_end);

	return (1);
}
