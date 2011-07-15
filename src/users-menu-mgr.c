/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * user-menu-mgr.c
 * Copyright (C) Conor Curran 2011 <conor.curran@canonical.com>
 * 
 * user-menu-mgr.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * user-menu-mgr.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "user-menu-mgr.h"
#include "gconf-helper.h"
#include "users-service-dbus.h"

static GConfClient * gconf_client = NULL;
static DbusmenuMenuitem  *switch_menuitem = NULL;

G_DEFINE_TYPE (UserMenuMgr, user_menu_mgr, G_TYPE_OBJECT);

static void activate_new_session (DbusmenuMenuitem * mi,
                                  guint timestamp,
                                  gpointer user_data);
static void activate_user_session (DbusmenuMenuitem *mi,
                                   guint timestamp,
                                   gpointer user_data)
static gint compare_users_by_username (const gchar *a,
                                       const gchar *b);
static void activate_online_accounts (DbusmenuMenuitem *mi,
                                      guint timestamp,
                                      gpointer user_data);

static void
user_menu_mgr_init (UserMenuMgr *self)
{
  self->users_dbus_interface = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);
  self->root_item = dbusmenu_menuitem_new ();
  user_menu_mgr_rebuild_items (self);  
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
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = user_menu_mgr_finalize;
}

/* Ensures that we have a GConf client and if we build one
   set up the signal handler. */
static void
ensure_gconf_client ()
{
	if (!gconf_client) {
		gconf_client = gconf_client_get_default ();
		gconf_client_add_dir(gconf_client, LOCKDOWN_DIR, GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
		gconf_client_notify_add(gconf_client, LOCKDOWN_DIR, lockdown_changed, NULL, NULL, NULL);
		gconf_client_add_dir(gconf_client, KEYBINDING_DIR, GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
		gconf_client_notify_add(gconf_client, KEYBINDING_DIR, keybinding_changed, NULL, NULL, NULL);
	}
}

/* Builds up the menu for us */
static void 
user_menu_mgr_rebuild_items (UserMenuMgr *self)
{
  DbusmenuMenuitem *mi = NULL;
  DbusmenuMenuitem *guest_mi = NULL;
  GList *u;
  UserData *user;
  gboolean can_activate;
  GList *children;

  /* Make sure we have a valid GConf client, and build one
     if needed */
  ensure_gconf_client ();

  /* Check to see which menu items we're allowed to have */
  can_activate = users_service_dbus_can_activate_session (service) &&
      !gconf_client_get_bool (gconf_client, LOCKDOWN_KEY_USER, NULL);

  /* Remove the old menu items if that makes sense */
  children = dbusmenu_menuitem_take_children (root);
  g_list_foreach (children, (GFunc)g_object_unref, NULL);
  g_list_free (children);

  /* Set to NULL just incase we don't end up building one */
  users_service_dbus_set_guest_item(service, NULL);

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
      dbusmenu_menuitem_child_append (self->root, switch_menuitem);
      g_signal_connect (G_OBJECT (switch_menuitem),
                        DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK (activate_new_session),
                        service);
    }

    GList * users = NULL;
    users = users_service_dbus_get_user_list (service);
    self->user_count = g_list_length(users);
    
    // TODO !!!!!
    // g_debug ("USER COUNT = %i", user_count);
    // We only want to show this menu when we have more than one registered 
    // user
    // session_dbus_set_user_menu_visibility (session_dbus, user_count > 1);

    if (self->user_count > MINIMUM_USERS && self->user_count < MAXIMUM_USERS) {
      users = g_list_sort (users, (GCompareFunc)compare_users_by_username);
    }

    for (u = users; u != NULL; u = g_list_next (u)) {
      user = u->data;
      user->service = service;

      g_debug ("%i %s", (gint)user->uid, user->user_name);

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
        dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_TYPE, USER_ITEM_TYPE);
        if (user->real_name_conflict) {
          gchar * conflictedname = g_strdup_printf("%s (%s)", user->real_name, user->user_name);
          dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, conflictedname);
          g_free(conflictedname);
        } else {
          dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, user->real_name);
        }
        dbusmenu_menuitem_property_set_bool (mi,
                                             USER_ITEM_PROP_LOGGED_IN,
                                             user->sessions != NULL);
        if (user->icon_file != NULL && user->icon_file[0] != '\0') {
          dbusmenu_menuitem_property_set(mi, USER_ITEM_PROP_ICON, user->icon_file);
        } else {
          dbusmenu_menuitem_property_set(mi, USER_ITEM_PROP_ICON, USER_ITEM_ICON_DEFAULT);
        }
        
        gboolean logged_in = g_strcmp0 (user->user_name, g_get_user_name()) == 0;       
        dbusmenu_menuitem_property_set_bool (mi,
                                             USER_ITEM_PROP_IS_CURRENT_USER,
                                             logged_in);          
        // TODO
        // Figure where this lives.
        if (logged_in == TRUE){
          g_debug ("about to set the users real name to %s for user %s",
                    user->real_name, user->user_name);
          session_dbus_set_users_real_name (session_dbus, user->real_name);
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
                                  _("Online Accounts..."));

  g_signal_connect (G_OBJECT (online_accounts_item),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (activate_online_accounts),
                    NULL);
                                  
  dbusmenu_menuitem_child_append (self->root_item, online_accounts_item);    
}

/* Checks to see if we can create sessions */
// TODO what is this ?
static gboolean
check_new_session (void)
{
	return TRUE;
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

/* Signal called when a user is added.  It updates the count and
   rebuilds the menu */
static void
user_change (UsersServiceDbus *service,
             const gchar      *user_id,
             gpointer          user_data)
{
	DbusmenuMenuitem *root = (DbusmenuMenuitem *)user_data;
	rebuild_user_items (root, service);
	return;
}


