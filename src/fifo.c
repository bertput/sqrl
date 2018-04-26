#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtk.h>

#include "fifo.h"
#include "client.h"

#include "log.h"

static char *FIFOfilename = "/tmp/sqrl.FIFO";


gboolean
fifo_gio_incoming_data(GIOChannel *source,
                       GIOCondition condition,
                       gpointer data);


/**
 * Create FIFO for incoming requests and add watch for main loop.
 */
void fifo_new(void)
{
  int mkfiforv = mkfifo(FIFOfilename, 0600);
  if (mkfiforv < 0)
  {
    log_error("failed to make fifo\n");
    perror("FIFO");
    exit(-2);
  }


  log_info("opening the FIFO for read only\n");

  int fifofd = open(FIFOfilename, O_RDONLY | O_NONBLOCK);

  if (fifofd < 0)
  {
    log_error("open of FIFO failed.\n");
    perror("FIFO");
    exit(-3);
  }
  else
  {
    log_debug("FIFO is opened\n");
  }

  GIOChannel *fifo_gio_channel = g_io_channel_unix_new (fifofd);

  guint event_source_id =
    g_io_add_watch( fifo_gio_channel,
                    G_IO_IN,
                   &fifo_gio_incoming_data,
                    NULL);
}


gboolean
fifo_gio_incoming_data(GIOChannel *source,
                       GIOCondition condition,
                       gpointer data)
{
  char buf[1000];
  gsize bytes_read = 0;

  memset(buf, 0, 1000);

  GIOStatus gio_status =
    g_io_channel_read_chars( source,
                             buf,
                             999,
                            &bytes_read,
                             NULL);

  log_debug("We got %d chars.  <%s>\n", bytes_read, buf);

  client_authenticate(buf);

  return TRUE; // Keep active.
}
