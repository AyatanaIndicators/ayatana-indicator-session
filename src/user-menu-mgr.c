/*
Copyright 2011 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libdbusmenu-glib/client.h>
#include "user-menu-mgr.h"
#include "settings-helper.h"
#include "dbus-shared-names.h"
#include "dbusmenu-shared.h"
#include "lock-helper.h"
#include "users-service-dbus.h"

struct ActivateUserSessionData
{
  UserMenuMgr * menu_mgr;
  AccountsUser * user;
};

struct _UserMenuMgr
{
  GObject parent_instance;
  UsersServiceDbus* users_dbus_interface;
  DbusmenuMenuitem* root_item;
  DbusmenuMenuitem * guest_mi;
  SessionDbus* session_dbus_interface;  
  GSettings * lockdown_settings;
  gboolean greeter_mode;
};

static void activate_new_session (DbusmenuMenuitem * mi,
                                  guint timestamp,
                                  gpointer user_data);
static void activate_user_session (DbusmenuMenuitem *mi,
                                   guint timestamp,
                                   gpointer user_data);
static void activate_user_accounts (DbusmenuMenuitem *mi,
                                    guint timestamp,
                                    gpointer user_data);
static gint compare_users_by_username (gconstpointer a,
                                       gconstpointer b);
static void activate_user_accounts (DbusmenuMenuitem *mi,
                                    guint timestamp,
                                    gpointer user_data);                                      
static void user_menu_mgr_rebuild_items (UserMenuMgr *self);
static void on_user_list_changed (UsersServiceDbus *service,
                                  UserMenuMgr * umm);

static void on_guest_logged_in_changed (UsersServiceDbus * users_dbus,
                                        UserMenuMgr      * self);

static void on_user_logged_in_changed (UsersServiceDbus  * users_dbus,
                                       AccountsUser      * user,
                                       UserMenuMgr       * self);
static gboolean is_this_guest_session (void);
static void activate_guest_session (DbusmenuMenuitem * mi,
                                    guint timestamp,
                                    gpointer user_data);
                                    

G_DEFINE_TYPE (UserMenuMgr, user_menu_mgr, G_TYPE_OBJECT);

/***
****
***/

static void
user_menu_mgr_init (UserMenuMgr *self)
{
  self->lockdown_settings = g_settings_new (LOCKDOWN_SCHEMA);
  self->users_dbus_interface = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);
  self->root_item = dbusmenu_menuitem_new ();
  g_signal_connect (self->users_dbus_interface, "user-list-changed",
                    G_CALLBACK (on_user_list_changed), self);
  g_signal_connect (self->users_dbus_interface, "user-logged-in-changed",
                    G_CALLBACK(on_user_logged_in_changed), self);
  g_signal_connect (self->users_dbus_interface, "guest-logged-in-changed",
                    G_CALLBACK(on_guest_logged_in_changed), self);
}

static void
user_menu_mgr_dispose (GObject *object)
{
  UserMenuMgr * menu_mgr = USER_MENU_MGR(object);

  g_clear_object (&menu_mgr->users_dbus_interface);
  g_clear_object (&menu_mgr->lockdown_settings);
}

static void
user_menu_mgr_finalize (GObject *object)
{
  /* TODO: Add deinitalization code here */
  G_OBJECT_CLASS (user_menu_mgr_parent_class)->finalize (object);
}

static void
user_menu_mgr_class_init (UserMenuMgrClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = user_menu_mgr_dispose;
  object_class->finalize = user_menu_mgr_finalize;
}

/***
****
***/

static GQuark
get_menuitem_quark (void)
{
  static GQuark q = 0;

  if (G_UNLIKELY(!q))
    {
      q = g_quark_from_static_string ("menuitem");
    }

  return q;
}

static DbusmenuMenuitem*
user_get_menuitem (AccountsUser * user)
{
  return g_object_get_qdata (G_OBJECT(user), get_menuitem_quark());
}

