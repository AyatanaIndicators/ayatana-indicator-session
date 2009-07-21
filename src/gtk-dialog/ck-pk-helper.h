
#ifndef __CK_PK_HELPER_H__
#define __CK_PK_HELPER_H__ 1

#include <polkit-gnome/polkit-gnome.h>

gboolean pk_require_auth (LogoutDialogAction action);
gboolean pk_can_do_action (const gchar *action_id, PolKitResult * pol_result);

#endif /* __CK_PK_HELPER__ */
