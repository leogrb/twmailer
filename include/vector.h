#ifndef VECTOR_H__
#define VECTOR_H__

#include "vector.h"
#include "ip.h"

typedef struct vector_
{
  void **data;
  int size;
  int count;
} vector;

void vector_init(vector *);
int vector_count(vector *);
void vector_add(vector *, void *);
void vector_set(vector *, int, void *);
void *vector_get(vector *, int);
void vector_delete(vector *, int);
void vector_free(vector *);
int vector_get_index(vector *, char *);

#endif