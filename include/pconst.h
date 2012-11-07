/*
 * pconst.h
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#ifndef __PCONST_H__
#define __PCONST_H__

#include <sys/types.h>

/* Tipo di dato BOOLEAN */
#define TRUE   1
#define FALSE  0
typedef unsigned short boolean;

#define MAXNUMEMBEDDEDOBJECT       10
#define INACTIVITY_TIMEOUT_SECONDS 10

#define MAXLENREQ    ((MAXLENPATH)+(1024))
#define MAXLENRESP   ((MAXLENREQ)+(MAXLENDATA))
#define MAXLENREF    ((MAXLENPATH)+(ADDRLEN))
#define MINLENGTH    (strlen("mhttp://0.0.0.0:0/0.mhtml"))

#define MAXLENPATH   2048
#define MAXLENDATA   5000
#define ADDRLEN      30
#define MAXTYPLEN    5
#define MAXIDXLEN    5
#define MAXNUMLINK   10

#define MAXCLIENTS   500
#define MINPORT      1024
#define MAXPORT      65535
#define LOCALPORT    55554
#define BUFFSIZE     200

#define MAXLENTIME   80
#define NOT_VALID    "ERR"

#define PROG_NAME    "PP/Network"
#define PROG_VERSION "v1.0"

/* Identifica un server in modo univoco, rappresentandolo
 * una coppia di valori: Address:Port.
 */
typedef struct server_t {
	int  port;                   /* porta del server */
	char address[ADDRLEN];       /* indirizzo IP server */
} ServerID;

/* Struttura della cache in cui vengono memorizzate le
 * informazioni richieste dai client.
 */
typedef struct cache_t {
	time_t endtime;              /* scadenza della risorsa */
	char   resource[MAXLENPATH]; /* nome della risorsa */
	char   response[MAXLENRESP]; /* risposta del server */
	struct server_t sid;         /* identificativo server */
	struct cache_t *next;
} Cache;

#endif  /* __PCONST_H__ */
