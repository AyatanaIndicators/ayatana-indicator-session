
#include <polkit-gnome/polkit-gnome.h>

gboolean pk_require_auth (LogoutDialogAction action);
gboolean pk_can_do_action (const gchar *action_id, PolKitResult * pol_result);