static void
user_set_menuitem (AccountsUser * user, DbusmenuMenuitem * mi)
{
  g_message ("%s %s() associating user %s with mi %p", G_STRLOC, G_STRFUNC, accounts_user_get_user_name(user), mi);
  g_object_set_qdata_full (G_OBJECT(user), get_menuitem_quark(), g_object_ref(G_OBJECT(mi)), g_object_unref);
}

static GQuark
get_name_collision_quark (void)
{
  static GQuark q = 0;

  if (G_UNLIKELY(!q))
    {
      q = g_quark_from_static_string ("name-collision");
    }

  return q;
}

static gboolean
get_user_name_collision (AccountsUser * user)
{
  return g_object_get_qdata (G_OBJECT(user), get_name_collision_quark()) != NULL;
}

static void
set_user_name_collision (AccountsUser * user, gboolean b)
{
  g_object_set_qdata (G_OBJECT(user), get_name_collision_quark(), GINT_TO_POINTER(b));
}

/***
****
***/

static void
update_menuitem_icon (DbusmenuMenuitem * mi, AccountsUser * user)
{
  const gchar * str = accounts_user_get_icon_file (user);
  
  if (!str || !*str)
    {
      str = USER_ITEM_ICON_DEFAULT;
    }

  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_ICON, str);
}

static void
on_user_icon_file_changed (AccountsUser * user, GParamSpec * pspec, DbusmenuMenuitem * mi)
{
  update_menuitem_icon (mi, user);
}

static DbusmenuMenuitem*
create_user_menuitem (UserMenuMgr * menu_mgr, AccountsUser * user)
{
  DbusmenuMenuitem * mi = dbusmenu_menuitem_new ();
  dbusmenu_menuitem_property_set (mi,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  USER_ITEM_TYPE);

  /* set the name property */
  const gchar * const real_name = accounts_user_get_real_name (user);
  const gchar * const user_name = accounts_user_get_user_name (user);
  char * str = get_user_name_collision (user)
             ? g_strdup_printf ("%s (%s)", real_name, user_name)
             : g_strdup (real_name);
  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, str);
  g_free (str);

  /* set the logged-in property */
  const gboolean is_logged_in = users_service_dbus_is_user_logged_in (menu_mgr->users_dbus_interface, user);
  dbusmenu_menuitem_property_set_bool (mi,
                                       USER_ITEM_PROP_LOGGED_IN,
                                       is_logged_in);

  /* set the icon property */
  update_menuitem_icon (mi, user);
  g_signal_connect (user, "notify::icon-file", G_CALLBACK(on_user_icon_file_changed), mi);

  /* set the is-current-user property */
  dbusmenu_menuitem_property_set_bool (mi,
                                       USER_ITEM_PROP_IS_CURRENT_USER,
                                       !g_strcmp0 (user_name, g_get_user_name()));

  /* set the activate callback */
  struct ActivateUserSessionData * data = g_new (struct ActivateUserSessionData, 1);
  data->user = user;
  data->menu_mgr = menu_mgr;
  g_signal_connect_data (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                         G_CALLBACK (activate_user_session),
                         data, (GClosureNotify)g_free,
                         0);

  /* give this AccountsUser a hook back to this menuitem */
  user_set_menuitem (user, mi);

  /* done */
  return mi;
}

static void
on_guest_logged_in_changed (UsersServiceDbus * users_service_dbus,
                            UserMenuMgr      * self)
{
  if (self->guest_mi != NULL)
    {
      const gboolean b = users_service_dbus_is_guest_logged_in (users_service_dbus);
      dbusmenu_menuitem_property_set_bool (self->guest_mi, USER_ITEM_PROP_LOGGED_IN, b);
    }
}

/* When a user's login state changes,
   update the corresponding menuitem's LOGGED_IN property */
