/*
 * proxy.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <ctype.h>
#include <getopt.h>

#include <util.h>
#include <xlib.h>
#include <pconst.h>
#include <addrparser.h>
#include <resparser.h>
#include <reqparser.h>
#include <cache.h>
#include <log.h>


int     contclient   = 0;
char    *LOGFILE     = NULL;
boolean logging      = FALSE;
boolean print_cache  = FALSE;
boolean active_cache = TRUE;
boolean cache_level3 = FALSE;

pthread_cond_t  condition  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexdata  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexcache = PTHREAD_MUTEX_INITIALIZER;

struct cache_t *cache;

/* Struttura parametri Thread */
typedef struct thparams {
	int clientsd;
	struct sockaddr_in client;
} ThParams;


/* Prototipi di Funzioni */
void *new_client(void *ptr);
void *print_remove_cache(void *ptr);


int main(int argc, char *argv[])
{
	struct sockaddr_in proxy;      /* struttura proxy  */
	struct sockaddr_in client;     /* struttura client */
	struct thparams *thp;          /* struttura i parametri al thread */

	pthread_t mythread;

	pthread_attr_t attribute;      /* attributi del thread */
	socklen_t clientsize;          /* dimensione indirizzo del client */

	int proxysd;                   /* socket descriptor del proxy */
	int clientsd;                  /* socket descriptor del client */
	int optvalue;                  /* opzione socket */
	int localport;                 /* porta proxy */
	int maxclients;                /* numero max. di clienti che possono connettersi */
	int choice;
	
	localport = LOCALPORT;
	maxclients = MAXCLIENTS;

	/* Lettura parametri del Proxy */
	while (TRUE)
	{
		int index = 0;
		static struct option long_options[] = 
			{{"help",         no_argument,       0, 'h'},
		    {"maxclient" ,   required_argument, 0, 'm'},
		    {"port",         required_argument, 0, 'p'},
		    {"log",          required_argument, 0, 'l'},
		    {"print-cache",  no_argument,       0, 'k'},
		    {"cache-level3", no_argument,       0, '3'},
		    {"no-caching",   no_argument,       0, 'n'}};
		                                       
		if ((choice = getopt_long(argc, argv, "hm:p:l:k3n", long_options, &index)) == -1)
			break;

		switch (choice)
		{
			case 'h':
				usage(argv[0]);
				break;
			case 'p':
				localport = atoi(optarg);
				break;
			case 'm':
				maxclients = atoi(optarg);
				break;
			case 'l':
				logging = TRUE;
				LOGFILE = optarg;
				break;
			case 'k':
				print_cache = TRUE;
				break;
			case '3':
				cache_level3 = TRUE;
				break;
			case 'c':
				active_cache = FALSE;
				cache_level3 = FALSE;
				print_cache = FALSE;
				break;
			case '?':
			default :
				usage(argv[0]);
		}
	}

	/* Argomenti non validi */
	if (optind < argc) {
		fprintf(stdout, "%s: invalid argument: ", argv[0]);
		while (optind < argc)
			fprintf(stdout, "`%s' ", argv[optind++]);
		fprintf(stdout, "\n");
		usage(argv[0]);
	}

	/* Creazione socket locale locale */
	if ((proxysd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		errquit("PROXY[main]: socket()");

	/* Inizializzazione della struttura locale */
	memset((void*)&proxy, 0, sizeof(proxy));
	proxy.sin_family = AF_INET;
	proxy.sin_port = htons(localport);
	proxy.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Impostazioni specifiche per il socket */
	optvalue = 1;
	if ((setsockopt(proxysd, SOL_SOCKET, SO_REUSEADDR, (char*)&optvalue, sizeof(optvalue))) < 0)
		errquit(" PROXY[main]: setsockopt()");

	/* Associamo al socket un processo */
	if ((bind(proxysd, (struct sockaddr*)&proxy, sizeof(proxy))) < 0)
		errquit(" PROXY[main]: bind()");

	/* rendo i thread detached */
	pthread_attr_init(&attribute);
	pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);

	/* inizializzazione della struttura per il logging */
	log_init(LOGFILE, logging);

	if (active_cache == TRUE) {
		if ((pthread_create(&mythread, &attribute, print_remove_cache, NULL)) < 0)
			errquit("PROXY[main] pthread_create()");
	}
	
	/* Una volta creato il socket bisogna metterlo in ascolto
	 * per eventuali richieste.
	 */
	listen(proxysd, maxclients);

	while (TRUE)
	{
		pthread_t thread;
		clientsize = sizeof(client);

		if ((clientsd = accept(proxysd, (struct sockaddr*)&client, &clientsize)) < 0)
			errquit("PROXY[main]: accept()");	
		
		thp = (struct thparams*)malloc(sizeof(struct thparams));
		thp->clientsd = clientsd;
		thp->client = client;

		if ((pthread_create(&thread, &attribute, new_client, (void*)thp)) != 0) {
			close(thp->clientsd);
			free(thp);
		}
	}

	close(proxysd);

	pthread_mutex_destroy(&mutexdata);
	pthread_mutex_destroy(&mutexcache);
	pthread_cond_destroy(&condition);
	pthread_attr_destroy(&attribute);

	return (EXIT_SUCCESS);
}


/* new_client()
 * Funzione da associare ad ogni nuovo client connesso.
 * @ptr: e' una struttura dati che contiene il socket descriptor del client
 * appena connesso, e la struttura del client.
 */
void *new_client(void *ptr) 
{
	struct thparams *thp = (struct thparams*)ptr;
	struct server_t *sid;
	
	char request[MAXLENREQ];
	char response[MAXLENRESP];
	char resource[MAXLENPATH];
	char REF[MAXLENREF];
	char IDX[MAXIDXLEN];
	char data[MAXLENDATA];
	char newdata[MAXLENDATA];

	time_t now;
	char *clientaddr;
	int clientport;
	int expire;
	int code;
	int nread;
	int numresources;
	int otheresources;
	int tentativo;
	int i, j;

	pthread_mutex_lock(&mutexdata);
	clientaddr = inet_ntoa(thp->client.sin_addr);
	clientport = htons(thp->client.sin_port);
	pthread_mutex_unlock(&mutexdata);

	if ((sid = (struct server_t*)malloc(sizeof(struct server_t))) == NULL)
		errquit("PROXY[new_client]: malloc()");

	/* ricezione richiesta dal client */
	nread = data_receive_from_client(thp->clientsd, request, MAXLENREQ);

	getReqAddress(request, REF);		/* estrazione dell'indirizzo (mhttp://127.../k.mhtml) */
	
	/* verifico se il client mi ha mandato una richiesta
	 * ben formata (ovvero che segue le specifiche del protocollo),
	 * altrimenti la comunicazione con il client viene chiusa.
	 */
	if (!IS_CORRECT(REF))
	{
		log_request(LOGFILE, request, clientaddr, clientport, logging);
		close(thp->clientsd);
		free(thp);
		pthread_exit((void*)0);
	}
		
	getResource(REF, resource);      /* estrazione della risorsa (k.mhtml) */
	sid->port = getPort(REF);        /* estrazione porta del server */
	getIPaddress(REF, sid->address); /* estrazione indirizzo IP del server */
	log_request(LOGFILE, request, clientaddr, clientport, logging);

	/* verifico se la risorsa richiesta dal client e' contenuta
	 * in cache ed e' ancora valida
	 */
	if (cache_exists(cache, resource, response, sid, &mutexcache, active_cache)) {
		data_send(thp->clientsd, response);
		close(thp->clientsd);
		log_cache(LOGFILE, resource, TRUE, logging);
	}
	else
	{
		pthread_mutex_lock(&mutexdata);
		while (contclient > 4) {
			pthread_cond_wait(&condition, &mutexdata);
		}
		contclient++;
		pthread_cond_broadcast(&condition);
		pthread_mutex_unlock(&mutexdata);
		
		log_cache(LOGFILE, resource, FALSE, logging);

		tentativo = 0;
		while (tentativo < 4) {
			if ((nread = data_receive_from_server(sid, request, response)) != 0) {
				time(&now);
				if (nread == -1)
					log_response(LOGFILE, "Timeout scaduto. Server bloccato!", resource, sid->address, clientaddr, logging);
				else if (nread == -2)
					log_response(LOGFILE, "Impossibile connettersi al server!", resource, sid->address, clientaddr, logging);
				break;
			}
			if (nread == 0)
				log_response(LOGFILE, "Errore nei dati ricevuti! Riprovo ...", resource, sid->address, clientaddr, logging);
			tentativo++;
		}
		
		/* i dati sono stati ricevuti correttamente */
		if (nread > 0)
		{
			code = getResponse(response);

			if (code == 200)
			{
				/* inivio la risposta al client */
				data_send(thp->clientsd, response);
				close(thp->clientsd);

				log_response(LOGFILE, "200 OK", resource, sid->address, clientaddr, logging);

				/* caching di 1°, 2° livello */
				if (active_cache == TRUE)
				{
					getResContent(response, data);
					expire = getResExpire(response);

					/* inserisco la risorsa in cache se non esiste */
					if (!(cache_exists(cache, resource, response, sid, &mutexdata, active_cache)))
						cache_insert(&cache, resource, response, (now + expire), sid, &mutexcache, active_cache);

					/* estraggo il numero di risorse dai dati ricevuti dal client */
					numresources = getNumResources(data);

					/* PARSING DI 1° e 2° LIVELLO */
					for (i=0; i<numresources; i++)
					{
						getINFO(data, IDX, REF, i);
						getResource(REF, resource);
						getIPaddress(REF, sid->address);
						sid->port = getPort(REF);

						if (!(cache_exists(cache, resource, response, sid, &mutexcache, active_cache)))
						{
							if ((request_resource_to_server(sid, REF, response) > 0))
							{
								time(&now);
								expire = getResExpire(response);
								cache_insert(&cache, resource, response, (now+expire), sid, &mutexcache, active_cache);

								/* caching di 3° livello */
								if (cache_level3 == TRUE)
								{
									getResContent(response, newdata);         /* estraggo la parte dati dalla rispsota */
									otheresources = getNumResources(newdata); /* estraggo il numero di risorse */

									/* PARSING DI 3° LIVELLO */
									for (j=0; j<otheresources; j++) 
									{
										getINFO(newdata, IDX, REF, j);
										getResource(REF, resource);
										getIPaddress(REF, sid->address);
										sid->port = getPort(REF);

										/* il parsing del 3° livello va effettuato solo sulle risorse
										 * IDX+REF, per cui mi assicuro che IDX sia valido. */
										if ((strcmp(IDX, NOT_VALID)) != 0)
											if (!(cache_exists(cache, resource, response, sid, &mutexcache, active_cache)))
												if ((request_resource_to_server(sid, REF, response) > 0)) {
													time(&now);
													expire = getResExpire(response);
													cache_insert(&cache, resource, response, (now+expire), sid, &mutexcache, active_cache);
												}
									}
									memset((void*)sid->address, 0x00, ADDRLEN);
								}	
							}
						}
						memset((void*)sid->address, 0x00, ADDRLEN);
					}
				}
			}
			else if (code == 201)
				log_response(LOGFILE, "201; OK RANGE", resource, sid->address, clientaddr, logging);
			else if (code == 201)
				log_response(LOGFILE, "202; INFO", resource, sid->address, clientaddr, logging);	
			else if (code == 402)
				log_response(LOGFILE, "402; UNKNOWN ERROR", resource, sid->address, clientaddr, logging);		
			else if (code == 403)
				log_response(LOGFILE, "403; WRONG REQUEST", resource, sid->address, clientaddr, logging);
			else if (code == 404) 
				log_response(LOGFILE, "404; FILE NOT FOUND", resource, sid->address, clientaddr, logging);
			else if (code == 405)
				log_response(LOGFILE, "405; INTERVAL NOT FOUND", resource, sid->address, clientaddr, logging);
		}
		
		pthread_mutex_lock(&mutexdata);
		contclient--;
		pthread_mutex_unlock(&mutexdata);
	}
	
	close(thp->clientsd);

	free(thp);
	pthread_exit((void*)0);
}



void *print_remove_cache(void *ptr)
{
	while (TRUE) 
	{
		if (print_cache == TRUE)
			cache_print(cache);
		
		sleep(1);
		
		/* rimozione di tutte le risorse scadute */
		cache_remove_expiry(&cache, &mutexcache, active_cache);
	}

	pthread_exit((void*)0);
}
