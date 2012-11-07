/*
 * reqparser.c
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#ifndef __REQPARSER_H__
#define __REQPARSER_H__

#include <pconst.h>

/* isGET()
 * La funzione verifica se un messaggio e' di tipo GET, ed in quel caso
 * restituisce 'TRUE', altrimenti FALSE.
 */
boolean isGET(char *request);

/* isINF()
 * La funzione verifica se una ricchiesta e' di tipo INF, ed in quel caso
 * restituisce 'TRUE', altrimenti FALSE.
 */
boolean isINF(char *request);

/* getReqType()
 * La funzione memorizza all'interno del parametro 'type' il tipo di richiesta
 * (GET o INF), altrimenti type vale 'ERR'.
 */
void getReqType(char *request, char *type);

/* getReqAddress()
 * Estrae la risorsa che si vuole scaricare.
 */
int getReqAddress(char *request, char *address);

/* getReqRange()
 * Estrae il range dalla richiesta del server.
 */
int getReqRange(char *request, int *start, int *end);

#endif /* __REQPARSER_H__ */
