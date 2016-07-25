
/* 
 * File:   rabbitlog.c
 * Author: peter
 *
 * Created on 25 July 2016, 08:29
 */

#include <stdio.h>
#include <stdlib.h>
#include <area51/log.h>
#include <rabbit-logger.h>

/*
 * 
 */
int main(int argc, char** argv) {
    logconsole("Rabbit Logger");
    
    initLoggers();
    return (EXIT_SUCCESS);
}

