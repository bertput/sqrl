#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <resolv.h>
#include <arpa/inet.h>
#include <string.h>


#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>


#include <gtk/gtk.h>

#include "client.h"
#include "log.h"
#include "settings.h"
#include "progresswindow.h"
#include "requestwindow.h"

#include "uriparse.h"


// Client currently supports only a single user ID...
Sqrl_User single_user = NULL;


char statusText[4][10] = {
    "SUCCESS",
    "FAILED",
    "CANCELLED",
    "WORKING"
};

const SSL_METHOD *method;
SSL_CTX *ctx;




void error(const char *msg) { perror(msg); exit(0); }


int client_authenticate(char *in_url)
{
  log_info("Authenticating against website: %s\n", in_url);

  /*
   * Initialize openssl
   */
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();

  /*
   * initialize SSL library and register algorithms
   */
  if (SSL_library_init() < 0)
  {
    log_fatal("Could not initialize the OpenSSL library !\n");
  }

  /*
   * Set SSLv2 client hello, also announce SSLv3 and TLSv1
   */
  method = SSLv23_client_method();

  /*
   * Try to create a new SSL context
   */
  if ( (ctx = SSL_CTX_new(method)) == NULL)
  {
    log_fatal("Unable to create a new SSL context structure.\n");
  }
  else
  {
    log_info("Created new ctx at %p\n", ctx);
  }


  /*
   * Disabling SSLv2 will leave v3 and TSLv1 for negotiation
   */
  log_info("Setting options on ctx at %p\n", ctx);
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);


  Sqrl_Transaction_Status sqrlTransactionStatus =
    sqrl_client_begin_transaction(SQRL_TRANSACTION_AUTH_IDENT, NULL, in_url, strlen(in_url));

  if (sqrlTransactionStatus == SQRL_TRANSACTION_STATUS_SUCCESS)
  {
    log_info("Successfully logged into %s\n", in_url);
  }
  else
  {
    printf("*** ERROR: failed to log into %s with status %d (%s)\n", in_url, sqrlTransactionStatus, statusText[sqrlTransactionStatus]);
    return(-1);
  }

  return 0;
}



