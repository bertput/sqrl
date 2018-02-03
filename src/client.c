#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <resolv.h>
#include <arpa/inet.h>
#include <string.h>

#define SSL_ENABLED

#ifdef SSL_ENABLED
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#endif // SSL_ENABLED

#include <glib.h>

#include "client.h"
#include "log.h"
#include "settings.h"

#include "sqrl_expert.h"


Sqrl_Server *server = NULL;

Sqrl_User t1_user = NULL;

char load_uid[SQRL_UNIQUE_ID_LENGTH + 1];
Sqrl_User load_user = NULL;

bool client_onAuthenticationRequired(
    Sqrl_Transaction transaction,
    Sqrl_Credential_Type credentialType );

int client_onProgress( Sqrl_Transaction transaction, int p );

void client_onTransactionComplete( Sqrl_Transaction transaction );

void client_onSaveSuggested( Sqrl_User user );

Sqrl_User client_onSelectUser (Sqrl_Transaction transaction);

char statusText[4][10] = {
    "SUCCESS",
    "FAILED",
    "CANCELLED",
    "WORKING"
};

/**
A structure to hold information about a parsed URI
*/
typedef struct URI_type {
	/** The domain + extension */
	char *host;
	/** Internal use */
	char *prefix;
	/** the https url */
	char *url;
	/** Internal use */
	Sqrl_Scheme scheme;
  /** path */
  char *path;
  /** query */
  char *query;
	/** Server Friendly Name */
	char *sfn;
} URI;

//URI*	client_uri_create_copy( URI *original );
URI*	client_uri_parse(const char *);
URI*	client_uri_free(URI *);



void client_onSend(
    Sqrl_Transaction transaction,
    const char *url, size_t url_len,
    const char *payload, size_t payload_len );



void error(const char *msg) { perror(msg); exit(0); }


int client_authenticate(char *in_url)
{
  log_info("Authenticating against website: %s\n", in_url);

  settings_new();

  char *sqrl_id_filename = settings_get_sqrl_id_filename();

  log_info("Using identity file at %s with strlen %d\n", sqrl_id_filename, strlen(sqrl_id_filename));

  Sqrl_Client_Callbacks cbs;
  memset( &cbs, 0, sizeof( Sqrl_Client_Callbacks ));
//  cbs.onAsk
  cbs.onAuthenticationRequired = client_onAuthenticationRequired;
  cbs.onProgress = client_onProgress;
  cbs.onSaveSuggested = client_onSaveSuggested;
//  cbs.onSelectAlternateIdentity
  cbs.onSelectUser = client_onSelectUser;
  cbs.onSend = client_onSend;
  cbs.onTransactionComplete = client_onTransactionComplete;




  sqrl_client_set_callbacks( &cbs );

  sqrl_init();

  Sqrl_Transaction_Status sqrlTransactionStatus =
    sqrl_client_begin_transaction(SQRL_TRANSACTION_IDENTITY_LOAD, NULL, sqrl_id_filename, strlen(sqrl_id_filename));

  if (sqrlTransactionStatus == SQRL_TRANSACTION_STATUS_SUCCESS)
  {
    log_info("Successfully loaded identity from %s\n", sqrl_id_filename);
  }
  else
  {
    printf("*** ERROR: failed to load identity from %s\n", sqrl_id_filename);
    exit(-1);
  }

  sqrlTransactionStatus =
    sqrl_client_begin_transaction(SQRL_TRANSACTION_AUTH_IDENT, NULL, in_url, strlen(in_url));

  if (sqrlTransactionStatus == SQRL_TRANSACTION_STATUS_SUCCESS)
  {
    log_info("Successfully logged into %s\n", in_url);
  }
  else
  {
    printf("*** ERROR: failed to log into %s with status %d (%s)\n", in_url, sqrlTransactionStatus, statusText[sqrlTransactionStatus]);
    exit(-1);
  }


  return 0;
}


#define PC(a,b) printf( "%10s: %s\n", (a), (b))

