#include <stdlib.h>
#include <stdbool.h>
#include <area51/list.h>
#include <area51/json.h>
#include <string.h>
#include "rabbitmq-int.h"

static char *get(struct json_object *o, char *k) {
    char *s = json_getString(o, k);
    return s ? strdup(s) : NULL;
}

RabbitMQ *rabbitmq_new(
        struct json_object *cfg,
        void (*config)(RabbitMQ *, RabbitConsumer*, struct json_object *, void *),
        void *c,
        MainTasks *main) {
    struct json_object *obj;

    RabbitMQ *mq = malloc(sizeof (struct RabbitMQ));
    if (!mq)
        return NULL;

    memset(mq, 0, sizeof (struct RabbitMQ));

    mq->hostname = get(cfg, "host");
    mq->port = json_getInt(cfg, "port");
    mq->user = get(cfg, "user");
    mq->pass = get(cfg, "pass");
    mq->vhost = get(cfg, "vhost");

    if (!mq->vhost)
        mq->vhost = "/";

    list_init(&mq->consumers);

    // Single queue
    if (json_object_object_get_ex(cfg, "queue", &obj)) {
        rabbitmq_newConsumer(mq, obj, config, c, main);
    }

    // Array of queues
    if (json_object_object_get_ex(cfg, "queues", &obj)) {
        struct array_list *list = json_object_get_array(obj);
        int len = array_list_length(list);
        for (int i = 0; i < len; i++) {
            obj = (json_object *) array_list_get_idx(list, i);
            rabbitmq_newConsumer(mq, obj, config, c, main);
        }
    }

    return mq;
}