bool client_onAuthenticationRequired(
    Sqrl_Transaction transaction,
    Sqrl_Credential_Type credentialType )
{
  GString *credential_gstr = g_string_new("");

  log_info("in client_onAuthenticationRequired\n");

  switch( credentialType )
  {
  case SQRL_CREDENTIAL_PASSWORD:
    log_info("%10s: %s\n", "AUTH_REQ", "Password" );
    requestwindow_new("Enter full password", credential_gstr, 1024);
    break;

  case SQRL_CREDENTIAL_HINT:
    log_info("%10s: %s\n", "AUTH_REQ", "Hint" );
    int hint_len = sqrl_user_get_hint_length( sqrl_transaction_user( transaction ));
    requestwindow_new("Enter Hint", credential_gstr, hint_len);
    break;

  case SQRL_CREDENTIAL_RESCUE_CODE:
    log_info("%10s: %s\n", "AUTH_REQ", "Rescue Code" );
    requestwindow_new("Enter Rescue Code", credential_gstr, 1024);
    break;

  case SQRL_CREDENTIAL_NEW_PASSWORD:
    log_info("%10s: %s\n", "AUTH_REQ", "New Password" );
    requestwindow_new("Enter NEW password", credential_gstr, 1024);
    break;

  default:
    log_info("OUT client_onAuthenticationRequired with FALSE\n");
    return false;
  }

  log_debug("Got credential %s (%d chars)\n", credential_gstr->str, credential_gstr->len);

  sqrl_client_authenticate( transaction, credentialType, credential_gstr->str, credential_gstr->len);

  g_string_free(credential_gstr, TRUE);

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
GtkWidget *client_progress_window = NULL;


int client_onProgress( Sqrl_Transaction transaction, int p )
{

  if( !showingProgress )
  {
    showingProgress = true;

    StartProgress();
  }
  else
  {
    UpdateProgress(p, 100);

    if (p >= 100)
    {
      showingProgress = false;
      EndProgress();
    }
  }

#if 0
  if( !showingProgress )
  {
    // Transaction type
    showingProgress = true;
    nextProgress = 2;
    printf( "%10s: ", transactionType[sqrl_transaction_type(transaction)] );
  }

  const char sym[] = "|****";
  while( p >= nextProgress )
  {
    if( nextProgress != 100 )
    {
      printf( "%c", sym[nextProgress%5] );
    }
    nextProgress += 2;
  }

  if( p >= 100 )
  {
    printf( "\n" );
    showingProgress = false;
  }
  fflush( stdout );
#endif // 0

  return 1;
}

void client_onTransactionComplete( Sqrl_Transaction transaction )
{
  log_info("in client_onTransactionComplete\n");

  Sqrl_Transaction_Type type = sqrl_transaction_type(transaction);
  Sqrl_Transaction_Status status = sqrl_transaction_status(transaction);
  Sqrl_User user = sqrl_transaction_user(transaction);

  log_info("%10s: %s\n", transactionType[type], statusText[status] );

  if( status == SQRL_TRANSACTION_STATUS_SUCCESS )
  {
    switch( type )
    {
    case SQRL_TRANSACTION_IDENTITY_LOAD:
		if( single_user != NULL ) {
			// We already have a user loaded, so release it first.
			sqrl_user_release( single_user );
		}
		// Hold a reference to the newly loaded user.
		single_user = sqrl_user_hold( user );
      break;
#if 0
    case SQRL_TRANSACTION_IDENTITY_GENERATE:
      gen_user = sqrl_user_hold( user );
      char *rc = sqrl_user_get_rescue_code( transaction );

      if( rc )
      {
        strcpy( gen_rescue_code, rc );
        log_info("%10s: %s\n", "GEN_RC", gen_rescue_code );
        sqrl_client_export_user( gen_user, NULL, SQRL_EXPORT_ALL, SQRL_ENCODING_BASE64 );
      }
      else
      {
        printf( "RC not retrieved\n" );
        exit(1);
      }
      break;

    case SQRL_TRANSACTION_IDENTITY_SAVE:
      if( !gen_data )
      {
        size_t len = sqrl_transaction_string( transaction, NULL, 0 );
        if( len )
        {
          gen_data = malloc( ++len );
          sqrl_transaction_string( transaction, gen_data, &len );
          sqrl_user_unique_id( gen_user, gen_uid );
          log_info("%10s: %s\n", "SAVE_UID", gen_uid );
          log_info("%10s: %s\n", "SAVE_DATA", gen_data );
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

  if( strlen( buf ) == 0 )
  {
    log_info("%10s: %s\n", "SAVE_SUG", "New Identity" );
  }
  else
  {
    log_info("%10s: %s\n", "SAVE_SUG", buf );
  }
}

Sqrl_User client_onSelectUser (Sqrl_Transaction transaction)
{
	log_info("in client_onSelectUser\n");
	if( single_user == NULL ) {
		char *sqrl_id_filename = settings_get_sqrl_id_filename();
		log_info("Using identity file at %s with strlen %d\n", sqrl_id_filename, strlen(sqrl_id_filename));

		Sqrl_Transaction_Status sqrlTransactionStatus =
			sqrl_client_begin_transaction(
				SQRL_TRANSACTION_IDENTITY_LOAD,
				NULL,
				sqrl_id_filename,
				strlen(sqrl_id_filename));

		if (sqrlTransactionStatus == SQRL_TRANSACTION_STATUS_SUCCESS)
		{
			log_info("Successfully loaded identity from %s\n", sqrl_id_filename);
		}
		else
		{
			printf("*** ERROR: failed to load identity from %s\n", sqrl_id_filename);
			exit(-1);
		}
	}
	return single_user;
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

//  X509                *cert = NULL;
//  X509_NAME       *certname = NULL;
  SSL *ssl;

  log_info( "Dest URL: %s\n", url );
  log_info( "Payload : %s\n", payload );

//  UT_string *payload_decode_utstr = sqrl_b64u_decode(NULL, payload, strlen(payload));
//  log_info( " (Decoded) : %s\n", payload_decode_utstr->d);
//  utstring_free(payload_decode_utstr);

  uri *uri = uriparse_parse_uri(url);

  log_info("    host: %s\n", uri->host);
  log_info("    path: %s\n", uri->path);
  log_info("   query: %s\n", uri->query);
  log_info("  scheme: %s\n", uri->scheme);
//  log_info("     url: %s\n", uri->url);

  /*
   * Create new SSL connection state object
   */
  log_info("Creating ssl object from ctx at %p\n", ctx);
  ssl = SSL_new(ctx);
  log_info("         ssl object at %p\n", ssl);

  /* create the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    error("ERROR opening socket");
  }

  /* lookup the ip address */
  server = gethostbyname(uri->host);

  if (server == NULL)
  {
    error("ERROR, no such host");
  }

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

  /*
   * Attach the SSL session to the socket descriptor
   */
  log_info("Setting FD using ssl object at %p\n", ssl);
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
  log_info("Connecting using ssl object at %p\n", ssl);
  int ssl_connect_rc =  SSL_connect(ssl);

  if ( ssl_connect_rc == 1 )
  {
    log_info("Successfully enabled SSL/TLS session to: %s.\n", uri->host);
  }
  else
  {
    log_fatal("Error %d: Could not build a SSL session to: %s.\n", ssl_connect_rc, uri->host);
    error("ssl_connect");
  }

  GString *message_gstr = g_string_new("");
  g_string_append_printf(message_gstr, "POST /%s?%s HTTP/1.0\r\n", uri->path, uri->query);
  g_string_append_printf(message_gstr, "Host: %s\r\n", uri->host);
  g_string_append_printf(message_gstr, "User-Agent: SQRL/1\r\n");
  g_string_append_printf(message_gstr, "Content-type: application/x-www-form-urlencoded\r\n");
  g_string_append_printf(message_gstr, "Content-Length: %ld\r\n", payload_len);
  g_string_append_printf(message_gstr, "\r\n");
  g_string_append_printf(message_gstr, "%s", payload);


  /* send the request */
  total = message_gstr->len;
  char *message = message_gstr->str;
  log_info("    sending message:\n%s\n", message);
  sent = 0;
  do
  {
    log_info("Writing to ssl object at %p\n", ssl);
    bytes = SSL_write(ssl,message+sent,total-sent);

    if (bytes < 0)
    {
      error("ERROR writing message to socket");
    }
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

  do
  {
    log_info("Reading from ssl object at %p\n", ssl);
    received = SSL_read(ssl, response, 1000);

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

#if 0
  int shutdown_rc = 0;
  do
  {
    log_info("Sending SSL shutdown\n");
    shutdown_rc = SSL_shutdown(ssl);
  } while (shutdown_rc == 1);
#endif // 0

  log_info("Freeing ssl object at %p\n", ssl);
  SSL_free(ssl);

  /* close the socket */
  close(sockfd);

  uriparse_free_uri(uri);

  /* process response */
  log_info("Response:\n%s\n",response_gstr->str);

  // Extract only the body of the response and send it to sqrl_client_receive

  gchar *response_body = g_strstr_len(response_gstr->str, response_gstr->len, "\r\n\r\n");

  if (response_body == NULL)
  {
    error("******* BODY NOT FOUND\n");
  }
  else
  {
    GString *response_body_gstr = g_string_new(g_strchug(response_body));

    log_info("Response BODY:\n%s\n", response_body_gstr->str);
//    UT_string *response_decode_utstr = sqrl_b64u_decode(NULL, response_body_gstr->str, response_body_gstr->len);
//    log_info( " (Decoded) : %s\n", response_decode_utstr->d);
//    utstring_free(response_decode_utstr);

    sqrl_client_receive(transaction, response_body_gstr->str, response_body_gstr->len);
  }



  log_info("OUT client_onSend\n");
}

