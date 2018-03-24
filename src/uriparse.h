#ifndef URIPARSE_H
#define URIPARSE_H

typedef struct uri_struct
{
  char *scheme;
  char *path;
  char *host;
  char *query;
} uri;

uri *uriparse_parse_uri(const char *in_str);


void uriparse_free_uri(uri *in_uri);



#endif // URIPARSE_H
