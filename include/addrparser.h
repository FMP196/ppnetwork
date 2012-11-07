/*
 * addrparser.h
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#ifndef __ADDRPARSER_H__
#define __ADDRPARSER_H__

#include <pconst.h>

/* IS_CORRECT()
 * Veririca se l'indirizzo passatogli e' corretto, ovvero contiene
 * tutte le informazioni necessarie per reperire la risors.
 * @REF: identifica l'indirizzo completo (ad es. mhttp://123.123.123.123:12/risorsa.html)
 */
boolean IS_CORRECT(char *REF);

/* isMHTTP()
 * Verifica che l'indirizzo sia un indirizzo del protocollo mhttp.
 * @REF: identifica l'indirizzo completo.
 * @return TRUE (1) in caso di successo, FALSE (0) in caso di errore.
 */
boolean isMHTTP(char *REF);

/* isMHTML()
 * Verifica se la risorsa sia un file .mhtml
 * @REF:    identifica l'indirizzo completo.
 * @return: TRUE (1) in caso di successo, FALSE (0) altrimenti.
 */
boolean isMHTML(char *REF);

/* hasDoublePoints()
 * Devono esistere solo due "due punti" (:), uno prima del doppio slash (://)
 * e uno per indicare la porta.
 * @return: TRUE per indicare che esistono entrambi,
 *          FALSE per indicare che esiste uno o meno.
 */
boolean hasDoublePoints(char *REF);

/* getIPaddress()
 * Estrae l'indirizzo IP.
 * @REF:     identifica l'indirizzo completo
 * @ipaddr:  buffer in cui memorizzare l'indirizzo.
 * @return:  1 per indicare che l'indirizzo e' stato ottenuto con successo,
 *           0 altimenti
 */
int getIPaddress(char *REF, char *ipaddr);

/* getPort()
 * Estrae la porta utilizzata
 * @REF: identifica l'indirizzo completo
 * @return:  numero di porta in caso di successo, -1 altrimenti
 */
int getPort(char *REF);

/* getResource()
 * Estrae la risorsa a cui si vuole accedere.
 * @REF:      identifica l'indirizzo completo
 * @resource: buffer in cui memorizzare la risorsa
 * @return    1 per indicare che la risorsa e' stata ottentua con successo,
 *            0 altrimenti.
 */
int getResource(char *REF, char *resource);

#endif /* __ADDRPARSER_H__ */
