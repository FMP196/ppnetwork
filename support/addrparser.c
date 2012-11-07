/*
 * addrparser.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xlib.h> 
#include <addrparser.h>


/* IS_CORRECT()
 * Veririca se l'indirizzo passatogli e' corretto, ovvero contiene
 * tutte le informazioni necessarie per reperire la risorsa.
 */
boolean IS_CORRECT(char *REF)
{
	if ((strlen(REF) >= MINLENGTH))     /* verifico che l'indirizzo sia sufficientemente lungo */
		if (hasDoublePoints(REF))        /* verifica i due punti ":" dopo "mhttp" e prima del numero della porta */
			if (isMHTTP(REF))             /* inizia con mhttp:// ? */
				if (isMHTML(REF))          /* finisce con .mhtml  ? */
					return (TRUE);

	return (FALSE);
}


/* isMHTTP()
 * Verifica che l'indirizzo sia un indirizzo del protocollo mhttp.
 */
boolean isMHTTP(char *REF)
{
	if ((strncmp(REF, "mhttp://", strlen("mhttp://")) == 0))
			return (TRUE);

	return (FALSE);
}


/* isMHTML()
 * Verifica se la risorsa e' un file .mhtml
 * @REF:    identifica l'indirizzo completo.
 * @return: TRUE in caso di successo, FALSE altrimenti.
 */
boolean isMHTML(char *REF)
{
	/* Cerco l'ultima occorrenza del carattere "." in address.
	 * *ptr dovrebbe contenere la stringa .mhtml
	 */
	char *ptr = strrchr(REF, '.');

	/* Effettuo il controllo per assicurarmi che il file
	 * abbia estensione .mhtml
	 */
	if ((strncmp(ptr, ".mhtml", strlen(".mhtml")) == 0))
		return (TRUE);

	return (FALSE);
}


/* hasDoublePoints()
 * Devono esistere solo due "due punti" (:), uno prima del doppio slash (mhttp://)
 * e uno per indicare la porta (:444).
 * @RETURN: TRUE per indicare che esistono entrambi,
 *          FALSE per indicare che esiste uno/meno/piu'
 */
boolean hasDoublePoints(char *REF)
{
	int i;
	int cont = 0;
	
	for (i=0; i<strlen(REF); i++) {
		if (REF[i] == ':')
			cont++;
	}

	if (cont == 2)
		return (TRUE);

	return (FALSE);
}


/* getPort()
 * Estrae la porta utilizzata
 * @REF:    identifica l'indirizzo completo
 * @RETURN: Numero di porta (-1 in caso di errore)
 */
int getPort(char *REF)
{
	char port[10];
	char *ptr;
	int i, j=0;

	/* Cerco l'ultima occorrenza del carattere ":" in address.
	 * Adesso *ptr contiene tutto cio' che segue dal numero di porta in poi..
	 */
	if ((ptr = strrchr(REF, ':')) == NULL)
		return (0);

	/* i parte da 1 per trascurare il primo carattere ":" */
	for (i=1; i<strlen(REF); i++) {
			
		/* quando incontro il carattere "/" significa che sta iniziando
		 * il percorso della risorsa e quindi non fa parte del numero di porta;
		 * percio' appena si incontra il carattere "/" bisogna interrompere il ciclo for.
		 */
		if (ptr[i] == '/') {
			break;
		}
			
		port[j] = ptr[i];
		j++;
	}
		
	port[j] = '\0';	/* terminatore di stringa */
	
	return (atoi(port));
}


/* getResource()
 * Estrae la risorsa a cui si vuole accedere.
 * @REF:      identifica l'indirizzo completo
 * @resource: buffer in cui memorizzare la risorsa
 * @RETURN:   1 per indicare che la risorsa e' stata ottentua con successo,
 *            0 altrimenti.
 */
int getResource(char *REF, char *resource)
{
	char *ptr;
	int i = 0; 
	int j = 0;
	boolean OK_COPY = FALSE;

	/* Cerco l'ultima occorrenza del carattere ":" in address
	 * dopo ptr dovrebbe contenere ':port/resource.mhtml'
	 */
	if ((ptr = strrchr(REF, ':')) == NULL)
		return (0);

	/* inizializzazione della struttura in cui verra' memorizzata la risorsa */
	memset((void*)resource, 0x00, MAXLENPATH);

	for (i=1; i<strlen(REF); i++) {
		if ((ptr[i] == '/') && (OK_COPY == FALSE)) {
			OK_COPY = TRUE;
			i++; 	/* incremento per evitare il 1° carattere "/" */
		}

		if (OK_COPY == TRUE) {
			resource[j] = ptr[i];
			j++;
		}
		resource[j] = '\0';
	}
	
	/* controllo che la lunghezza della risorsa non superi un
	 * certo numero di caratteri (MAXLENPATH = 2048)
	 */
	if (strlen(resource) > MAXLENPATH)
		return (0);

	return (1);
}


/* getIPaddress()
 * Estrae l'indirizzo IP.
 * @REF:    identifica l'indirizzo completo
 * @ipaddr: buffer in cui memorizzare l'indirizzo.
 * @RETURN: 1 per indicare che l'indirizzo e' stato ottenuto con successo,
 *          0 altimenti
 */
int getIPaddress(char *REF, char *ipaddr)
{
	int i = 0;
	int j = 0;
	char *ptr;

	/* Trovo la prima occorenza del carattere /
	 * A questo punto *ptr dovrebbe contienre: "//address:port/resource.mhtml" */
	if ((ptr = strchr(REF, '/')) == NULL)
		return (0);
	
	memset((void*)ipaddr, 0x00, ADDRLEN);

	/* i parte da 2 per saltare i primi due caratteri // */
	for (i=2; i<strlen(REF); i++) {
		/* Mi fermo quando arrivo al carattere ":", prima dell'inizio
		 * della porta. */
		if (ptr[i] == ':')
			break;
		ipaddr[j] = ptr[i];
		j++;
	}

	ipaddr[j] = '\0';
	return (1);
}
