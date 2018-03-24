#include <stdlib.h>
#include <string.h>

#include "settings.h"

#include "log.h"

GKeyFile *settings_keyfile;

void
settings_new(void)
{
  settings_keyfile = g_key_file_new();

  GString *sqrl_ini_location_gstr = g_string_new(getenv("HOME"));
  g_string_append(sqrl_ini_location_gstr, "/.sqrl/sqrl.ini");

  gboolean fileOK =
    g_key_file_load_from_file(settings_keyfile,
                              sqrl_ini_location_gstr->str,
                              G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
                              NULL);

  g_string_free(sqrl_ini_location_gstr, TRUE);

  if (fileOK)
  {
    log_info("Settings file loaded OK.\n");
  }
  else
  {
    log_fatal("Failed to load settings file: sqrl.ini\n");
    exit(-1);
  }
}

void
settings_free(void)
{
  gchar *file_contents;

  file_contents =
    g_key_file_to_data(settings_keyfile, NULL, NULL);


  log_info(file_contents);

  GError *gerror = NULL;
  gsize chars_written;

  GString *sqrl_ini_location_gstr = g_string_new(getenv("HOME"));
  g_string_append(sqrl_ini_location_gstr, "/.sqrl/sqrl.ini");

  GIOChannel *g_io_channel =
    g_io_channel_new_file(sqrl_ini_location_gstr->str, "w", &gerror);

  g_string_free(sqrl_ini_location_gstr, TRUE);

//  GIOStatus gio_status =
    g_io_channel_write_chars( g_io_channel,
                              file_contents,
                              strlen(file_contents),
                             &chars_written,
                             &gerror);

  g_io_channel_shutdown(g_io_channel, TRUE, &gerror);

  g_key_file_free(settings_keyfile);
}


void
settings_set_sqrl_id_filename(gchar *filename)
{
  g_key_file_set_string(settings_keyfile, "sqrl", "id_filename", filename);
}

gchar *
settings_get_sqrl_id_filename(void)
{
  return g_key_file_get_string(settings_keyfile, "sqrl", "id_filename", NULL);
}


void
settings_set_sqrl_password(gchar *pwd)
{
  g_key_file_set_string(settings_keyfile, "sqrl", "password", pwd);
}

gchar *
settings_get_sqrl_password(void)
{
  return g_key_file_get_string(settings_keyfile, "sqrl", "password", NULL);
}


void
settings_set_sqrl_rescue_code(gchar *rescue_code)
{
  g_key_file_set_string(settings_keyfile, "sqrl", "rescue_code", rescue_code);
}

gchar *
settings_get_sqrl_rescue_code(void)
{
  return g_key_file_get_string(settings_keyfile, "sqrl", "rescue_code", NULL);
}

