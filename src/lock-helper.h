#ifndef LOCK_HELPER_H__
#define LOCK_HELPER_H__

#include <libdbusmenu-glib/menuitem.h>

typedef void (*gdm_autologin_cb_t) (void);

gboolean will_lock_screen (void);
void lock_screen (DbusmenuMenuitem * mi, gpointer data);
gboolean lock_screen_setup (gpointer data);

#endif /* LOCK_HELPER_H__ */
