
#ifndef RABBITMQ_INT_H
#define RABBITMQ_INT_H

#include <pthread.h>
#include <area51/list.h>
#include <area51/math.h>
#include <area51/memory.h>
#include <area51/rabbitmq.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct RabbitMQ {
        const char *hostname;
        int port;
        const char *user;
        const char *pass;
        const char *vhost;
        int consumerId;
        List consumers;
    };

    struct RabbitConsumer {
        Node node;
        int consumerId;
        RabbitMQ *mq;
        const char *exchange;
        const char *bindingkey;
        char *queuename;
        int status;
        struct Statistic stats;
        bool reconnect;
        // Socket connection
        amqp_socket_t *socket;
        amqp_connection_state_t conn;
        // Thread for the consumer
        pthread_t thread;
        // The consumer
        void (*consumer)(RabbitConsumer *);
        // Acknowledge
        void (*ack)(amqp_envelope_t *);
        // Return
        void (*ret)(amqp_message_t *);
        // channel close
        void (*chanClose)(RabbitConsumer *);
        // connection close
        void (*conClose)(RabbitConsumer *);
        // Userdata
        Freeable *userdata;
    };

#ifdef __cplusplus
}
#endif

#endif /* RABBITMQ_INT_H */

