/*
 * cache.h
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#ifndef __CACHE_H__
#define __CACHE_H__

#include <pconst.h>

/* cache_insert_order()
 * Inserimento di un elemento in cache.
 * @cache:    puntatore alla struttura della Cache
 * @resource: nome della risorsa che si intende inserire in cache
 * @response: risposta del server
 * @endtime:  tempo in cui finisce la validita' della risorsa
 * @addr:     indirizzo del server di cui appartiene la risorsa
 * @port:     porta del server a cui appartiene la risorsa
 * @mutex:    mutex per gestire la mutua esclusione all'interno della cache
 * @RETURN:   la funzione ritorna un 1 per indicare che la risorsa e' stata
 *            aggiunta in cache, 0 in caso di errore.
 */
int cache_insert(struct cache_t **cache, char *resource, char *response, time_t endtime, struct server_t *sid, pthread_mutex_t *mutex, boolean active);

/* cache_exists()
 * Verifica se una risorsa e' in cache e che sia ancora valida (ovvero non e'
 * scaduto il suo expire time.
 * @cache:    puntatore alla struttura della Cache
 * @resource: nome della risorsa che si cercare
 * @buffer:   buffer in cui memorizzare il messaggio estratto dalla cache
 * @addr:     indirizzo del server a cui deve appartiene la risorsa
 * @port:     porta del server a cui deve appartiene la risorsa
 * @mutex:    mutex per gestire la mutua esclusione all'interno della cache
 * @RETURN:   la funzione ritorna un 1 per indicare che la risorsa esiste in cache
 *            ed e' ancora valida, 0 altrimenti.
 */
int cache_exists(struct cache_t *cache, char *resource, char *buffer, struct server_t *sid, pthread_mutex_t *mutex, boolean active);

/* cache_remove_expire()
 * Rimuovo dalla cache tutte le informazioni scadute.
 * @cache:   puntatore alla struttura della Cache
 * @mutex:   mutex per gestire la mutua esclusione all'interno della cache
 * @RETURN:  la funzione ritorna un 1 per indicare che la risorsa e' stata
 *           aggiunta in cache, 0 in caso di errore.
 */
void cache_remove_expiry(struct cache_t **cache, pthread_mutex_t *mutex, boolean active);

/* cache_destroy()
 * Rimuove tutti gli elementi dalla cache.
 * @cache:   puntatore alla struttura della cache
 */
void cache_destroy(struct cache_t **cache, pthread_mutex_t *mutex, boolean active);

/* cache_print()
 * Stampa tutte le informazioni contenute nella cache.
 */
void cache_print(struct cache_t *cache);

#endif /* __CACHE_H__ */
