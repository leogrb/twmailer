#ifndef IP_H__
#define IP_H__

#include <time.h>

#define MIN_30 1800 // value in sec

typedef struct ip
{
    char *ip_address;
    time_t saved_time;
} ip_t;

#endif