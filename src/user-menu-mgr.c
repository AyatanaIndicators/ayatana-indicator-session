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

static GSettings* settings = NULL;
static DbusmenuMenuitem  *switch_menuitem = NULL;

struct _UserMenuMgr
{
	GObject parent_instance;
  UsersServiceDbus* users_dbus_interface;
  DbusmenuMenuitem* root_item;
  gint user_count;
  SessionDbus* session_dbus_interface;  
};

static void activate_new_session (DbusmenuMenuitem * mi,
                                  guint timestamp,
                                  gpointer user_data);
static void activate_user_session (DbusmenuMenuitem *mi,
                                   guint timestamp,
                                   gpointer user_data);
static gint compare_users_by_username (const gchar *a,
                                       const gchar *b);
static void activate_online_accounts (DbusmenuMenuitem *mi,
                                      guint timestamp,
                                      gpointer user_data);
static void user_menu_mgr_rebuild_items (UserMenuMgr *self,
                                         gboolean greeter_mode);
static gboolean check_new_session ();
static void user_change (UsersServiceDbus *service,
                         const gchar      *user_id,
                         gpointer          user_data);

static void ensure_settings_client ();
static gboolean check_guest_session (void);
static void activate_guest_session (DbusmenuMenuitem * mi,
                                    guint timestamp,
                                    gpointer user_data);


G_DEFINE_TYPE (UserMenuMgr, user_menu_mgr, G_TYPE_OBJECT);


static void
user_menu_mgr_init (UserMenuMgr *self)
{
  self->users_dbus_interface = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);
  self->root_item = dbusmenu_menuitem_new ();
  g_signal_connect (G_OBJECT (self->users_dbus_interface),
                    "user-added",
                    G_CALLBACK (user_change),
                    self);
  g_signal_connect (G_OBJECT (self->users_dbus_interface),
                    "user-deleted",
                    G_CALLBACK (user_change),
                    self);
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
	object_class->finalize = user_menu_mgr_finalize;
}

