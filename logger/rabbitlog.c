
/* 
 * File:   rabbitlog.c
 * Author: peter
 *
 * Created on 25 July 2016, 08:29
 */

#include <stdio.h>
#include <stdlib.h>
#include <area51/filelogger.h>
#include <area51/json.h>
#include <area51/log.h>
#include <area51/main.h>
#include <area51/math.h>
#include <area51/rabbitmq.h>
#include <rabbit-logger.h>
#include <fcntl.h>
#include <unistd.h>
#include <area51/charbuffer.h>
#include <area51/curl.h>

void logMessage(RabbitConsumer *con, amqp_envelope_t *msg) {
    FileLogger *f = rabbitmq_getUserData(con);

    filelogger_log_r(f, (char *) msg->message.body.bytes, 0, msg->message.body.len);
}

static void addTasks(RabbitMQ *mq, RabbitConsumer *con, struct json_object *o, void *c) {

    struct json_object *obj;
    if (json_object_object_get_ex(o, "recording", &obj)) {
        FileLogger *f = filelogger_new_json(obj);
        if (f) {
            rabbitmq_setUserData(con, f, NULL);
            rabbitmq_setMessage(con, logMessage);
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
            mq = rabbitmq_new(config, addTasks, NULL, main);
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

