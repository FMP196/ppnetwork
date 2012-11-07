/*
 * xlib.h
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#ifndef __XLIB_H__
#define __XLIB_H__

#include <cache.h>
#include <pconst.h>

/* errquit()
 * Stampa il relativo messaggio d'errore ed esce.
 * @msg: messaggio da stampare
 */
void errquit(char *msg);

/* usage()
 * Mostra il modo corretto di inserire i parametri per l'avvio del proxy.
 */
void usage(char *progname);

/* server_connect()
 * Connessione con il server.
 */
int server_connect(struct server_t *sid);

/* request_resource_server()
 * Chiede una risorsa al server.
 */
int request_resource_to_server(struct server_t *sid, char *REF, char *response);

/* data_receive_from_server()
 * Riceve i dati dal server e li memorizza all'interno di un buffer.
 * @RETURN:  n (dove n > 0) numero di byte ricevuti
 *           0 i dati sono arrivati ma non sono completi
 *          -1 timeout scaduto (server bloccato)
 *          -2 impossibile connettersi al server
 */
int data_receive_from_server(struct server_t *sid, char *request, char *response);

/* data_receive_from_client()
 * Riceve i dati dal client e li memorizza all'interno di un buffer.
 * @return: numero di byte letti
 */
int data_receive_from_client(int sd, char *buffer, int size);

/* data_send()
 * Invia i dati all'interno del 'buffer' al socket descriptor 'sd'.
 */
int data_send(int sd, char *buffer);

/* set_socket_nonblock()
 * Imposta il socket come non bloccante.
 */
void set_socket_nonblock(int *sd);

/* set_socket_block()
 * Imposta il socket come bloccante.
 */
void set_socket_block(int *sd);

#endif /* _XLIB_H_ */