/* Builds up the menu for us */
static void 
user_menu_mgr_rebuild_items (UserMenuMgr *self, gboolean greeter_mode)
{
  DbusmenuMenuitem *mi = NULL;
  DbusmenuMenuitem *guest_mi = NULL;
  GList *u;
  UserData *user;
  gboolean can_activate;
  GList *children;

  /* Make sure we have a valid GConf client, and build one
     if needed */
  ensure_settings_client ();

  /* Check to see which menu items we're allowed to have */
  can_activate = users_service_dbus_can_activate_session (self->users_dbus_interface) &&
      !g_settings_get_boolean (settings, LOCKDOWN_KEY_USER);

  /* Remove the old menu items if that makes sense */
  children = dbusmenu_menuitem_take_children (self->root_item);
  g_list_foreach (children, (GFunc)g_object_unref, NULL);
  g_list_free (children);

  /* Set to NULL just incase we don't end up building one */
  users_service_dbus_set_guest_item(self->users_dbus_interface, NULL);

  /* Build all of the user switching items */
  if (can_activate == TRUE)
  {
    
    if (check_new_session ()){
      switch_menuitem = dbusmenu_menuitem_new ();
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
                        self->users_dbus_interface);
    }    
    
    if (check_guest_session ())
    {
      guest_mi = dbusmenu_menuitem_new ();
      dbusmenu_menuitem_property_set (guest_mi,
                                      DBUSMENU_MENUITEM_PROP_TYPE,
                                      USER_ITEM_TYPE);
      dbusmenu_menuitem_property_set (guest_mi,
                                      USER_ITEM_PROP_NAME,
                                      _("Guest Session"));
      dbusmenu_menuitem_property_set_bool (guest_mi,
                                           USER_ITEM_PROP_LOGGED_IN,
                                           FALSE);
      dbusmenu_menuitem_child_append (self->root_item, guest_mi);
      g_signal_connect (G_OBJECT (guest_mi),
                        DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK (activate_guest_session),
                        self);
      users_service_dbus_set_guest_item (self->users_dbus_interface,
                                         guest_mi);
    }
    
    GList * users = NULL;
    users = users_service_dbus_get_user_list (self->users_dbus_interface);
    self->user_count = g_list_length(users);
    
    gboolean user_menu_is_visible = FALSE;
    
    if (!greeter_mode){
      user_menu_is_visible = self->user_count > 1;
    }
    
    session_dbus_set_user_menu_visibility (self->session_dbus_interface,
                                           user_menu_is_visible);

    if (self->user_count > MINIMUM_USERS && self->user_count < MAXIMUM_USERS) {
      users = g_list_sort (users, (GCompareFunc)compare_users_by_username);
    }

    for (u = users; u != NULL; u = g_list_next (u)) {
      user = u->data;
      g_debug ("%p: %s", user, user->real_name);      
      user->service = self->users_dbus_interface;

      if (g_strcmp0(user->user_name, "guest") == 0) {
        /* Check to see if the guest has sessions and so therefore should
           get a check mark. */
        if (user->sessions != NULL) {
          dbusmenu_menuitem_property_set_bool (guest_mi,
                                               USER_ITEM_PROP_LOGGED_IN,
                                               TRUE);
        }
        /* If we're showing user accounts, keep going through the list */
        if (self->user_count > MINIMUM_USERS && self->user_count < MAXIMUM_USERS) {
          continue;
        }
        /* If not, we can stop here */
        break;
      }

      if (self->user_count > MINIMUM_USERS && self->user_count < MAXIMUM_USERS) {
        mi = dbusmenu_menuitem_new ();
        dbusmenu_menuitem_property_set (mi,
                                        DBUSMENU_MENUITEM_PROP_TYPE,
                                        USER_ITEM_TYPE);
        if (user->real_name_conflict) {
          gchar * conflictedname = g_strdup_printf("%s (%s)", user->real_name, user->user_name);
          dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, conflictedname);
          g_free(conflictedname);
        } else {
          g_debug ("%p: %s", user, user->real_name);                
          dbusmenu_menuitem_property_set (mi,
                                          USER_ITEM_PROP_NAME,
                                          user->real_name);
        }
        dbusmenu_menuitem_property_set_bool (mi,
                                             USER_ITEM_PROP_LOGGED_IN,
                                             user->sessions != NULL);
        if (user->icon_file != NULL && user->icon_file[0] != '\0') {
          g_debug ("user %s has this icon : %s",
                    user->user_name,
                    user->icon_file);
          dbusmenu_menuitem_property_set (mi,
                                          USER_ITEM_PROP_ICON,
                                          user->icon_file);
        } else {
          dbusmenu_menuitem_property_set (mi,
                                          USER_ITEM_PROP_ICON,
                                          USER_ITEM_ICON_DEFAULT);
        }
        
        gboolean logged_in = g_strcmp0 (user->user_name, g_get_user_name()) == 0;       
        dbusmenu_menuitem_property_set_bool (mi,
                                             USER_ITEM_PROP_IS_CURRENT_USER,
                                             logged_in);          
        if (logged_in == TRUE){
          g_debug ("about to set the users real name to %s for user %s",
                    user->real_name, user->user_name);
          session_dbus_set_users_real_name (self->session_dbus_interface, user->real_name);
        }
        
        dbusmenu_menuitem_child_append (self->root_item, mi);
        g_signal_connect (G_OBJECT (mi),
                          DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                          G_CALLBACK (activate_user_session),
                          user);
        user->menuitem = mi;
      }
    }
    g_list_free(users);
  }
  // Add the online accounts and separator
  DbusmenuMenuitem * separator1 = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (separator1,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_SEPARATOR);
  dbusmenu_menuitem_child_append (self->root_item, separator1);
  DbusmenuMenuitem * online_accounts_item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (online_accounts_item,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_DEFAULT);
  dbusmenu_menuitem_property_set (online_accounts_item,
                                  DBUSMENU_MENUITEM_PROP_LABEL,
                                  _("Online Accounts…"));

  g_signal_connect (G_OBJECT (online_accounts_item),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (activate_online_accounts),
                    NULL);
                                  
  dbusmenu_menuitem_child_append (self->root_item, online_accounts_item);

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

