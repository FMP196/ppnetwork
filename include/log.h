/*
 * log.h
 * (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <pconst.h>

/* log_init()
 */
void log_init(char *logfile, boolean logging);

/* log_request()
 * Inserisce nel logfile una richiesta da parte del client.
 */
void log_request(char *logfile, char *request, char *addr, int port, boolean logging);

/* log_cache()
 * Inserisce nel logfile un'informazione che indica la presenza
 * o meno della risorsa in cache.
 */
void log_cache(char *logfile, char *resource, boolean exists, boolean logging);

/* log_response()
 * Inserisce nel logfile la risposta del server
 */
void log_response(char *logfile, char *msg, char *resource, char *saddr, char *caddr, boolean logging);

#endif /* __LOG_H__ */
