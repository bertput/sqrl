#include <gtk/gtk.h>

#include "requestwindow.h"

#include "log.h"



void
requestwindow_new(char *window_title,
                  GString *response_gstr,
                  int   request_str_length)
{
  GtkWidget *requestwindow;
  GtkWidget *requesttext_input_field;

  log_info("In requestwindow_new.\n");

  requestwindow =
    gtk_dialog_new_with_buttons(window_title,
                                 NULL,
                                 0,
                                 GTK_STOCK_OK,
                                 GTK_RESPONSE_OK,
                                 NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(requestwindow), GTK_RESPONSE_OK);

  // Create input field for search term
  requesttext_input_field = gtk_entry_new();
  gtk_entry_set_activates_default(GTK_ENTRY(requesttext_input_field), TRUE);
  gtk_entry_set_max_length(GTK_ENTRY(requesttext_input_field), request_str_length);

  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(requestwindow)->vbox), requesttext_input_field);

  gtk_widget_show_all(requestwindow);

  gint response = gtk_dialog_run(GTK_DIALOG(requestwindow));
  log_debug("Response: %d\n", response);

  g_string_assign(response_gstr, gtk_entry_get_text(GTK_ENTRY(requesttext_input_field)));

  gtk_widget_destroy(requestwindow);

  return;
}
