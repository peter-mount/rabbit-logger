#include <stdlib.h>
#include <stdbool.h>
#include <area51/list.h>
#include <area51/json.h>
#include <area51/log.h>
#include <string.h>
#include "rabbitmq-int.h"

static void *worker(void *arg) {
    RabbitConsumer *mq = arg;

    logconsole("%3d Connecting to %s:%d", mq->consumerId, mq->mq->hostname, mq->mq->port);
    mq->conn = amqp_new_connection();

    mq->socket = amqp_tcp_socket_new(mq->conn);
    if (!mq->socket) {
        logconsole("%3d failed to create TCP socket", mq->consumerId);
        exit(1);
    }

    int status = amqp_socket_open(mq->socket, mq->mq->hostname, mq->mq->port);
    if (status) {
        logconsole("%3d failed to open TCP socket", mq->consumerId);
        exit(1);
    }

    logconsole("%3d Logging in", mq->consumerId);
    rabbitmq_die_on_amqp_error(amqp_login(mq->conn, mq->mq->vhost, 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, mq->mq->user, mq->mq->pass), "Logging in");

    logconsole("%3d open channel", mq->consumerId);
    amqp_channel_open(mq->conn, 1);
    rabbitmq_die_on_amqp_error(amqp_get_rpc_reply(mq->conn), "Opening channel");

    // Create queue
    amqp_bytes_t queuename;
    if (!mq->queuename) {
        amqp_queue_declare_ok_t *r = amqp_queue_declare(mq->conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
        rabbitmq_die_on_amqp_error(amqp_get_rpc_reply(mq->conn), "Declaring queue");
        queuename = amqp_bytes_malloc_dup(r->queue);
    } else
        queuename = amqp_cstring_bytes(mq->queuename);

    if (queuename.bytes == NULL) {
        fprintf(stderr, "Out of memory while copying queue name");
        exit(1);
    }

    amqp_queue_bind(mq->conn, 1, queuename, amqp_cstring_bytes(mq->exchange), amqp_cstring_bytes(mq->bindingkey), amqp_empty_table);
    rabbitmq_die_on_amqp_error(amqp_get_rpc_reply(mq->conn), "Binding queue");

    amqp_basic_consume(mq->conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    rabbitmq_die_on_amqp_error(amqp_get_rpc_reply(mq->conn), "Consuming");

    logconsole("%3d Consumer %lx", mq->consumerId, mq->consumer);

    if (mq->consumer)
        mq->consumer(mq);

    logconsole("%3d Completed", mq->consumerId);

    pthread_exit(NULL);
    return NULL;
}

void rabbitmq_connectConsumer(RabbitConsumer *mq) {
    pthread_create(&mq->thread, NULL, worker, mq);
}

void rabbitmq_connect(RabbitMQ *mq) {
    Node *n = list_getHead(&mq->consumers);
    while (list_isNode(n)) {
        RabbitConsumer *con = (RabbitConsumer *) n;
        n = list_getNext(n);
        rabbitmq_connectConsumer(con);
    }
}
