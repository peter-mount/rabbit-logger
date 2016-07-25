
/* 
 * File:   rabbit-logger.h
 * Author: peter
 *
 * Created on 25 July 2016, 08:33
 */

#ifndef RABBIT_LOGGER_H
#define RABBIT_LOGGER_H

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <area51/list.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct Log {
        struct Node node;
        // File path
        char *path;
        // Routing key to subscribe to
        char *routingKey;
        // The rotation period in seconds
        int rotatePeriod;
        // Current output file
        FILE *file;
        // WHen the file was last created
        time_t fileTime;
        // Time used to generate file
        struct tm tm;
        // Mutex
        pthread_mutex_t mutex;
    };

    extern struct List loggers;
    
    extern void initLoggers();
    
#ifdef __cplusplus
}
#endif

#endif /* RABBIT_LOGGER_H */

