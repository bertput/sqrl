#ifndef PROGRESS_WINDOW_H
#define PROGRESS_WINDOW_H

#include <gtk/gtk.h>

void UpdateProgress (long pos, long len);

void StartProgress (void);

void EndProgress (void);




#endif // PROGRESS_WINDOW_H
