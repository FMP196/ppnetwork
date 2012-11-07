/*
 * cache.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#include <cache.h>
#include <xlib.h>
#include <util.h>


/* cache_insert()
 * Inserisce un elemento in testa alla cache.
 */
int cache_insert(struct cache_t **cache, char *resource, char *response, time_t endtime, struct server_t* sid, pthread_mutex_t *mutex, boolean active)
{
 if (active == TRUE)
 {
	struct cache_t *temp;
	
	if ((temp = (struct cache_t*)malloc(sizeof(struct cache_t))) == NULL)
		return (0);

	temp->endtime = endtime;
	temp->sid.port = sid->port;
	strcpy(temp->sid.address, sid->address);
	strcpy(temp->resource, resource);
	strcpy(temp->response, response);
	
	pthread_mutex_lock(mutex);
	temp->next = (*cache);
	(*cache) = temp;
	pthread_mutex_unlock(mutex);
	
	return (1);
 }
 return (0);
}


/* cache_exists()
 * Verifica se una risorsa e' in cache.
 * La funzione ritorna 0 se la risorsa non esite in cache, 1 altrimenti e in questo
 * caso viene memorizzato nel 'buffer' il valore contenuto all'interno della
 * cache.
 */
int cache_exists(struct cache_t *cache, char *resource, char *buffer, struct server_t *sid, pthread_mutex_t *mutex, boolean active)
{
 if (active == TRUE)
 {
	time_t now;
	pthread_mutex_lock(mutex);
	while (cache != NULL)
	{
		/* verifico che la risorsa sia in cache, e mi assicuro che il file richiesto
		 * sia proprio quello del server a cui sto facendo riferimento.
		 */
		if (((strcmp(cache->resource, resource)) == 0) && 
				((strcmp(cache->sid.address, sid->address)) == 0) && 
					(cache->sid.port == sid->port))
		{
			time(&now); /* tempo corrente */
			if ((cache->endtime > now)) {
				strcpy(buffer, cache->response);
				pthread_mutex_unlock(mutex);
				return (1);
			}
			else {
				pthread_mutex_unlock(mutex);
				return (0);
			}
		}
		else
			cache = cache->next;
	}
	pthread_mutex_unlock(mutex);
 }
 return (0);
}



/* cache_remove_expiry()
 * Rimuovo dalla cache tutte le informazioni scadute.
 */
void cache_remove_expiry(struct cache_t **cache, pthread_mutex_t *mutex, boolean active)
{
 if (active == TRUE)
 {
	time_t now;
	struct cache_t *temp = NULL;
	pthread_mutex_lock(mutex);
	while ((*cache) != NULL)
	{
		time(&now);
		if (((*cache)->endtime < now)) 
		{
			temp = (*cache)->next;
			free(*cache);
			*cache = temp;
		}
		else
			cache = &(*cache)->next;
	}
	pthread_mutex_unlock(mutex);
 }
}


/* cache_destroy()
 * Svuota la cache.
 */
void cache_destroy(struct cache_t **cache, pthread_mutex_t *mutex, boolean active)
{
 if (active == TRUE)
 {
	struct cache_t *temp;
	struct cache_t *ptr = *cache;	
	
	pthread_mutex_lock(mutex);
	while (ptr != NULL) {
		temp = ptr->next;
		free(ptr);
		ptr = temp;
		ptr = ptr->next;
	}
	(*cache) = NULL;
	pthread_mutex_unlock(mutex);
 }
}


/* cache_print()
 * Stampa tutte le informazioni contenute nella cache.
 */
void cache_print(struct cache_t *cache)
{
	time_t now;
	long int valid = 0;
	int cont = 0;

	fprintf(stdout, "%s", CLEARSCREEN);
	fprintf(stdout, "\n  [%sINTERNAL CACHE%s] %s%s %s%s\n", RED, DEFAULTCOLOR, BLUE, PROG_NAME, PROG_VERSION, DEFAULTCOLOR);
	fprintf(stdout, " ______________________________________________________________\n\n");

	if (cache == NULL)
		return;

	while (cache != NULL)
	{
		cont++;
		time(&now);
		fprintf(stdout, "  [%s*%s] ", RED, DEFAULTCOLOR);
		if (cache->endtime > now) {
			valid = (cache->endtime - now);
			if (valid < 10)
				fprintf(stdout, "[%sVALID (+0%li)%s] ", BLUE, valid, DEFAULTCOLOR);
			else
				fprintf(stdout, "[%sVALID (+%li)%s] ", BLUE, valid, DEFAULTCOLOR);
		}
		else
			fprintf(stdout, "[ %sNOT VALID%s ] ", RED, DEFAULTCOLOR);

		fprintf(stdout, "Resource %s%s%s:", BLUE, cache->sid.address, DEFAULTCOLOR);
		fprintf(stdout, "%s%d%s", BLUE, cache->sid.port, DEFAULTCOLOR);
		fprintf(stdout, "/%s%s%s\n", BLUE, cache->resource, DEFAULTCOLOR);
		
		fflush(stdout);
		cache = cache->next;
	}

	fprintf(stdout, " ______________________________________________________________\n\n");
	fprintf(stdout, "  [%s*%s] %s%d%s resources in cache ", RED, DEFAULTCOLOR, BLUE, cont, DEFAULTCOLOR);	
	fflush(stdout);
}
