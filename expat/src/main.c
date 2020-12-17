#include <expat.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
  int phones;
  int humans;
} context_t;
  
void start_element(void* data, const XML_Char* name, const XML_Char** attrs) {
  context_t* ctx = data;
  if (!strcmp(name, "human")) {
    ctx->humans++;
  }
  if (!strcmp(name, "phone")) {
    ctx->phones++;
  }
}

int main(int argc, char** argv) {
  assert(argc > 1);
  int ret = 0;
  FILE* fd = fopen(argv[1], "r");

  context_t ctx;
  memset(&ctx, 0, sizeof(ctx));

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(parser, start_element, NULL);
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

  printf("Humans: %d\nPhones; %d\n", ctx.humans, ctx.phones);

out:
  XML_ParserFree(parser);
  free(buff);
  return ret;
}


