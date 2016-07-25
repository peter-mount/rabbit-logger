#include <area51/list.h>
#include <rabbit-logger.h>

struct List loggers;

void initLoggers() {
    list_init(&loggers);
}
