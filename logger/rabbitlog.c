
/* 
 * File:   rabbitlog.c
 * Author: peter
 *
 * Created on 25 July 2016, 08:29
 */

#include <stdio.h>
#include <stdlib.h>
#include <area51/json.h>
#include <area51/log.h>
#include <area51/math.h>
#include <area51/rabbitmq.h>
#include <rabbit-logger.h>
#include <fcntl.h>
#include <unistd.h>
#include <area51/charbuffer.h>
#include <area51/curl.h>

#include "area51/main.h"

struct logging {
    RabbitConsumer *con;
    struct Statistic stat;
    time_t time;
    char *name;
    // boolean Log to console
    bool log;
    // iot.onl api key
    char *iotApiKey;
    // thingspeak api key
    char *thingSpeakApiKey;
};

static void logGet(char *url, char *keyName, char *keyVal, char *countName, int count) {
    CharBuffer *b = charbuffer_new();
    if (b) {
        charbuffer_printf(b, "%s?%s=%s&%s=%d", url, keyName, keyVal, countName, count);
        int len;
        char *url = charbuffer_tostring(b, &len);
        logconsole("logGet %s", url);
        curl_get(url, b);
        free(url);
        charbuffer_free(b);
    }
}

static void logStats(MainTask *t) {
    struct logging *logging = area51_mainGetUserData(t);
    rabbitmq_getStatistic(logging->con, &logging->stat);
    time(&logging->time);

    int count = logging->stat.count;

    logconsole("start");
    
    if (logging->log)
        logconsole("%03d %s %d", 0, "x", count);

    if (logging->iotApiKey)
        logGet("https://iot.onl/update", "api_key", logging->iotApiKey, "field1", count);

    if (logging->thingSpeakApiKey)
        logGet("https://api.thingspeak.com/update", "api_key", logging->thingSpeakApiKey, "field1", count);

    logconsole("end");
}

static void addTasks(RabbitMQ *mq, RabbitConsumer *con, struct json_object *o, void *c) {

    struct json_object *obj;
    if (json_object_object_get_ex(o, "logging", &obj)) {
        struct logging *logging = malloc(sizeof (struct logging));
        if (logging) {
            memset(logging, 0, sizeof (struct logging));
            
            logging->con = con;

            char *s = json_getString(obj, "name");
            if (s)
                logging->name = strdup(s);
            if(!logging->name) {
                
            }
            // Log to the console
            logging->log = json_getBoolean(obj, "log");

            s = json_getString(obj, "iot.onl");
            if (s)
                logging->iotApiKey = strdup(s);

            s = json_getString(obj, "thingspeak");
            if (s)
                logging->thingSpeakApiKey = strdup(s);

            // Update period
            int period = json_getInt(obj, "period");
            if (period < 1)
                period = 60;
            area51_mainRunPeriodic((MainTasks *) c, logStats, period, logging, NULL);
        }
    }
}

static RabbitMQ *config(char *confFile, MainTasks *main) {
    struct json_object *config = NULL;
    RabbitMQ *mq = NULL;

    int fdsock = open(confFile, O_RDONLY);
    if (fdsock < 0) {
        logconsole("Failed to open %s", confFile);
    } else {
        config = json_parse_file(fdsock);
        close(fdsock);

        if (config) {
            mq = rabbitmq_new(config, NULL, NULL, main);
            json_object_put(config);
        }
    }

    return mq;
}

int main(int argc, char** argv) {

    if (argc != 2) {
        logconsole("usage: %s {config.json}", argv[0]);
        return EXIT_FAILURE;
    }

    curl_pool_init(10);
    
    MainTasks *main = area51_mainInit();
    area51_mainSetVerbosity(main, 0);
    area51_mainSetPeriod(main, 5);

    char *confFile = argv[1];
    logconsole("Reading %s", confFile);

    RabbitMQ *mq = config(confFile, main);

    if (!mq) {

        logconsole("Failed to configure RabbitMQ");
        return EXIT_FAILURE;
    }

    rabbitmq_connect(mq);

    area51_mainLoop(main);

    logconsole("Finish");
    return EXIT_SUCCESS;
}

