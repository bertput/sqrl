/*
 * uriparser code copied from uriparser project,
 * but modified for this project.
 *
 * The original copyright notice is preserved below:
 *
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2013, Radu Hociung <radu.uriparser@ohmi.org>
 * Copyright (C) 2013, Sebastian Pipping <sebastian@pipping.org>
 * All rights reserved.
 *
 * Redistribution  and use in source and binary forms, with or without
 * modification,  are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions   of  source  code  must  retain  the   above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer.
 *
 *     * Redistributions  in  binary  form must  reproduce  the  above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer   in  the  documentation  and/or  other  materials
 *       provided with the distribution.
 *
 *     * Neither  the name of the <ORGANIZATION> nor the names of  its
 *       contributors  may  be  used to endorse  or  promote  products
 *       derived  from  this software without specific  prior  written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT  NOT
 * LIMITED  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS
 * FOR  A  PARTICULAR  PURPOSE ARE DISCLAIMED. IN NO EVENT  SHALL  THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL,    SPECIAL,   EXEMPLARY,   OR   CONSEQUENTIAL   DAMAGES
 * (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES;  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT  LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE  OR  OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <uriparser/Uri.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <uriparser/Uri.h>

#include "uriparse.h"


#define RANGE(x)  (int)((x).afterLast-(x).first), ((x).first)
#define LENGTH(x)  (int)((x).afterLast-(x).first)


uri *uriparse_parse_uri(const char *in_str)
{
  uri *retval = malloc(sizeof(uri));
  retval->host = NULL;
  retval->path = NULL;
  retval->query = NULL;
  retval->scheme = NULL;

  UriParserStateA state;
  UriUriA *uri_a = malloc(sizeof(UriUriA));
  char ipstr[INET6_ADDRSTRLEN];

  state.uri = uri_a;
  printf ("uri:          %s\n", in_str);
  if (uriParseUriA (&state, in_str) != URI_SUCCESS)
  {
    /* Failure */
    printf ("Failure:      %s @ '%.18s' (#%lu)\n",
            (state.errorCode == URI_ERROR_SYNTAX)
            ? "syntax"
            : (state.errorCode == URI_ERROR_MALLOC)
            ? "not enough memory"
            : "liburiparser bug (please report)",
            state.errorPos, state.errorPos - in_str);

  }
  else
  {
    if (uri_a->scheme.first)
    {
      printf ("scheme:       %.*s\n", RANGE (uri_a->scheme));
      retval->scheme = malloc(LENGTH(uri_a->scheme) + 1);
      memset(retval->scheme, 0, LENGTH(uri_a->scheme) + 1);
      strncpy(retval->scheme, uri_a->scheme.first, LENGTH(uri_a->scheme));
    }

    if (uri_a->userInfo.first)
    {
      printf ("userInfo:     %.*s\n", RANGE (uri_a->userInfo));
    }

    if (uri_a->hostText.first)
    {
      printf ("hostText:     %.*s\n", RANGE (uri_a->hostText));
      retval->host = malloc(LENGTH(uri_a->hostText) + 1);
      memset(retval->host, 0, LENGTH(uri_a->hostText) + 1);
      strncpy(retval->host, uri_a->hostText.first, LENGTH(uri_a->hostText));
    }

    if (uri_a->hostData.ip4)
    {
      inet_ntop (AF_INET, uri_a->hostData.ip4->data, ipstr, sizeof ipstr);
      printf ("hostData.ip4: %s\n", ipstr);
    }

    if (uri_a->hostData.ip6)
    {
      inet_ntop (AF_INET6, uri_a->hostData.ip6->data, ipstr, sizeof ipstr);
      printf ("hostData.ip6: %s\n", ipstr);
    }

    if (uri_a->portText.first)
    {
      printf ("portText:     %.*s\n", RANGE (uri_a->portText));
    }

    if (uri_a->pathHead)
    {
      const UriPathSegmentA *p = uri_a->pathHead;
      retval->path = malloc(LENGTH(p->text) + 1);
      memset(retval->path, 0, LENGTH(p->text) + 1);
      strncpy(retval->path, p->text.first, LENGTH(p->text));
      for (; p; p = p->next)
      {
        printf (" .. pathSeg:  %.*s\n", RANGE (p->text));
      }
    }

    if (uri_a->query.first)
    {
      printf ("query:        %.*s\n", RANGE (uri_a->query));
      retval->query = malloc(LENGTH(uri_a->query) + 1);
      memset(retval->query, 0, LENGTH(uri_a->query) + 1);
      strncpy(retval->query, uri_a->query.first, LENGTH(uri_a->query));
    }

    if (uri_a->fragment.first)
    {
      printf ("fragment:     %.*s\n", RANGE (uri_a->fragment));
    }

    {
      const char *const absolutePathLabel = "absolutePath: ";
      printf ("%s%s\n", absolutePathLabel,
              (uri_a->absolutePath == URI_TRUE) ? "true" : "false");
      if (uri_a->hostText.first != NULL)
      {
        printf ("%*s%s\n", (int) strlen (absolutePathLabel), "",
                "(always false for URIs with host)");
      }
    }
  }

  uriFreeUriMembersA (uri_a);

  return retval;
}

void uriparse_free_uri(uri *in_uri)
{
  if (in_uri)
  {
    if (in_uri->scheme)
    {
      free(in_uri->scheme);
    }
    if (in_uri->path)
    {
      free(in_uri->path);
    }
    if (in_uri->host)
    {
      free(in_uri->host);
    }
    if (in_uri->query)
    {
      free(in_uri->query);
    }

    free(in_uri);
  }
}

