#ifndef URIPARSE_H
#define URIPARSE_H

#include <uriparser/Uri.h>


UriUriA *uriparse_parse_uri(char *in_str);

char *
uriparse_get_scheme(UriUriA *uri);

char *
uriparse_get_path(UriUriA *uri);

char *
uriparse_get_host(UriUriA *uri);

char *
uriparse_get_query(UriUriA *uri);

void uriparse_free_uri(UriUriA *uri);



#endif // URIPARSE_H