static void
on_user_logged_in_changed (UsersServiceDbus  * users_service_dbus,
                           AccountsUser      * user,
                           UserMenuMgr       * self)
{
  DbusmenuMenuitem * mi = user_get_menuitem (user);

  if (mi != NULL)
    {
      const gboolean b = users_service_dbus_is_user_logged_in (users_service_dbus, user);
      dbusmenu_menuitem_property_set_bool (mi, USER_ITEM_PROP_LOGGED_IN, b);
    }
}

/* Builds up the menu for us */
static void 
user_menu_mgr_rebuild_items (UserMenuMgr *self)
{
  GList *u;
  gboolean can_activate;
  GList *children;

  /* Check to see which menu items we're allowed to have */
  can_activate = users_service_dbus_can_activate_session (self->users_dbus_interface) &&
      !g_settings_get_boolean (self->lockdown_settings, LOCKDOWN_KEY_USER);

  /* Remove the old menu items if that makes sense */
  children = dbusmenu_menuitem_take_children (self->root_item);
  g_list_foreach (children, (GFunc)g_object_unref, NULL);
  g_list_free (children);

  /* Set to NULL in case we don't end up building one */
  self->guest_mi = NULL;

  /* Build all of the user switching items */
  if (can_activate)
  {
    /* TODO: This needs to be updated once the ability to query guest session support is available */
    GList * users = users_service_dbus_get_user_list (self->users_dbus_interface);
    const gint user_count = g_list_length(users);
    const gboolean guest_enabled = users_service_dbus_guest_session_enabled (self->users_dbus_interface);
    const gboolean is_guest = guest_enabled && is_this_guest_session();
    const gboolean other_users_exist = user_count>1 || (is_guest && user_count>0);
    const gboolean visible = !self->greeter_mode && should_show_user_menu() && other_users_exist;
    session_dbus_set_user_menu_visibility (self->session_dbus_interface, visible);
    
    /* TODO we should really return here if the menu is not going to be shown. */
    
    DbusmenuMenuitem * switch_menuitem = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_property_set (switch_menuitem,
                                    DBUSMENU_MENUITEM_PROP_TYPE,
                                    MENU_SWITCH_TYPE);
    dbusmenu_menuitem_property_set (switch_menuitem,
                                    MENU_SWITCH_USER,
                                    g_get_user_name());
    dbusmenu_menuitem_child_append (self->root_item, switch_menuitem);
    g_signal_connect (G_OBJECT (switch_menuitem),
                      DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                      G_CALLBACK (activate_new_session),
                      self);
    
    if ( !is_this_guest_session () && guest_enabled)
    {
      self->guest_mi = dbusmenu_menuitem_new ();
      dbusmenu_menuitem_property_set (self->guest_mi,
                                      DBUSMENU_MENUITEM_PROP_TYPE,
                                      USER_ITEM_TYPE);
      dbusmenu_menuitem_property_set (self->guest_mi,
                                      USER_ITEM_PROP_NAME,
                                      _("Guest Session"));
      on_guest_logged_in_changed (self->users_dbus_interface, self);
      dbusmenu_menuitem_child_append (self->root_item, self->guest_mi);
      g_signal_connect (G_OBJECT (self->guest_mi),
                        DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK (activate_guest_session),
                        self);
    }
    else{
      session_dbus_set_users_real_name (self->session_dbus_interface,
                                        _("Guest"));      
    }
    
    

    users = g_list_sort (users, compare_users_by_username);

    for (u = users; u != NULL; u = g_list_next (u))
      {
        AccountsUser * user = u->data;

        DbusmenuMenuitem * mi = create_user_menuitem (self, user);
        dbusmenu_menuitem_child_append (self->root_item, mi);

        const char * const user_name = accounts_user_get_user_name (user);
        if (!g_strcmp0 (user_name, g_get_user_name()))
          {
            const char * const real_name = accounts_user_get_real_name (user);
            session_dbus_set_users_real_name (self->session_dbus_interface, real_name);
          }
      }

    g_list_free(users);
  }

  // Add the user accounts and separator
  DbusmenuMenuitem * separator1 = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (separator1,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_SEPARATOR);
  dbusmenu_menuitem_child_append (self->root_item, separator1);

  DbusmenuMenuitem * user_accounts_item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (user_accounts_item,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_DEFAULT);
  dbusmenu_menuitem_property_set (user_accounts_item,
                                  DBUSMENU_MENUITEM_PROP_LABEL,
                                  _("User Accounts…"));

  g_signal_connect (G_OBJECT (user_accounts_item),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (activate_user_accounts),
                    NULL);
                                  
  dbusmenu_menuitem_child_append (self->root_item, user_accounts_item); 


}

