#include <expat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "names.h"
#include "clist.h"

typedef struct {
  char phone[32];
  intrusive_node_t node;
  intrusive_node_t all_nodes;
} phone_t;

typedef struct {
  char f_name[32];
  char m_name[32];
  char l_name[32];
  intrusive_list_t phones;
  intrusive_node_t node;
} human_t;

phone_t* get_phone(intrusive_node_t* node) {
  return container_of(node, phone_t, node);
}

phone_t* get_phone_a(intrusive_node_t* node) {
  return container_of(node, phone_t, all_nodes);
}

human_t* get_human(intrusive_node_t* node) {
  return container_of(node, human_t, node);
}

typedef struct {
  intrusive_list_t humans;
  intrusive_list_t phones;

} phonebook_t;

void init_book(phonebook_t* book) {
  init_list(&book->humans);
  init_list(&book->phones);
}

void free_book(phonebook_t* book) {
  while (!list_empty(&book->humans)) {
    human_t* h = get_human(book->humans.head.next);
    remove_node(&book->humans, &h->node);
    free(h);
  }
  while (!list_empty(&book->phones)) {
    phone_t* p = get_phone_a(book->phones.head.next);
    remove_node(&book->phones, &p->all_nodes);
    free(p);
  }
}

typedef struct {
  phonebook_t* book;
  human_t* cur_human;
  phone_t* cur_phone;

  int inside_phone;
  int phone_len;
} context_t;

void parse_name(human_t* human, const XML_Char* name) {
  char buf[100];
  strncpy(buf, name, 99);

  char* tok = strtok(buf, " ");
  strcpy(human->f_name, tok);

  tok = strtok(NULL, " ");
  strcpy(human->m_name, tok);

  tok = strtok(NULL, " ");
  strcpy(human->l_name, tok);
}

void start_element(void *user_data,
                   const XML_Char *name, 
                   const XML_Char **atts) {
  context_t* ctx = user_data;
  if (!strcmp("human", name)) {
    ctx->cur_human = malloc(sizeof(human_t));
    init_list(&ctx->cur_human->phones);
    add_node(&ctx->book->humans, &ctx->cur_human->node);
    parse_name(ctx->cur_human, atts[1]);
  }
  if (!strcmp("phone", name)) {
    ctx->cur_phone = malloc(sizeof(phone_t));
    add_node(&ctx->cur_human->phones, &ctx->cur_phone->node);
    add_node(&ctx->book->phones, &ctx->cur_phone->all_nodes);
    ctx->inside_phone = 1;
  }
}

void end_element(void *user_data, const XML_Char *name) {
  context_t* ctx = user_data;
  if (!strcmp("phone", name)) {
    ctx->phone_len = 0;
    ctx->inside_phone = 0;
  }
}

void data_handler(void* user_data, const XML_Char* data, int len) {
  context_t* ctx = user_data;
  
  if (!ctx->inside_phone)
    return;

  memcpy(ctx->cur_phone->phone + ctx->phone_len, data, len);
  ctx->phone_len += len;
  ctx->cur_phone->phone[ctx->phone_len] = '\0';
}


void print_phone(phone_t* phone, void* data) {
  char* pref = data;

  printf("%s%s\n", pref, phone->phone);
}

void print_phone_n(intrusive_node_t* node, void* data) {
  print_phone(get_phone(node), data);
}

void print_phone_a(intrusive_node_t* node, void* data) {
  print_phone(get_phone_a(node), data);
}

void print_human(intrusive_node_t* node, void* data) {
  char* pref = data;
  human_t* human = get_human(node);

  printf("%sfirst name: %s\n%smid name: %s\n%slast name: %s\n", 
         pref, human->f_name, pref, human->m_name, pref, human->l_name);
  printf("%sPhones:\n", pref);

  char* tab = malloc(strlen(pref) + 2);
  strcpy(tab, pref);
  strcat(tab, "\t");
  apply(&human->phones, print_phone_n, tab);
  free(tab);
}


int gen_phonebook_xml(const char *filename, size_t size) {
  FILE* fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open %s", filename);
    return 1;
  }

  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fp);
  fputs("<phonebook>\n", fp);

  for (int i = 0; i < size; ++i) {
    fprintf(fp, "<human name=\"%s %s %s\">\n",
            kNames[rand() % NAMES_CNT],
            kMiddleNames[rand() % MIDDLE_NAMES_CNT],
            kFamilyNames[rand() % FAMILY_NAME_CNT]);

    int phones_cnt = 1 + rand() % 10;
    while(phones_cnt--) {
      int phone_len = 7 + rand() % 8;
      char phone[32];
      for (int j = 0; j < phone_len; ++j) {
        phone[j] = rand() % 10 + '0';
      }
      phone[phone_len] = '\0';
      fprintf(fp, "<phone>%s</phone>\n", phone);
    }
    fputs("</human>\n", fp);
  }

  fputs("</phonebook>\n", fp);
  fclose(fp);

  return 0;
}

int load_xml(phonebook_t* book, char* filename) {
  int ret = 0;
  XML_Parser parser = XML_ParserCreate(NULL);

  context_t ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.book = book;

  XML_SetElementHandler(parser, start_element, end_element);
  XML_SetCharacterDataHandler(parser, data_handler);
  XML_SetUserData(parser, &ctx);

  FILE* fd = fopen(filename, "r");
  const int buff_size = 1024 * 10;
  char* buff = malloc(buff_size);

  size_t len = 0;
  int done = 0;
  while (!done) {
    len = fread(buff, 1, buff_size, fd);
    done = len < buff_size;
    
    if(XML_Parse(parser, buff, len, done) == XML_STATUS_ERROR) {
      printf("Error!");
      ret = 1;
      goto out;
    }
  }
out:
  XML_ParserFree(parser);
  free(buff);
  fclose(fd);
  return ret;
}

void print_book(phonebook_t* book) {
  printf("=================\nHumans:\n"); 
  apply(&book->humans, print_human, "\t");

  printf("=================\nAll phones:\n");
  apply(&book->phones, print_phone_a, "\t");
}

int main(int argc, char* argv[]) {
  assert(argc > 1);

  if (!strcmp(argv[1], "gen")) {
    gen_phonebook_xml(argv[2], 10);
    return 0;
  }

  if (!strcmp(argv[1], "print")) {
    phonebook_t book;
    init_book(&book);

    load_xml(&book, argv[2]);
    print_book(&book);

    free_book(&book);
    return 0;
  }

  printf("Wrong arguments!");
  return 1;
}

