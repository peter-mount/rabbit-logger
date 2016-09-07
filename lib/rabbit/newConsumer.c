
#include <stdlib.h>
#include <stdbool.h>
#include <area51/json.h>
#include <area51/list.h>
#include <area51/log.h>
#include <area51/main.h>
#include <area51/math.h>
#include <area51/memory.h>
#include <string.h>
#include "rabbitmq-int.h"

static char *get(struct json_object *o, char *k) {
    char *s = json_getString(o, k);
    return s ? strdup(s) : NULL;
}

static void consumer(RabbitConsumer *con) {
    while (true) {
        amqp_frame_t frame;
        amqp_envelope_t envelope;

        amqp_maybe_release_buffers(con->conn);
        amqp_rpc_reply_t ret = amqp_consume_message(con->conn, &envelope, NULL, 0);

        if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
            if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type && AMQP_STATUS_UNEXPECTED_STATE == ret.library_error) {
                if (AMQP_STATUS_OK != amqp_simple_wait_frame(con->conn, &frame)) {
                    return;
                }

                if (AMQP_FRAME_METHOD == frame.frame_type) {
                    switch (frame.payload.method.id) {
                            /* if we've turned publisher confirms on, and we've published a message
                             * here is a message being confirmed
                             */
                        case AMQP_BASIC_ACK_METHOD:
                            if (con->ack)
                                con->ack(&envelope);
                            break;

                            /* if a published message couldn't be routed and the mandatory flag was set
                             * this is what would be returned. The message then needs to be read.
                             */
                        case AMQP_BASIC_RETURN_METHOD:
                        {
                            amqp_message_t message;
                            ret = amqp_read_message(con->conn, frame.channel, &message, 0);
                            if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                                return;
                            }

                            if (con->ret)
                                con->ret(&message);

                            amqp_destroy_message(&message);
                        }
                            break;

                            /* a channel.close method happens when a channel exception occurs, this
                             * can happen by publishing to an exchange that doesn't exist for example
                             *
                             * In this case you would need to open another channel redeclare any queues
                             * that were declared auto-delete, and restart any consumers that were attached
                             * to the previous channel
                             */
                        case AMQP_CHANNEL_CLOSE_METHOD:
                            if (con->chanClose)
                                con->chanClose(con);
                            return;

                            /* a connection.close method happens when a connection exception occurs,
                             * this can happen by trying to use a channel that isn't open for example.
                             *
                             * In this case the whole connection must be restarted.
                             */
                        case AMQP_CONNECTION_CLOSE_METHOD:
                            if (con->conClose)
                                con->conClose(con);
                            return;

                        default:
                            logconsole("An unexpected method was received %u\n", frame.payload.method.id);
                            return;
                    }
                }
            }

        } else {
            amqp_destroy_envelope(&envelope);
        }

        statistic_increment(&con->stats);
    }
}

void rabbitmq_getStatistic(RabbitConsumer *c, struct Statistic *s) {
    if (c && s)
        statistic_copy(s, &c->stats);
}

RabbitConsumer *rabbitmq_newConsumer(
        RabbitMQ *mq,
        struct json_object *cfg,
        void (*config)(RabbitMQ *, RabbitConsumer*, struct json_object *, void *),
        void *c,
        MainTasks *main) {
    RabbitConsumer *con = malloc(sizeof (struct RabbitConsumer));
    if (!con)
        return NULL;

    memset(con, 0, sizeof (struct RabbitConsumer));

    con->mq = mq;
    con->consumer = consumer;
    con->consumerId = ++mq->consumerId;

    statistic_init(&con->stats);

    con->exchange = get(cfg, "exchange");
    if (!con->exchange)
        con->exchange = "amq.topic";

    con->bindingkey = get(cfg, "key");

    // Optional
    con->queuename = get(cfg, "queueName");

    // Optional
    con->node.name = get(cfg, "name");

    // Logging
    json_object *logging;
    if (main && json_object_object_get_ex(cfg, "logging", &logging))
        statistic_recorder(main, &con->stats, logging);

    logconsole("Consumer %d exchange=\"%s\" bindingKey=\"%s\"", con->consumerId, con->exchange, con->bindingkey);

    if (config)
        config(mq, con, cfg, c);

    list_addTail(&mq->consumers, &con->node);

    return con;
}

void *rabbitmq_getUserData(RabbitConsumer *c) {
    return c ? freeable_get(c->userdata) : NULL;
}

void rabbitmq_setUserData(RabbitConsumer *c, void *v, void (*f)(void *)) {
    if (c)
        c->userdata = freeable_set(c->userdata, v, f);
}
