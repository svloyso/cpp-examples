#ifndef CLIST_H_INCLUDED
#define CLIST_H_INCLUDED

#include <stddef.h>

struct intrusive_node {
  struct intrusive_node *next;
  struct intrusive_node *prev;
};

struct intrusive_list {
  struct intrusive_node head;
};

typedef struct intrusive_list intrusive_list_t;
typedef struct intrusive_node intrusive_node_t;
typedef void (*node_op_t)(intrusive_node_t *node, void *data);

#define container_of(ptr, type, mem) (type*)((char*)(ptr) - offsetof(type, mem))

void init_list(intrusive_list_t *il);
void add_node(intrusive_list_t *il, intrusive_node_t *in);
void remove_node(intrusive_list_t *il, intrusive_node_t *in);

int get_length(intrusive_list_t *il);
void apply(intrusive_list_t *il, node_op_t op, void *arg);

int list_empty(intrusive_list_t *il);

#endif
