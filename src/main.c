#include <stdio.h>

#include "glib.h"

#include "log.h"
#include "client.h"
#include "uriparse.h"

static gboolean say_hi_flag = FALSE;
static gboolean test_uri_flag = FALSE;
static char *loglevel_str;


static GOptionEntry entries[] =
{
  { "hello", 'h', 0, G_OPTION_ARG_NONE, &say_hi_flag, "Just Says Hi", NULL },
  { "testUri", 't', 0, G_OPTION_ARG_NONE, &test_uri_flag, "Test and print URI details", NULL },
  { "loglevels", 'l', 0, G_OPTION_ARG_STRING, &loglevel_str, "Sets Logging Level", "NONE" },
  { NULL }
};



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
  log_set_loglevel(loglevel_info);

  if (loglevel_str != NULL)
  {
    log_msg("Got debug level: %s\n", loglevel_str);

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

  if (say_hi_flag)
  {
    printf("G'day!\n");
    return 0;
  }

  if (argc > 0)
  {
    printf("Found %d arguments.\n", argc);
  }
  if (argc > 1)
  {
    printf(" Argument 1 is %s\n", argv[1]);
    if (test_uri_flag)
    {
      UriUriA *uri = uriparse_parse_uri(argv[1]);

      log_info("    host: %s\n", uriparse_get_host(uri));
      log_info("    path: %s\n", uriparse_get_path(uri));
      log_info("   query: %s\n", uriparse_get_query(uri));
      log_info("  scheme: %s\n", uriparse_get_scheme(uri));
    }
    else
    {
      client_authenticate(argv[1]);
    }
  }

  // From this point forward, we need X running.

#ifdef GTK_INIT

  GtkWidget *main_window;

  gtk_init(&argc, &argv);

  main_window = mainwindow_new();

  log_debug("Showing main window...\n");
  gtk_widget_show_all(main_window);

  log_debug("Entering main loop...\n");
  gtk_main();
#else
  printf("Main window is not enabled.\n");
#endif

  return 0;
}