bool client_onAuthenticationRequired(
    Sqrl_Transaction transaction,
    Sqrl_Credential_Type credentialType )
{
    char *cred = NULL;
    uint8_t len;

    char *sqrl_password = settings_get_sqrl_password();
    char *sqrl_rescue_code = settings_get_sqrl_rescue_code();

    log_info("in client_onAuthenticationRequired\n");

    switch( credentialType ) {
    case SQRL_CREDENTIAL_PASSWORD:
        PC( "AUTH_REQ", "Password" );
        cred = malloc( strlen( sqrl_password ) + 1 );
        strcpy( cred, sqrl_password );
        break;
    case SQRL_CREDENTIAL_HINT:
        PC( "AUTH_REQ", "Hint" );
        len = sqrl_user_get_hint_length( sqrl_transaction_user( transaction ));
        cred = malloc( len + 1 );
        strncpy( cred, sqrl_password, len );
        break;
    case SQRL_CREDENTIAL_RESCUE_CODE:
        PC( "AUTH_REQ", "Rescue Code" );
        cred = malloc( strlen( sqrl_rescue_code ) + 1 );
        strcpy( cred, sqrl_rescue_code );
        break;
#if 0
    case SQRL_CREDENTIAL_NEW_PASSWORD:
        PC( "AUTH_REQ", "New Password" );
        cred = malloc( strlen( new_password ) + 1 );
        strcpy( cred, new_password );
        break;
#endif // 0
    default:
  log_info("OUT client_onAuthenticationRequired with FALSE\n");
        return false;
    }
    sqrl_client_authenticate( transaction, credentialType, cred, strlen( cred ));
    if( cred ) {
        free( cred );
    }
  log_info("OUT client_onAuthenticationRequired with TRUE\n");
    return true;
}

char transactionType[14][10] = {
    "UNKNWN",
    "QUERY",
    "IDENT",
    "DISABLE",
    "ENABLE",
    "REMOVE",
    "SAVE",
    "RESCUE",
    "REKEY",
    "UNLOCK",
    "LOCK",
    "LOAD",
    "GENRATE",
    "CHNG_PSWD"
};
bool showingProgress = false;
int nextProgress = 0;
int client_onProgress( Sqrl_Transaction transaction, int p )
{
//  log_info("in client_onProgress\n");

    if( !showingProgress ) {
        // Transaction type
        showingProgress = true;
        nextProgress = 2;
        printf( "%10s: ", transactionType[sqrl_transaction_type(transaction)] );
    }
    const char sym[] = "|****";
    while( p >= nextProgress ) {
        if( nextProgress != 100 ) {
            printf( "%c", sym[nextProgress%5] );
        }
        nextProgress += 2;
    }
    if( p >= 100 ) {
        printf( "\n" );
        showingProgress = false;
    }
    fflush( stdout );
    return 1;

}

void client_onTransactionComplete( Sqrl_Transaction transaction )
{
  log_info("in client_onTransactionComplete\n");

    Sqrl_Transaction_Type type = sqrl_transaction_type(transaction);
    Sqrl_Transaction_Status status = sqrl_transaction_status(transaction);
    Sqrl_User user = sqrl_transaction_user(transaction);
    PC( transactionType[type], statusText[status] );
    if( status == SQRL_TRANSACTION_STATUS_SUCCESS ) {
        switch( type ) {
        case SQRL_TRANSACTION_IDENTITY_LOAD:
            if( !t1_user ) {
                t1_user = sqrl_user_hold( user );
            } else if( !load_user ) {
                load_user = sqrl_user_hold( user );
                sqrl_user_unique_id( load_user, load_uid );
            } else {
                PC( "FAIL", "Loaded too many users!" );
                exit(1);
            }
            break;
#if 0
        case SQRL_TRANSACTION_IDENTITY_GENERATE:
            gen_user = sqrl_user_hold( user );
            char *rc = sqrl_user_get_rescue_code( transaction );
            if( rc ) {
                strcpy( gen_rescue_code, rc );
                PC( "GEN_RC", gen_rescue_code );
                sqrl_client_export_user( gen_user, NULL, SQRL_EXPORT_ALL, SQRL_ENCODING_BASE64 );
            } else {
                printf( "RC not retrieved\n" );
                exit(1);
            }
            break;
        case SQRL_TRANSACTION_IDENTITY_SAVE:
            if( !gen_data ) {
                size_t len = sqrl_transaction_string( transaction, NULL, 0 );
                if( len ) {
                    gen_data = malloc( ++len );
                    sqrl_transaction_string( transaction, gen_data, &len );
                    sqrl_user_unique_id( gen_user, gen_uid );
                    PC( "SAVE_UID", gen_uid );
                    PC( "SAVE_DATA", gen_data );
                }
            }
#endif // 0
        default:
            break;
        }
    }
}

