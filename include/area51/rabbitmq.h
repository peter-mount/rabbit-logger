#ifndef RABBITMQ_H
#define RABBITMQ_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <area51/json.h>
#include <area51/main.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct RabbitMQ RabbitMQ;
    typedef struct RabbitConsumer RabbitConsumer;

    /**
     * Connect to rabbitmq
     * @param RabbitMQ
     * @return 
     */
    extern void rabbitmq_connect(RabbitMQ *);

    /**
     * Configure new RabbitMQ from JSON config
     * @param Json
     * @param optional callback to configure a consumer
     * @param userdata for callback
     * @return 
     */
    extern RabbitMQ *rabbitmq_new(
            struct json_object *,
            void (*)(RabbitMQ *, RabbitConsumer*, struct json_object *, void *),
            void *,
            MainTasks *);

    /**
     * Add a new consumer
     * @param RabbitMQ
     * @param json object for this consumer
     * @param optional callback to configure a consumer
     * @param userdata for callback
     * @return 
     */
    extern RabbitConsumer *rabbitmq_newConsumer(
            RabbitMQ *,
            struct json_object *,
            void (*)(RabbitMQ *, RabbitConsumer*, struct json_object *, void *),
            void *,
            MainTasks *);

    extern void rabbitmq_die_on_amqp_error(amqp_rpc_reply_t, char const *);

    /**
     * Copy statistics and reset. This statistic contains the message rate only.
     * 
     * @param Consumer
     * @param Statistic to update
     */
    extern void rabbitmq_getStatistic(RabbitConsumer *, struct Statistic *);

    extern void *rabbitmq_getUserData(RabbitConsumer *);
    extern void rabbitmq_setUserData(RabbitConsumer *, void *, void (*)(void *));

    extern void rabbitmq_setAck(RabbitConsumer *, void (*)(RabbitConsumer *, amqp_envelope_t *));

    extern void rabbitmq_setRet(RabbitConsumer *, void (*)(RabbitConsumer *, amqp_message_t *));

    extern void rabbitmq_setMessage(RabbitConsumer *, void (*)(RabbitConsumer *, amqp_envelope_t *));

    extern void rabbitmq_setChanClose(RabbitConsumer *, void (*)(RabbitConsumer *));

    extern void rabbitmq_setConClose(RabbitConsumer *, void (*)(RabbitConsumer *));

#ifdef __cplusplus
}
#endif

#endif /* RABBITMQ_H */

