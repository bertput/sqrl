#include <gtk/gtk.h>

#include "settings.h"
#include "version.h"

#include "mainwindow.h"

#include "log.h"


GtkWidget *status_bar;

void mainwindow_on_file_import_pipe_delimited(GtkMenuItem *in_menuitem, gpointer data);
void mainwindow_on_file_exit(GtkMenuItem *in_menuitem, gpointer data);

void mainwindow_on_edit_find(GtkMenuItem *in_menuitem, gpointer data);
void mainwindow_on_entry_new(GtkMenuItem *in_menuitem, gpointer data);
void mainwindow_on_help_about(GtkMenuItem *in_menuitem, gpointer data);



void
mainwindow_on_file_import_pipe_delimited(GtkMenuItem *in_menuitem,
                                         gpointer data)
{
  log_info("In mainwindow_on_file_import_pipe_delimited()\n");
}


void
mainwindow_on_file_exit(GtkMenuItem *in_menuitem,
                        gpointer data)
{
  gtk_main_quit();
}

void
mainwindow_on_edit_find(GtkMenuItem *in_menuitem,
                        gpointer data)
{
  log_info("In mainwindow_on_edit_find()\n");
}

void
mainwindow_on_entry_new(GtkMenuItem *in_menuitem,
                        gpointer data)
{
  log_info("In mainwindow_on_entry_new()\n");
}

void
mainwindow_on_help_about(GtkMenuItem *in_menuitem,
                         gpointer data)
{
  GtkWidget *about_box;

  const gchar  *authors[] = {"Bert Put", NULL};

  about_box = gtk_about_dialog_new();
  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_box), "sqrl");
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_box), FULLVERSION_STRING);
//  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_box), "(C) WAP Consulting");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_box), "SQRL");
  gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_box), authors);

  gtk_widget_show(about_box);
}

GtkWidget *
mainwindow_build_menuitem(gchar *menu_text,
                          gchar  accelerator_key,
                          GtkSignalFunc signal_handler,
                          GtkWidget *menu,
                          GtkAccelGroup *accel_group)
{
  GtkWidget *retval;

  if (menu_text == NULL)
  {
    retval = gtk_menu_item_new();
  }

  else
  {
    retval = gtk_menu_item_new_with_mnemonic(menu_text);
  }

  if (signal_handler != NULL)
  {
    gtk_signal_connect(GTK_OBJECT(retval),
                       "activate",
                       signal_handler,
                       NULL);
  }

  if (menu != NULL)
  {
    gtk_menu_append(GTK_MENU(menu), retval);
  }

  if (accel_group != NULL
   && (guint) accelerator_key != 0)
  {
    gtk_widget_add_accelerator( retval,
                                "activate",
                                accel_group,
                                (guint) accelerator_key,
                                GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE );
  }

  return retval;
}

void
mainwindow_create_menus(GtkWidget *in_menubar,
                        GtkAccelGroup *in_accel_group)
{
  GtkWidget *file_menu;
  GtkWidget *file_menuitem;
  GtkWidget *file_exit_menuitem;

  GtkWidget *edit_menu;
  GtkWidget *edit_menuitem;
  GtkWidget *edit_find_menuitem;

  GtkWidget *entry_menu;
  GtkWidget *entry_menuitem;
  GtkWidget *entry_new_menuitem;

  GtkWidget *view_menu;
  GtkWidget *view_menuitem;

  GtkWidget *help_menu;
  GtkWidget *help_menuitem;
  GtkWidget *help_about_menuitem;

  file_menu = gtk_menu_new();
  file_menuitem = mainwindow_build_menuitem("_File", 0, NULL, NULL, NULL);
  gtk_menu_bar_append(GTK_MENU_BAR(in_menubar), file_menuitem);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menuitem), file_menu);

  file_exit_menuitem =
    mainwindow_build_menuitem("E_xit",
                              0,
                              GTK_SIGNAL_FUNC(mainwindow_on_file_exit),
                              file_menu,
                              in_accel_group);


  edit_menu = gtk_menu_new();
  edit_menuitem = mainwindow_build_menuitem("_Edit", 0, NULL, NULL, NULL);
  gtk_menu_bar_append(GTK_MENU_BAR(in_menubar), edit_menuitem);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_menuitem), edit_menu);

  edit_find_menuitem =
    mainwindow_build_menuitem("_Find",
                              'f',
                              GTK_SIGNAL_FUNC(mainwindow_on_edit_find),
                              edit_menu,
                              in_accel_group);

#if 0
  view_menu = gtk_menu_new();
  view_menuitem = mainwindow_build_menuitem("_View", 0, NULL, NULL, NULL);
  gtk_menu_bar_append(GTK_MENU_BAR(in_menubar), view_menuitem);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menuitem), view_menu);

  view_graphical_timeline_menuitem =
    mainwindow_build_menuitem("_Graphical Timeline",
                              0,
                              GTK_SIGNAL_FUNC(mainwindow_on_view_graphical_timeline),
                              view_menu,
                              in_accel_group);
#endif // 0

  help_menu = gtk_menu_new();
  help_menuitem = mainwindow_build_menuitem("_Help", 0, NULL, NULL, NULL);
  gtk_menu_bar_append(GTK_MENU_BAR(in_menubar), help_menuitem);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menuitem), help_menu);

  help_about_menuitem =
    mainwindow_build_menuitem("_About",
                              0,
                              GTK_SIGNAL_FUNC(mainwindow_on_help_about),
                              help_menu,
                              NULL);



}

GtkWidget * mainwindow_new(void)
{
  GtkWidget *main_window;
  GtkWidget *main_vbox;
//  GtkWidget *hpaned_window;

  GtkWidget *menu_bar;

  GtkAccelGroup *accelGroup;


  log_info("In mainwindow_new()\n");

  settings_new();


  main_window =
    gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size(GTK_WINDOW (main_window),
                              settings_get_main_window_width(),
                              settings_get_main_window_height() );

  gtk_window_set_title(GTK_WINDOW (main_window),
                       settings_get_main_window_title());

  gtk_signal_connect(GTK_OBJECT (main_window),
                     "destroy",
                     GTK_SIGNAL_FUNC(gtk_main_quit),
                     NULL);


  accelGroup = gtk_accel_group_new();

  menu_bar = gtk_menu_bar_new();

  mainwindow_create_menus(menu_bar, accelGroup);
  gtk_window_add_accel_group(GTK_WINDOW (main_window), accelGroup);

  status_bar = gtk_statusbar_new();
  guint context =
    gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "main");
  gtk_statusbar_push(GTK_STATUSBAR(status_bar), context, "Ready.");


  main_vbox = gtk_vbox_new(FALSE, 0);

  // Pack the widgets in.
  gtk_box_pack_start(GTK_BOX(main_vbox), // box
                     menu_bar, // widget to pack into box
                     FALSE,    // center widget in allocated space?
                     FALSE,    // make widget expand into all available allocated space?
                     0);       // padding between widgets.

//  gtk_box_pack_start(GTK_BOX(main_vbox), hpaned_window, TRUE, TRUE, 0);

  gtk_box_pack_end(GTK_BOX(main_vbox), status_bar, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(main_window), main_vbox);




  return main_window;
}
