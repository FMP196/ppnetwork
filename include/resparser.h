/*
 * resparser.h
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#ifndef __RESPARSER_H__
#define __RESPARSER_H__

#include <pconst.h>

/* Codici di risposta del server */
boolean is200(char *response);   /* OK */
boolean is201(char *response);   /* OK RANGE */
boolean is202(char *response);   /* INFO */
boolean is402(char *response);   /* UNKNOWN ERROR */
boolean is403(char *response);   /* WRONG REQUEST */
boolean is404(char *response);   /* FILE NOT FOUND */
boolean is405(char *response);   /* INTERVAL NOT FOUND */

/* getResponse()
 * Ottiene il codice mhttp di risposta da parte del server.
 */
int getResponse(char *response);

/* getResLen()
 * Ottiene la lunghezza in byte della dimensione del file
 * che il server ha inviato.
 */
int getResLen(char *response);

/* getResRange()
 * Ottiene il range della risorsa che si vuole scaricare.
 */
int getResRange(char *response, int *start, int *end);

/* getResExpire()
 * Ottiene il tempo di expire della risorsa.
 */
int getResExpire(char *response);

/* getResContent()
 * Estrae i dati da inivare al client, dalla risposta del server.
 */
void getResContent(char *response, char *buffer);

/* getNumResources()
 * @return: numero di risorse che si trovano nel testo (data).
 */
int getNumResources(char *data);

/* getINFO()
 * Estrare IDX e REF da un testo. Se l'IDX non e' presente
 * allora vale NULL.
 * @text:  testo su cui effettuare il parsing
 * @IDX:   buffer in cui memorizzare IDX
 * @REF:   buffer in cui memorizzare REF
 * @index: indica l'indice da cui partire nel testo; questo
 * valore deve contenere la posizione dell'ultimo IDX visitato.
 */
void getINFO(char *text, char *IDX, char *REF, int index);

/* getREF()
 * Estrae l'indirizzo della risorsa.
 * @text: testo su cui effetare il parsing (nella forma: <IDX=3;REF=mhttp://1.1.1.1.1:1/1.mhtml>
 * @REF: buffer in cui memorizzare REF
 */
void getREF(char *text, char *REF);

/* getIDX()
 * Estrae il numero IDX della risorsa.
 * @text: testo su cui effetare il parsing (nella forma: <IDX=3;REF=mhttp://1.1.1.1.1:1/1.mhtml>
 * @IDX: buffer in cui memorizzare IDX
 */
void getIDX(char *text, char *IDX);

#endif /* __RESPARSER_H__ */
