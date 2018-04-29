#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtk.h>

#include "client.h"
#include "settings.h"
#include "log.h"
#include "uriparse.h"
#include "mainwindow.h"
#include "fifo.h"


static gboolean uri_test_flag = FALSE;
static char *loglevel_str;

//static char *PIDfilename = "/tmp/sqrl.PID";
static char *FIFOfilename = "/tmp/sqrl.FIFO";



static GOptionEntry entries[] =
{
  { "loglevels", 'l', 0, G_OPTION_ARG_STRING, &loglevel_str, "Sets Logging Level", "NONE" },
  { "URItest", 'u', 0, G_OPTION_ARG_NONE, &uri_test_flag, "Test and print URI details", NULL },
  { NULL }
};


void start_server(int argc, char *argv[]);

/**
 * The main entry point of this application.
 */
int main (int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new("");
  g_option_context_add_main_entries(context, entries, "");
  if (!g_option_context_parse (context, &argc, &argv, &error))
  {
    printf("option parsing failed: %s\n", error->message);
    return -1;
  }

  log_info("sqrl: Use -?, --help or --usage for assistance.\n");

  // Pre-emptively set log level in case it is not set.
//  log_set_loglevel(loglevel_debug);

  if (loglevel_str != NULL)
  {
    log_msg("Got log level: %s\n", loglevel_str);

    if (g_strcmp0(loglevel_str, "info") == 0)
    {
      log_set_loglevel(loglevel_info);
    }
    else if (g_strcmp0(loglevel_str, "debug") == 0)
    {
      log_set_loglevel(loglevel_debug);
    }
    else if (g_strcmp0(loglevel_str, "warn") == 0)
    {
      log_set_loglevel(loglevel_warn);
    }
    else if (g_strcmp0(loglevel_str, "error") == 0)
    {
      log_set_loglevel(loglevel_error);
    }
    else if (g_strcmp0(loglevel_str, "fatal") == 0)
    {
      log_set_loglevel(loglevel_fatal);
    }
    else
    {
      printf("Unknown log level: %s\n.  Expected one of info, debug, warn, error, fatal.\n", loglevel_str);

      return -1;
    }
  }

  if (argc > 0)
  {
    log_msg("Found %d arguments.\n", argc);
    start_server( argc, argv );
  }
  if (argc > 1)
  {
    log_msg(" Argument 1 is %s\n", argv[1]);
    if (uri_test_flag)
    {
      uri *uri = uriparse_parse_uri(argv[1]);

      log_info("    host: %s\n", uri->host);
      log_info("    path: %s\n", uri->path);
      log_info("   query: %s\n", uri->query);
      log_info("  scheme: %s\n", uri->scheme);
    }
    else
    {
      // We have a URL to authenticate to.
      // Try sending it to the server via FIFO.
      //
      int fifofd = open(FIFOfilename, O_WRONLY);
      if (fifofd < 0)
      {
        // There's no FIFO.
        //
        log_error("couldn't open FIFO at %s for write-only\n", FIFOfilename);

        // Start the server.
        start_server(argc, argv);

        //  When this thread returns control to here, wait 1 second, and try opening the FIFO again.
        log_debug("sleeping...\n");
        sleep(1);  // seconds

        fifofd = open(FIFOfilename, O_WRONLY);
        if (fifofd < 0)
        {
          log_error("Failed to open FIFO the second time.  Exiting.\n");
          return(-1);
        }
      }

      log_info("sending message to server: %s\n", argv[1]);
      write(fifofd, argv[1], strlen(argv[1]));
      close(fifofd);

    }
  }

  return 0;
}



/**
 * We are starting server by spawning a thread.
 */
void start_server(int argc, char *argv[])
{
  log_info("In start_server()\n");

  int forked_pid = fork();

  if (forked_pid == 0)
  {
    log_debug("This is the child process.\n");
    log_debug("  Returning to main()\n");
    return;
  }
  else if (forked_pid > 0)
  {
    log_debug("This is the parent process.\n");
//    return;
  }
  else if (forked_pid < 0)
  {
    log_error("Failed to fork!\n");
    exit(-1);
  }

  // If we got to here, we are the original process and can safely start the GUI.

  log_debug("Calling sqrl_init()\n");

  sqrl_init();

  settings_new();

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

  // From this point forward, we need X running.

  gtk_init(&argc, &argv);

  GtkWidget *main_window;

  main_window = mainwindow_new();

  log_debug("Showing main window...\n");
  gtk_widget_show_all(main_window);

  fifo_new();

//      client_authenticate(argv[1]);

  log_debug("Entering main loop...\n");
  gtk_main();

  log_debug("Removing FIFO\n");
  unlink(FIFOfilename);


  log_debug("Calling sqrl_stop()\n");
  sqrl_stop();

  // Have to call exit() here otherwise control is returned to main()!
  //
  exit(0);
}