/* Check to see if the lockdown key is protecting from
   locking the screen.  If not, lock it. */
static void
lock_if_possible (UserMenuMgr * menu_mgr)
{
  if (!g_settings_get_boolean (menu_mgr->lockdown_settings, LOCKDOWN_KEY_SCREENSAVER))
    {
      lock_screen(NULL, 0, NULL);
    }
}

/* Starts a new generic session */
static void
activate_new_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
  UserMenuMgr * menu_mgr = USER_MENU_MGR (user_data);
  g_return_if_fail (menu_mgr != NULL);

  lock_if_possible (menu_mgr);
  users_service_dbus_show_greeter (menu_mgr->users_dbus_interface);
}

/* Activates a session for a particular user. */
static void
activate_user_session (DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  struct ActivateUserSessionData * data = user_data;

  lock_if_possible (data->menu_mgr);
  users_service_dbus_activate_user_session (data->menu_mgr->users_dbus_interface, data->user);
}

/* Comparison function to look into the UserData struct
   to compare by using the username value */
static gint
compare_users_by_username (gconstpointer ga, gconstpointer gb)
{
  AccountsUser * a = ACCOUNTS_USER(ga);
  AccountsUser * b = ACCOUNTS_USER(gb);

  const int ret = g_strcmp0 (accounts_user_get_real_name (a),
                             accounts_user_get_real_name (b));

  if (!ret) /* names are the same, so both have a name collision */
    {
      set_user_name_collision (a, TRUE);
      set_user_name_collision (b, TRUE);
    }

  return ret;
}

static void
activate_user_accounts (DbusmenuMenuitem *mi,
                          guint timestamp,
                          gpointer user_data)
{
  GError * error = NULL;
  if (!g_spawn_command_line_async("gnome-control-center user-accounts", &error))
  {
    g_warning("Unable to show control centre: %s", error->message);
    g_error_free(error);
  }
}

/* Signal called when a user is added.
   It updates the count and rebuilds the menu */
static void
on_user_list_changed (UsersServiceDbus * service   G_GNUC_UNUSED, 
                      UserMenuMgr      * user_mgr)
{
  g_return_if_fail (USER_IS_MENU_MGR(user_mgr));

  user_menu_mgr_rebuild_items (user_mgr);
}

DbusmenuMenuitem*
user_mgr_get_root_item (UserMenuMgr* self)
{
  return self->root_item;
}

/* Checks to see if we should show the guest session item.
   System users shouldn't have guest account shown.
   Mostly this would be the case of the guest user itself. */
static gboolean
is_this_guest_session (void)
{
  return geteuid() < 500;
}

/* Called when someone clicks on the guest session item. */
static void
activate_guest_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
  UserMenuMgr * menu_mgr = USER_MENU_MGR (user_data);
  g_return_if_fail (menu_mgr != NULL);

  lock_if_possible (menu_mgr);
  users_service_dbus_activate_guest_session (menu_mgr->users_dbus_interface);
}


/*
 * Clean Entry Point 
 */
UserMenuMgr* user_menu_mgr_new (SessionDbus* session_dbus, gboolean greeter_mode)
{
  UserMenuMgr* user_mgr = g_object_new (USER_TYPE_MENU_MGR, NULL);
  user_mgr->greeter_mode = greeter_mode;
  user_mgr->session_dbus_interface = session_dbus;
  user_menu_mgr_rebuild_items (user_mgr);
  return user_mgr;
}
  