void client_onSaveSuggested( Sqrl_User user )
{
  log_info("in client_onSaveSuggested\n");

    char buf[44];
    sqrl_user_unique_id( user, buf );
    if( strlen( buf ) == 0 ) {
        PC( "SAVE_SUG", "New Identity" );
    } else {
        PC( "SAVE_SUG", buf );
    }
}

Sqrl_User client_onSelectUser (Sqrl_Transaction transaction)
{
  log_info("in client_onSelectUser\n");
  return t1_user;
}

void client_onSend(
    Sqrl_Transaction transaction,
    const char *url, size_t url_len,
    const char *payload, size_t payload_len )
{
  struct hostent *server;
  struct sockaddr_in serv_addr;
  int sockfd, bytes, sent, received, total;
  uint16_t portno = 443;  // SSL
  char response[4096];

#ifdef SSL_ENABLED
  BIO              *certbio = NULL;
  BIO               *outbio = NULL;
  X509                *cert = NULL;
  X509_NAME       *certname = NULL;
  const SSL_METHOD *method;
  SSL_CTX *ctx;
  SSL *ssl;
#endif // SSL_ENABLED

  log_info( "Dest URL: %s\n", url );
  log_info( "Payload : %s\n", payload );

  URI *uri = client_uri_parse(url);

  log_info("    host: %s\n", uri->host);
  log_info("    path: %s\n", uri->path);
  log_info("   query: %s\n", uri->query);
  log_info("  prefix: %s\n", uri->prefix);
  log_info("  scheme: %s\n", uri->scheme);
  log_info("     sfn: %s\n", uri->sfn);
  log_info("     url: %s\n", uri->url);

#ifdef SSL_ENABLED
  /*
   * Initialize openssl
   */
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();

  /*
   * Create the Input/Output BIO's.
   */
  certbio = BIO_new(BIO_s_file());
  outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

  /*
   * initialize SSL library and register algorithms
   */
  if(SSL_library_init() < 0)
    BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");

  /*
   * Set SSLv2 client hello, also announce SSLv3 and TLSv1
   */
  method = SSLv23_client_method();

  /*
   * Try to create a new SSL context
   */
  if ( (ctx = SSL_CTX_new(method)) == NULL)
    BIO_printf(outbio, "Unable to create a new SSL context structure.\n");

  /*
   * Disabling SSLv2 will leave v3 and TSLv1 for negotiation
   */
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

  /*
   * Create new SSL connection state object
   */
  ssl = SSL_new(ctx);
#endif // SSL_ENABLED

  /* create the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");

  /* lookup the ip address */
#ifdef SSL_ENABLED
  server = gethostbyname(uri->host);
#else
  server = gethostbyname("httpbin.org");
#endif
  if (server == NULL) error("ERROR, no such host");

  /* fill in the structure */
  memset(&serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

  /* connect the socket */
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
  {
      error("ERROR connecting");
  }
  else
  {
    log_info("Successfully connected to %s\n", uri->host);
  }

#ifdef SSL_ENABLED
  /*
   * Attach the SSL session to the socket descriptor
   */
  int ssl_set_fs_rc = SSL_set_fd(ssl, sockfd);
  if (ssl_set_fs_rc == 1)
  {
    log_info("SSL_set_fd returned %d\n", ssl_set_fs_rc);
  }
  else
  {
    log_error("SSL_set_fd returned %d\n", ssl_set_fs_rc);
  }

  /*
   * Try to SSL-connect here, returns 1 for success
   */
  int ssl_connect_rc =  SSL_connect(ssl);
  if ( ssl_connect_rc == 1 )
    BIO_printf(outbio, "Successfully enabled SSL/TLS session to: %s.\n", uri->host);
  else
  {
    BIO_printf(outbio, "Error %d: Could not build a SSL session to: %s.\n", ssl_connect_rc, uri->host);
    error("ssl_connect");
  }
#endif // SSL_ENABLED

  GString *message_gstr = g_string_new("");
  g_string_append_printf(message_gstr, "POST /%s?%s HTTP/1.0\r\n", uri->path, uri->query);
  g_string_append_printf(message_gstr, "Host: %s\r\n", uri->host);
  g_string_append_printf(message_gstr, "User-Agent: SQRL/1\r\n");
  g_string_append_printf(message_gstr, "Content-type: application/x-www-form-urlencoded\r\n");
  g_string_append_printf(message_gstr, "Content-Length: %d\r\n", payload_len);
  g_string_append_printf(message_gstr, "\r\n");
  g_string_append_printf(message_gstr, "%s", payload);


  /* send the request */
  total = message_gstr->len;
  char *message = message_gstr->str;
  log_info("    sending message:\n%s\n", message);
  sent = 0;
  do {
#ifdef SSL_ENABLED
      bytes = SSL_write(ssl,message+sent,total-sent);
#else
      bytes = write(sockfd,message+sent,total-sent);
#endif
      if (bytes < 0)
          error("ERROR writing message to socket");
      if (bytes == 0)
      {
        break;
      }
      else
      {
        sent+=bytes;
        log_info("Sent %d bytes, %d of %d\n", bytes, sent, total);
      }
  } while (sent < total);

  log_info("    Receiving response (size unknown)\n");

  GString *response_gstr = g_string_new("");

  /* receive the response */
  memset(response,0,sizeof(response));
  received = 0;
  do {
#ifdef SSL_ENABLED
      received = SSL_read(ssl, response, 1000);
#else
      received = read(sockfd, response, 1);
#endif // SSL_ENABLED
      log_info("   recd %d bytes\n", received);
      if (received < 0)
      {
        error("ERROR reading response from socket");
      }
      else if (received > 0)  // we got some data.
      {
        // Rely on the zero terminator to find end of the data.
        //
        g_string_append(response_gstr, response);

        // Zero out the buffer for the next batch of data.
        memset(response,0,sizeof(response));
      }
  } while (received > 0); // keep looping as long as there's data to get.

  /* close the socket */
  close(sockfd);

  /* process response */
  log_info("Response:\n%s\n",response_gstr->str);

  sqrl_client_receive(transaction, response_gstr->str, response_gstr->len);

  log_info("OUT client_onSend\n");
}




static __inline__ int _is_scheme_char(int);

static __inline__ int
_is_scheme_char(int c)
{
	return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}


URI * client_uri_parse(const char *theUrl)
{
	URI *puri = NULL;
	const char *tmpstr;
	const char *curstr;
	int len;
	int i;
	int userpass_flag;
	int bracket_flag;
	char *host = NULL,
		 *port = NULL,
		 *path = NULL,
		 *query = NULL,
		 *fragment = NULL,
		 *username = NULL,
		 *password = NULL;

  log_info("in client_uri_parse()\n");

	UT_string *prefix = NULL;
	char *uri = malloc( strlen( theUrl ) + 1 );
	strcpy( uri, theUrl );
//	_fix_divider( uri );

	/* Allocate the parsed uri storage */
	puri = calloc(sizeof(Sqrl_Uri),1);
	if( puri == NULL ) goto ERROR;

	curstr = uri;

	/*
	 * <scheme>:<scheme-specific-part>
	 * <scheme> := [a-z\+\-\.]+
	 *             upper case = lower case for resiliency
	 */
	/* Read scheme */
	tmpstr = strchr(curstr, ':');
	if ( NULL == tmpstr ) goto ERROR;

	/* Get the scheme length */
	len = tmpstr - curstr;
	/* Check restrictions */
	for ( i = 0; i < len; i++ ) {
		if ( !_is_scheme_char(curstr[i]) ) goto ERROR;
	}
	/* Copy the scheme to the storage */
	char *sch = malloc( sizeof(char) * (len + 1 ));
	if ( NULL == sch ) goto ERROR;

	(void)strncpy(sch, curstr, len);
	sch[len] = '\0';
	sqrl_lcstr( sch );

  log_info("  sch is %s\n", sch);

	free( sch );

	/* Skip ':' */
	tmpstr++;
	curstr = tmpstr;

	/*
	 * //<user>:<password>@<host>:<port>/<uri-path>
	 * Any ":", "@" and "/" must be encoded.
	 */
	/* Eat "//" */
	for ( i = 0; i < 2; i++ ) {
		if ( '/' != *curstr ) goto ERROR;
		curstr++;
	}

  log_info("  Checking user and pwd\n");
	/* Check if the user (and password) are specified. */
	userpass_flag = 0;
	tmpstr = curstr;
	while ( '\0' != *tmpstr ) {
		if ( '@' == *tmpstr ) {
			/* Username and password are specified */
			userpass_flag = 1;
			break;
		} else if ( '/' == *tmpstr ) {
			/* End of <host>:<port> specification */
			userpass_flag = 0;
			break;
		}
		tmpstr++;
	}

  log_info("  read username\n");

	/* User and password specification */
	tmpstr = curstr;
	if ( userpass_flag ) {
		/* Read username */
		while ( '\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr ) {
			tmpstr++;
		}
		len = tmpstr - curstr;
		username = malloc(sizeof(char) * (len + 1));
		if ( NULL == username ) goto ERROR;
		(void)strncpy(username, curstr, len);
		username[len] = '\0';

  log_info("  read pwd\n");

	/* Proceed current pointer */
	curstr = tmpstr;
	if ( ':' == *curstr ) {
		/* Skip ':' */
		curstr++;
		/* Read password */
		tmpstr = curstr;
		while ( '\0' != *tmpstr && '@' != *tmpstr ) {
			tmpstr++;
		}
		len = tmpstr - curstr;
		password = malloc(sizeof(char) * (len + 1));
		if ( NULL == password ) goto ERROR;
		(void)strncpy(password, curstr, len);
		password[len] = '\0';
	curstr = tmpstr;
	}
	/* Skip '@' */
	if ( '@' != *curstr ) goto ERROR;
	curstr++;
	}

  log_info("  bracket\n");

	if ( '[' == *curstr ) {
		bracket_flag = 1;
	} else {
		bracket_flag = 0;
	}

  log_info("  host delimiter\n");

	/* Proceed on by delimiters with reading host */
	tmpstr = curstr;
	while ( '\0' != *tmpstr ) {
		if ( bracket_flag && ']' == *tmpstr ) {
			/* End of IPv6 address. */
			tmpstr++;
			break;
		} else if ( !bracket_flag && (':' == *tmpstr || '/' == *tmpstr) ) {
			/* Port number is specified. */
			break;
		}
		tmpstr++;
	}

  log_info("  host name\n");

	len = tmpstr - curstr;
	host = malloc(sizeof(char) * (len + 1));
	if ( NULL == host || len <= 0 ) goto ERROR;
	(void)strncpy(host, curstr, len);
	host[len] = '\0';
	curstr = tmpstr;

  log_info("  port number\n");

	/* Is port number specified? */
	if ( ':' == *curstr ) {
		curstr++;
		/* Read port number */
		tmpstr = curstr;
		while ( '\0' != *tmpstr && '/' != *tmpstr ) {
			tmpstr++;
		}
		len = tmpstr - curstr;
		port = malloc(sizeof(char) * (len + 1));
		if ( NULL == port ) goto ERROR;
		(void)strncpy(port, curstr, len);
		port[len] = '\0';
	curstr = tmpstr;
	}

	/* Skip '/' */
	if ( '/' != *curstr ) goto ERROR;
	curstr++;

  log_info("  path\n");

	/* Parse path */
	tmpstr = curstr;
	while ( '\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr ) {
		tmpstr++;
	}
	len = tmpstr - curstr;
	path = malloc(sizeof(char) * (len + 1));
	if ( NULL == path ) goto ERROR;

	(void)strncpy(path, curstr, len);
	path[len] = '\0';
	curstr = tmpstr;

  log_info("  query\n");

	/* Is query specified? */
	if ( '?' == *curstr ) {
		/* Skip '?' */
		curstr++;
		/* Read query */
		tmpstr = curstr;
		while ( '\0' != *tmpstr && '#' != *tmpstr ) {
			tmpstr++;
		}
		len = tmpstr - curstr;
		query = malloc(sizeof(char) * (len + 1));
		if ( NULL == query ) goto ERROR;

		(void)strncpy(query, curstr, len);
		query[len] = '\0';
	curstr = tmpstr;
	}

  log_info("  fragment\n");

	/* Is fragment specified? */
	if ( '#' == *curstr ) {
		/* Skip '#' */
		curstr++;
		/* Read fragment */
		tmpstr = curstr;
		while ( '\0' != *tmpstr ) {
			tmpstr++;
		}
		len = tmpstr - curstr;
		fragment = malloc(sizeof(char) * (len + 1));
		if ( NULL == fragment ) goto ERROR;

		(void)strncpy(fragment, curstr, len);
		fragment[len] = '\0';
	}

  log_info("Populate puri\n");

	size_t hl = strlen( host );
	long pl = 0;
	size_t ul = hl + 1;

  log_info("  host %s\n", host);
	if( pl ) ul += pl + 1;
	puri->host = (char*) calloc( ul, 1 );
	if( puri->host == NULL )
  {
    log_error( "*** ERROR puri->host\n");

    goto ERROR;
  }
	strcpy( puri->host, host );
//	utstring_bincpy( prefix, host, strlen( host ));

  log_info("  port %s\n", port);

	if( port ) {
		utstring_printf( prefix, ":%s", port );
	}

  log_info("  path %s\n", path);

	if( path ) {
    puri->path = (char*) malloc( strlen( path ));
    if( puri->path == NULL )
    {
      log_error( "*** ERROR puri->path\n");
      goto ERROR;
    }

		strcpy( puri->path, path);
	}

  log_info("  prefix %s\n", prefix);

  if (prefix)
  {
    puri->prefix = (char*) malloc( utstring_len( prefix ));
    if( puri->prefix == NULL )
    {
      log_error( "*** ERROR puri->prefix\n");
      goto ERROR;
    }
    strcpy( puri->prefix, utstring_body( prefix ));
  }

  log_info("  query %s\n", query);

  if (query)
  {
    puri->query = (char*) malloc( strlen(query));
    if( puri->query == NULL )
    {
      log_error( "*** ERROR puri->query\n");
      goto ERROR;
    }
    strcpy( puri->query, query );
  }
	goto END;

ERROR:
  log_error( " ERROR label\n");

	if( puri ) {
		client_uri_free( puri );
	}
	puri = NULL;

END:
  log_info( " END label\n");

	if( host ) free( host );
	if( port ) free( port );
	if( query ) free( query );
	if( fragment ) free( fragment );
	if( username ) free( username );
	if( password ) free( password );
	if( path ) free( path );
	if( prefix ) utstring_free( prefix );
	if( uri ) free( uri );

  log_info( "OUT client_uri_parse\n");

	return puri;
}

URI* client_uri_free( URI *uri )
{
	if ( uri ) {
		if( uri->host ) free( uri->host );
		if( uri->prefix ) free( uri->prefix );
		if( uri->query ) free( uri->query );
		if( uri->url ) free( uri->url );
		if( uri->sfn ) free( uri->sfn );
		free(uri);
	}
	return NULL;
}
