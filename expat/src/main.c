#include <expat.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "clist.h"

typedef intrusive_list_t ilist_t;
typedef intrusive_node_t inode_t;

typedef struct {
  char name[64];
  int len;
  intrusive_list_t phones;
  intrusive_node_t node;
} human_t;

typedef struct {
  char num[32];
  inode_t node;
} phone_t;

typedef struct {
  int len;
  ilist_t humans;
} pbook_t;

void init_human(human_t* h) {
  init_list(&h->phones);
  h->len = 0;
}

void init_book(pbook_t* b) {
  init_list(&b->humans);
  b->len = 0;
}

typedef struct {
  pbook_t* book;
  human_t* cur_human;
  phone_t* cur_phone;
  int in_phone;
  int phone_len;
} context_t;
  
void start_element(void* data, const XML_Char* name, const XML_Char** attrs) {
  context_t* ctx = data;
  if (!strcmp(name, "human")) {
    ctx->cur_human = malloc(sizeof(human_t));
    init_human(ctx->cur_human);
    strncpy(ctx->cur_human->name, attrs[1], 63);
  }
  if (!strcmp(name, "phone")) {
    ctx->cur_phone = malloc(sizeof(phone_t));
    ctx->in_phone = 1;
  }
}

void end_element(void* data, const XML_Char* name) {
  context_t* ctx = data;
  if (!strcmp(name, "human")) {
    add_node(&ctx->book->humans, &ctx->cur_human->node);
    ctx->cur_human = NULL;
    ctx->book->len += 1;
  }
  if (!strcmp(name, "phone")) {
    ctx->in_phone = 0;
    add_node(&ctx->cur_human->phones, &ctx->cur_phone->node);
    ctx->cur_phone = NULL;
    ctx->cur_human->len += 1;
    ctx->phone_len = 0;
  }
}

void data_handler(void* data, const XML_Char* buf, int len) {
  context_t* ctx = data;
  if (!ctx->in_phone)
    return;

  memcpy(ctx->cur_phone->num + ctx->phone_len, buf, len);
  ctx->phone_len += len;
  ctx->cur_phone->num[ctx->phone_len] = '\0';
}

void print_phone(inode_t* n, void* data) {
  phone_t* ph = container_of(n, phone_t, node);
  printf("%s\n", ph->num);
}

void print_human(inode_t* n, void* data) {
  human_t* h = container_of(n, human_t, node);
  printf("Human: %s\nPhones:", h->name);
  apply(&h->phones, print_phone, NULL);
  printf("==============================\n");
}

void print_book(pbook_t* book) {
  apply(&book->humans, print_human, NULL);
}

int main(int argc, char** argv) {
  assert(argc > 1);
  int ret = 0;
  FILE* fd = fopen(argv[1], "r");

  pbook_t book;
  init_book(&book);
  context_t ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.book = &book;

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(parser, start_element, end_element);
  XML_SetCharacterDataHandler(parser, data_handler);
  XML_SetUserData(parser, &ctx);

  size_t len = 0;
  int done = 0;
  const size_t buff_size = 1024;
  char* buff = malloc(buff_size);

  while(!done) {
    len = fread(buff, 1, buff_size, fd);
    done = len < buff_size;

    if (XML_Parse(parser, buff, len, done) == XML_STATUS_ERROR) {
      printf("Error!!!");
      ret = 1;
      goto out;
    }
  }

  print_book(&book);
out:
  XML_ParserFree(parser);
  free(buff);
  return ret;
}