/* Checks to see if we can create sessions */
// TODO what is this ?
static gboolean
check_new_session ()
{
	return TRUE;
}

/* Check to see if the lockdown key is protecting from
   locking the screen.  If not, lock it. */
static void
lock_if_possible (void) {
	ensure_settings_client ();

	if (!g_settings_get_boolean (settings, LOCKDOWN_KEY_SCREENSAVER)) {
		lock_screen(NULL, 0, NULL);
	}

	return;
}


/* Starts a new generic session */
static void
activate_new_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
	lock_if_possible();

  users_service_dbus_show_greeter (USERS_SERVICE_DBUS(user_data));

	return;
}

/* Activates a session for a particular user. */
static void
activate_user_session (DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  UserData *user = (UserData *)user_data;
  UsersServiceDbus *service = user->service;

  lock_if_possible();

  users_service_dbus_activate_user_session (service, user);
}

/* Comparison function to look into the UserData struct
   to compare by using the username value */
static gint
compare_users_by_username (const gchar *a,
                           const gchar *b)
{
  UserData *user1 = (UserData *)a;
  UserData *user2 = (UserData *)b;

  gint retval = g_strcmp0 (user1->real_name, user2->real_name);

  /* If they're the same, they're both in conflict. */
  if (retval == 0) {
    user1->real_name_conflict = TRUE;
    user2->real_name_conflict = TRUE;
  }

  return retval;
}

// TODO
// Wait until dialog is complete to find out name to pass
// to the control centre.
static void
activate_online_accounts (DbusmenuMenuitem *mi,
                          guint timestamp,
                          gpointer user_data)
{
  GError * error = NULL;
  if (!g_spawn_command_line_async("gnome-control-center online-accounts", &error))
  {
    g_warning("Unable to show control centre: %s", error->message);
    g_error_free(error);
  }
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

/* Signal called when a user is added.  It updates the count and
   rebuilds the menu */
static void
user_change (UsersServiceDbus *service,
             const gchar      *user_id,
             gpointer          user_data)
{
  g_return_if_fail (USER_IS_MENU_MGR (user_data));  
  UserMenuMgr* user_mgr = USER_MENU_MGR(user_data);  
	user_menu_mgr_rebuild_items (user_mgr, FALSE);
	return;
}

/* Ensures that we have a GConf client and if we build one
   set up the signal handler. */
static void
ensure_settings_client ()
{
	if(!settings) {
		settings = g_settings_new (LOCKDOWN_SCHEMA);
	}
	return;
}

DbusmenuMenuitem*
user_mgr_get_root_item (UserMenuMgr* self)
{
  return self->root_item;
}

/* Checks to see if we should show the guest suession item */
static gboolean
check_guest_session (void)
{
	if (geteuid() < 500) {
		/* System users shouldn't have guest account shown.  Mosly
		   this would be the case of the guest user itself. */
		return FALSE;
	}

	return TRUE;
}

/* Called when someone clicks on the guest session item. */
static void
activate_guest_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
  g_return_if_fail (USER_IS_MENU_MGR (user_data));  
  UserMenuMgr* user_mgr = USER_MENU_MGR(user_data);  
  UsersServiceDbus *service = user_mgr->users_dbus_interface;

	lock_if_possible();
  
  if (users_service_dbus_activate_guest_session(service)) {
    return;
  }
}


/*
 * Clean Entry Point 
 */
UserMenuMgr* user_menu_mgr_new (SessionDbus* session_dbus, gboolean greeter_mode)
{
  UserMenuMgr* user_mgr = g_object_new (USER_TYPE_MENU_MGR, NULL);
  user_mgr->session_dbus_interface = session_dbus;
  user_menu_mgr_rebuild_items (user_mgr, greeter_mode);    
  return user_mgr;
}
  

