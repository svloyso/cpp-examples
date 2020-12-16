#include <assert.h>
#include "clist.h"

void init_list(intrusive_list_t *il) {
  il->head.next = &il->head;
  il->head.prev = &il->head;
}

void add_node(intrusive_list_t *il, intrusive_node_t *in) {
  intrusive_node_t *last = il->head.prev;

  in->next = &il->head;
  in->prev = last;

  last->next = in;
  il->head.prev = in;
}

void remove_node(intrusive_list_t *il, intrusive_node_t *in) {
  assert(&il->head != in);

  in->prev->next = in->next;
  in->next->prev = in->prev;
}

int get_length(intrusive_list_t *il) {
  intrusive_node_t *node = il->head.next;
  int len = 0;
  while (node != &il->head) {
    node = node->next;
    len++;
  }
  return len;
}

void apply(intrusive_list_t *il, node_op_t op, void *arg) {
  intrusive_node_t *node = il->head.next;
  while (node != &il->head) {
    op(node, arg);
    node = node->next;
  }
  
}

int list_empty(intrusive_list_t *il) {
  return il->head.next == &il->head && il->head.prev == &il->head;
}
