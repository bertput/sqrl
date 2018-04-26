/**
 * @file settings.h
 *
 * Interface to the settings functionality.
 */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "glib.h"

void
settings_new(void);

void
settings_free(void);



void
settings_set_sqrl_id_filename(gchar *filename);

gchar *
settings_get_sqrl_id_filename(void);


void
settings_set_sqrl_password(gchar *pwd);

gchar *
settings_get_sqrl_password(void);


void
settings_set_sqrl_rescue_code(gchar *rescue_code);

gchar *
settings_get_sqrl_rescue_code(void);

void
settings_set_main_window_title(gchar *title);

gchar *
settings_get_main_window_title(void);

void
settings_set_main_window_width(gint width);

gint
settings_get_main_window_width(void);

void
settings_set_main_window_height(gint height);

gint
settings_get_main_window_height(void);


#endif /* _SETTINGS_H_ */
