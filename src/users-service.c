typedef struct _ActivateData ActivateData;
struct _ActivateData
{
  UsersServiceDbus *service;
  UserData *user;
};

static DBusGConnection   *session_bus = NULL;
static DBusGConnection   *system_bus = NULL;
static DBusGProxy        *bus_proxy = NULL;
static DBusGProxy        *gdm_proxy = NULL;
static DbusmenuMenuitem  *root_menuitem = NULL;
static GMainLoop         *mainloop = NULL;
static UsersServiceDbus  *dbus_interface = NULL;

static DbusmenuMenuitem  *lock_menuitem = NULL;

static gint   count;
static GList *users;

/* Respond to the signal of autologin changing to see if the
   setting for timed login changes. */
static void
gdm_settings_change (void)
{
	if (!will_lock_screen()) {
		dbusmenu_menuitem_property_set_bool(lock_menuitem, DBUSMENU_MENUITEM_PROP_SENSITIVE, FALSE);
	} else {
		dbusmenu_menuitem_property_set_bool(lock_menuitem, DBUSMENU_MENUITEM_PROP_SENSITIVE, TRUE);
	}

	return;
}

static gboolean
check_guest_session (void)
{
	if (geteuid() < 500) {
		/* System users shouldn't have guest account shown.  Mosly
		   this would be the case of the guest user itself. */
		return FALSE;
	}
	if (!g_file_test(GUEST_SESSION_LAUNCHER, G_FILE_TEST_IS_EXECUTABLE)) {
		/* It doesn't appear that the Guest session stuff is
		   installed.  So let's not use it then! */
		return FALSE;
	}

	return TRUE;
}

static void
activate_guest_session (DbusmenuMenuitem * mi, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async(GUEST_SESSION_LAUNCHER, &error)) {
		g_warning("Unable to start guest session: %s", error->message);
		g_error_free(error);
	}

	return;
}

static gboolean
check_new_session (void)
{
	if (system_bus == NULL) {
		system_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	}

	if (system_bus == NULL) {
		return FALSE;
	}

	if (gdm_proxy == NULL) {
		gdm_proxy = dbus_g_proxy_new_for_name(system_bus,
		                                      "org.gnome.DisplayManager",
		                                      "/org/gnome/DisplayManager/LocalDisplayFactory",
		                                      "org.gnome.DisplayManager.LocalDisplayFactory");
	}

	if (gdm_proxy == NULL) {
		return FALSE;
	}

	return TRUE;
}

static void
activate_new_session (DbusmenuMenuitem * mi, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async("gdmflexiserver --startnew", &error)) {
		g_warning("Unable to start guest session: %s", error->message);
		g_error_free(error);
	}

	return;
}

static void
activate_user_session (DbusmenuMenuitem *mi, gpointer user_data)
{
  UserData *user = (UserData *)user_data;
  UsersServiceDbus *service = user->service;

  users_service_dbus_activate_user_session (service, user);
}

static gint
compare_users_by_username (const gchar *a,
                           const gchar *b)
{
  UserData *user1 = (UserData *)a;
  UserData *user2 = (UserData *)b;

  return g_strcmp0 (user1->user_name, user2->user_name);
}

static void
rebuild_items (DbusmenuMenuitem *root,
               UsersServiceDbus *service)
{
  DbusmenuMenuitem *mi = NULL;
  GList *u;
  UserData *user;
  gboolean can_activate;
  GList *children;

  can_activate = users_service_dbus_can_activate_session (service);

  children = dbusmenu_menuitem_take_children (root);
  g_list_foreach (children, (GFunc)g_object_unref, NULL);
  g_list_free (children);

  lock_menuitem = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(lock_menuitem, DBUSMENU_MENUITEM_PROP_LABEL, _("Lock Screen"));
  g_signal_connect(G_OBJECT(lock_menuitem), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(lock_screen), NULL);
  dbusmenu_menuitem_child_append(root, lock_menuitem);
  if (!will_lock_screen()) {
    dbusmenu_menuitem_property_set_bool(lock_menuitem, DBUSMENU_MENUITEM_PROP_SENSITIVE, FALSE);
  } else {
    dbusmenu_menuitem_property_set_bool(lock_menuitem, DBUSMENU_MENUITEM_PROP_SENSITIVE, TRUE);
  }

  if (can_activate == TRUE)
    {
      if (check_guest_session ())
        {
          mi = dbusmenu_menuitem_new ();
          dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Guest Session"));
          dbusmenu_menuitem_child_append (root, mi);
          g_signal_connect (G_OBJECT (mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_guest_session), NULL);
        }

      if (count > MINIMUM_USERS && count < MAXIMUM_USERS)
        {
          if (users != NULL)
            {
              GList *l = NULL;

              for (l = users; l != NULL; l = l->next)
                {
                  users = g_list_delete_link (users, l);
                }

              users = NULL;
            }

          users = users_service_dbus_get_user_list (service);

          users = g_list_sort (users, (GCompareFunc)compare_users_by_username);

          for (u = users; u != NULL; u = g_list_next (u))
            {
              user = u->data;

              user->service = service;

              mi = dbusmenu_menuitem_new ();
              dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, user->real_name);
              dbusmenu_menuitem_child_append (root, mi);
              g_signal_connect (G_OBJECT (mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_user_session), user);
            }
        }

      if (check_new_session ())
        {
          mi = dbusmenu_menuitem_new ();
          dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Switch User..."));
          dbusmenu_menuitem_child_append (root, mi);
          g_signal_connect (G_OBJECT (mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_new_session), NULL);
        }
    }
}

static void
user_added (UsersServiceDbus *service,
            UserData         *user,
            gpointer          user_data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)user_data;

  count++;

  rebuild_items (root, service);
}

static void
user_removed (UsersServiceDbus *service,
              UserData         *user,
              gpointer          user_data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)user_data;

  count--;

  rebuild_items (root, service);
}

static void
create_items (DbusmenuMenuitem *root,
              UsersServiceDbus *service)
{
  g_return_if_fail (IS_USERS_SERVICE_DBUS (service));

  count = users_service_dbus_get_user_count (service);

  rebuild_items (root, service);
}

int
main (int argc, char ** argv)
{
	g_idle_add(lock_screen_setup, NULL);
	lock_screen_gdm_cb_set(gdm_settings_change);

    dbus_interface = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);

    root_menuitem = dbusmenu_menuitem_new ();
    g_debug ("Root ID: %d", dbusmenu_menuitem_get_id (root_menuitem));

    create_items (root_menuitem, dbus_interface);

    DbusmenuServer * server = dbusmenu_server_new(INDICATOR_USERS_DBUS_OBJECT);
    dbusmenu_server_set_root(server, root_menuitem);

    g_signal_connect (G_OBJECT (dbus_interface),
                      "user-added",
                      G_CALLBACK (user_added),
                      root_menuitem);
    g_signal_connect (G_OBJECT (dbus_interface),
                      "user-removed",
                      G_CALLBACK (user_removed),
                      root_menuitem);

}

