/*
 * resparser.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xlib.h>
#include <resparser.h>


/* is200() */
boolean is200(char *response)
{
	if (((strncmp(response, "200", strlen("200"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* is201() */
boolean is201(char *response)
{
	if (((strncmp(response, "201", strlen("201"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* is202() */
boolean is202(char *response)
{
	if (((strncmp(response, "202", strlen("202"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* is402() */
boolean is402(char *response)
{
	if (((strncmp(response, "402", strlen("402"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* is403() */
boolean is403(char *response)
{
	if (((strncmp(response, "403", strlen("403"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* is404() */
boolean is404(char *response)
{
	if (((strncmp(response, "404", strlen("404"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* is405() */
boolean is405(char *response)
{
	if (((strncmp(response, "405", strlen("405"))) == 0))
		return (TRUE);

	return (FALSE);
}


/* getResponse()
 * Ottiene il codice di risposta da parte del server.
 * @response: Risposta del server
 */
int getResponse(char *response)
{
	int i = 0;
	char tmp[4];	
	
	for (i=0; i<3; i++)
		tmp[i] = response[i];

	return (atoi(tmp));
}


/* getResLen()
 * Ottiene la lunghezza in byte della dimensione del file
 * che il server ha inviato.
 * @response: Risposta del server
 */
int getResLen(char *response)
{
	int i = 0;
	int j = 0;
	
	char tmp[20];
	char *ptr = strstr(response, "Len ");

	/* La risposta non contiene la lunghezza */
	if (ptr == NULL)
		return (0);
	
	/* Scrorro la stringa e estraggo solo la lunghezza:
	 * NB: i parte da 4 per trascurare la stringa iniziale
	 * "Len_" */
	for (i=strlen("Len "); i<strlen(ptr); i++) {
		if (ptr[i] == '\n')
			break;
		tmp[j] = ptr[i];
		j++;
	}

	tmp[j] = '\0';
	return (atoi(tmp));
}


/* getResRange()
 * Ottiene il range della risorsa che si vuole scaricare.
 * @start: buffer in cui memorizzare il byte di inizio da cui inizare la copia
 * @end: buffer in cui memorizzare il byte di fine per terminare la copia
 * @RETURN: numero di byte necessari
 */
int getResRange(char *response, int *start, int *end)
{
	int i = 0;
	int e = 0;
	int s = 0;

	char tmp_start[10];
	char tmp_end[10];
	char *ptr = strstr(response, "Range ");

	boolean OK_END = FALSE;
	boolean OK_START = TRUE;
	
	/* la richiesta non contiene range */
	if (ptr == NULL)
		return (-1);

	for (i=strlen("Range "); i<strlen(ptr); i++)
	{
		if (ptr[i] == '\n')
			break;

		/* quando incontro il carattere '-' significa che ho copiato
		 * la prima parte e rimane da copiare la seconda parte del
		 * range. */
		if (ptr[i] == '-')
		{
			i++;	/* per trascurare il carattere '-' */

			OK_START = FALSE;

			/* se il range e' nella forma Range x- allora non serve estrarre la fine
			 * del range */
			if (ptr[i] == '\n')
				OK_END = FALSE;
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
	*end = atoi(tmp_end);
	
	return ((*end)-(*start)+1);
}


/* getResExpire()
 * Ottiene il tempo di expire della risorsa.
 */
int getResExpire(char *response)
{
	int i = 0;
	int j = 0;
	char tmp[20];
	char *ptr = strstr(response, "Expire ");

	/* La risposta non contiene nessun expire */
	if (ptr == NULL)
		return (-1);

	for (i=strlen("Expire "); i<strlen(ptr); i++)
	{
		if (ptr[i] == '\n')
			break;
		tmp[j] = ptr[i];
		j++;
	}
	
	tmp[j] = '\0';

	return (atoi(tmp));
}


/* getResContent()
 * Estrazione del blocco dati dalla risposta del server.
 */
void getResContent(char *response, char *buffer)
{
	int i = 0;
	int j = 0;
	int cont = 0;
	
	boolean START_TO_COPY = FALSE;

	for (i=0; i<strlen(response); i++)
	{
		if (response[i] == '\n') {
			cont++;
			if (cont == 4) {
				START_TO_COPY = TRUE;
				i++;
			}
		}
		
		if (START_TO_COPY == TRUE) {
			buffer[j] = response[i];
			j++;
		}
	}
	
	buffer[j] = '\0';
}


/* getNumResources()
 * Ottiene il numero di risorse (IDX o REF) che sono presenti
 * nei dati.
 * @text:   rappresenta il blocco di dati nel quale cercare le risorse
 * @RETURN: numero di risorse che si trovano nel testo, -1 in caso di errore
 */
int getNumResources(char *text)
{
	int i;
	int cont = 0;

	for (i=0; i<strlen(text); i++) {
		if (text[i] == '<')
			cont++;
	}
	
	/* Controllo che non ci siano piu' di 10 riferimenti IDX+REF e REF */
	if ((cont > MAXNUMEMBEDDEDOBJECT) || (cont > MAXNUMLINK))
		return (-1);

	return (cont);
}


/* getINFO()
 * Estrare IDX e REF da un testo. Se l'IDX non e' presente
 * allora vale NULL.
 * @text:   testo su cui effettuare il parsing
 * @IDX:    buffer in cui memorizzare IDX
 * @REF:    buffer in cui memorizzare REF
 * @index:  indica l'indice della risorsa da estrare
 * @RETURN: la funzione ritorna 1 in caso di successo, e 0 per indicare
 *          che l'indice passato alla funzione non e' corretto.
 */
void getINFO(char *text, char *IDX, char *REF, int index)
{
	int  i = 0;
	int  j = 0;
	int  count = -1;
	char buffer[MAXLENDATA];
	
	boolean OK_COPY = FALSE;

	/* Inizialmente si verifica che l'indice passato non sia superiore al numero 
	 * di risorse disponibili nella pagina .mhtml. */
	if (getNumResources(text) >= index)
	{
		for (i=0; i<strlen(text); i++) {

			/* Ogni volta che incontro il carattere '<' significa che sono su una risorsa
			 * diversa per cui incremento il contatore. */
			if (text[i] == '<') {
				count++;
				
				/* se l'indice della risorsa in cui mi trovo e' uguale all'indice della 
				 * risorsa che sto cercando allora posso iniziare a copiare l'indirizzo
				 * della risorsa */
				if (count == index) {
					OK_COPY = TRUE;
				}
			}

			/* Ogni volta che incontro il carattere '>' significa che sono arrivato alla
			 * fine della risorsa per cui non e' piu' necessario copiare l'indirizzo */ 
			if ((text[i] == '>') && (count == index)) {
					OK_COPY = FALSE;
			}

			/* Se OK_TO_COPY vale 'true' siginifica che sono all'interno
			 * della risorsa interessata, e quindi posso copiare la stringa. */
			if (OK_COPY == TRUE) {
				buffer[j] = text[i];
				j++;
			}
			
		}
		
		/* Il buffer adesso contiene <IDX=n;REF=indirizzo> */
		buffer[j] = '\0';
	
		/* dal buffer viene estratto l'IDX e il riferimento */
		getREF(buffer, REF);
		getIDX(buffer, IDX);
		
	}
}


/* getREF()
 * Estrae l'indirizzo della risorsa.
 * @text: testo su cui effetare il parsing (nella forma: <IDX=3;REF=mhttp://1.1.1.1.1:1/1.mhtml>
 * @REF: buffer in cui memorizzare REF
 */
void getREF(char *text, char *REF) {

	int i = 0;
	int j = 0;

	/* Mi posiziono all'ultimo carattere "=" nella stringa in cui cercare. */
	char *ptr = strrchr(text, '=');

	memset((void*)REF, 0x00, MAXLENREF);

	/* i parte da 1 per trascurare il primo carattere "=" */
	for (i=1; i<strlen(ptr); i++) {
		REF[j] = ptr[i];
		j++;
	}

	REF[j] = '\0';
}


/* getIDX()
 * Estrae il numero IDX della risorsa.
 * @text: testo su cui effetare il parsing (nella forma: <IDX=3;REF=mhttp://1.1.1.1.1:1/1.mhtml>
 * @IDX: buffer in cui memorizzare IDX
 */
void getIDX(char *text, char *IDX) {

	int i = 0;
	int j = 0;
	char *ptr;

	memset((void*)IDX, 0x00, MAXIDXLEN);

	/* Alcune risorse nei file non hanno IDX per cui e' necessario
	 * controllare inizialmente prima di effettuare il parsing
	 * se IDX esiste o meno. Se IDX non esiste allora viene
	 * impostato a 'NOT_VALID' e la funzione termina immediatamente, altrimenti
	 * viene effettuato il parsing.
	 */
	if ((strncmp(text, "<IDX", strlen("<IDX"))) != 0) {
		sprintf(IDX, "%s", NOT_VALID);
		return;
	}

	/* mi posiziono nel primo uguale (che sarebbe quello di IDX) */
	ptr = strchr(text, '=');

	/* adesso *ptr contiene =3/blablabla:bla/bla.mhtml */

	for (i=1; i<strlen(text); i++) {
		if (ptr[i] == ';')
			break;
		IDX[j] = ptr[i];
		j++;
	}

	IDX[j] = '\0';
}
