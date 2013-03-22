/*
 * Copyright 2013 Canonical Ltd.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"

/***
**** indicator_session_util_get_session_proxies()
***/

struct session_proxy_data
{
  ConsoleKitManager * ck_manager;
  Accounts * account_manager;
  DisplayManagerSeat * dm_seat;

  ConsoleKitSeat * current_seat;
  ConsoleKitSession * current_session;
  AccountsUser * active_user;

  GCancellable * cancellable;
  GError * error;
  int pending;

  indicator_session_util_session_proxies_func callback;
  gpointer user_data;
};

static void
session_proxy_data_free (struct session_proxy_data * data)
{
  g_clear_object (&data->ck_manager);
  g_clear_object (&data->account_manager);
  g_clear_object (&data->dm_seat);

  g_clear_object (&data->current_seat);
  g_clear_object (&data->current_session);
  g_clear_object (&data->active_user);

  g_clear_object (&data->cancellable);
  g_clear_error (&data->error);

  g_free (data);
}

static void
finish_callback (struct session_proxy_data * data)
{
  g_assert (data != NULL);
  g_debug ("%s %s: pending is %d", G_STRLOC, G_STRFUNC, (data->pending-1));

  if (!--data->pending)
    {
      data->callback (data->ck_manager,
                      data->account_manager,
                      data->dm_seat,
                      data->current_seat,
                      data->current_session,
                      data->active_user,
                      data->error,
                      data->user_data);

      session_proxy_data_free (data);
    }
}

static void
on_user_proxy_ready (GObject       * o       G_GNUC_UNUSED,
                     GAsyncResult  * res,
                     gpointer        gdata)
{
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  data->active_user = accounts_user_proxy_new_for_bus_finish (res,  &data->error);

  if (data->error != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }
  else
    {
      g_debug ("%s %s user proxy is %p", G_STRLOC, G_STRFUNC, (void*)data->active_user);
    }

  finish_callback (data);
}

static void
on_user_path_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gdata)
{
  char * path = NULL;
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  accounts_call_find_user_by_id_finish (data->account_manager, &path, res, &data->error);

  if (data->error != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }
  else if (path != NULL)
    {
      g_debug ("%s %s user path is %s", G_STRLOC, G_STRFUNC, path);
      ++data->pending;
      accounts_user_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                       G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                                       "org.freedesktop.Accounts",
                                       path,
                                       data->cancellable,
                                       on_user_proxy_ready,
                                       data);
    }

  finish_callback (data);
  g_free (path);
}

static void
on_uid_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gdata)
{
  guint uid = 0;
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  console_kit_session_call_get_unix_user_finish (data->current_session, &uid, res, &data->error);
  if (data->error != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }
  else if (uid)
    {
      g_debug ("%s %s uid is %u", G_STRLOC, G_STRFUNC, uid);
      ++data->pending;
      accounts_call_find_user_by_id (data->account_manager,
                                     uid,
                                     data->cancellable,
                                     on_user_path_ready,
                                     data);
    }

  finish_callback (data);
}

static void
on_seat_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gdata)
{
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  data->current_seat = console_kit_seat_proxy_new_for_bus_finish (res, &data->error);

  if (data->error != NULL)
    g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);

  finish_callback (data);
}

static void
on_sid_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gdata)
{
  char * sid = NULL;
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  console_kit_session_call_get_seat_id_finish (data->current_session, &sid, res, &data->error);

  if (data->error != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }
  else if (sid != NULL)
    {
      g_debug ("%s %s sid is %s", G_STRLOC, G_STRFUNC, sid);
      ++data->pending;
      console_kit_seat_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                          G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                                          "org.freedesktop.ConsoleKit",
                                          sid,
                                          data->cancellable,
                                          on_seat_proxy_ready,
                                          data);
    }

  finish_callback (data);
  g_free (sid);
}

static void
on_session_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gdata)
{
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  data->current_session = console_kit_session_proxy_new_finish (res, &data->error);
  if (data->error != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }
  else
    {
      ++data->pending;
      console_kit_session_call_get_seat_id (data->current_session,
                                            data->cancellable,
                                            on_sid_ready,
                                            data);

      ++data->pending;
      console_kit_session_call_get_unix_user (data->current_session,
                                              data->cancellable,
                                              on_uid_ready,
                                              data);
    }

  finish_callback (data);
}

static void
on_current_session_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gdata)
{
  char * ssid = NULL;
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  ssid = NULL;
  console_kit_manager_call_get_current_session_finish (data->ck_manager,
                                                       &ssid, res,
                                                       &data->error);
  if (data->error != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }
  else if (ssid)
    {
      g_debug ("%s %s ssid is %s", G_STRLOC, G_STRFUNC, ssid);
      data->pending++;
      console_kit_session_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                             G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                                             "org.freedesktop.ConsoleKit",
                                             ssid,
                                             data->cancellable,
                                             on_session_proxy_ready,
                                             data);

    }

  finish_callback (data);
  g_free (ssid);
}

static void
on_display_manager_seat_proxy_ready (GObject      * o         G_GNUC_UNUSED,
                                     GAsyncResult * res,
                                     gpointer       gdata)
{
  DisplayManagerSeat * seat;
  struct session_proxy_data * data = gdata;

  seat = display_manager_seat_proxy_new_for_bus_finish (res, &data->error);

  if (data->error != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }
  else if (seat != NULL)
    {
      data->dm_seat = g_object_ref (seat);
    }

  finish_callback (data);
  g_clear_object (&seat);
}

static void
on_console_kit_manager_proxy_ready (GObject      * o       G_GNUC_UNUSED,
                                    GAsyncResult * res,
                                    gpointer       gdata)
{
  ConsoleKitManager * mgr;
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  if (data->error == NULL)
    { 
      mgr = console_kit_manager_proxy_new_for_bus_finish (res, &data->error);
      g_debug ("%s %s mgr is %p, err is %p", G_STRLOC, G_STRFUNC, (void*)mgr, (void*)data->error);

      if (data->error != NULL)
        {
          g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
        }
      else
        {
          data->ck_manager = mgr;

          data->pending++;
          console_kit_manager_call_get_current_session (mgr,
                                                        data->cancellable,
                                                        on_current_session_ready,
                                                        data);

        }
    }

  finish_callback (data);
}

static void
on_accounts_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gdata)
{
  struct session_proxy_data * data = gdata;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  if (data->error == NULL)
    {
      data->account_manager = accounts_proxy_new_for_bus_finish (res, &data->error);

      if (data->error != NULL)
        g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, data->error->message);
    }

  finish_callback (data);
}

/**
 * Getting all the proxies we want is kind of a pain -- 
 * especially without blocking (ie, using _sync() funcs) -- 
 * so it's farmed out to this wrapper utility.
 *
 * 1. in this func, start getting the ConsoleKit and Accounts proxies
 * 2. when the accounts proxy is ready, stash it in data.account_manager
 * 3. when the ck manager proxy is ready, stash it in data.ck_manager and
 *    ask it for the current session's ssid
 * 4. when the ssid is ready, start getting a proxy for it
 * 5. when the session's proxy is ready, stash it in data.current_session
 *    and ask it for both the current seat's sid and the active user's uid
 * 6. When the current seat's sid is ready, start getting a proxy for it
 * 7. When the current seat's proxy is ready, stash it in data.current_seat
 * 8. when the active user's uid is ready, ask data.account_manager for the path
 * 9. when the user path is ready, start getting an Accounts.User proxy for it
 * 10. when the Accounts.User proxy is read, stash it in data.active_user
 *
 * When everything is done, or if there's an error, invoke the data.callback
 */
void
indicator_session_util_get_session_proxies (
                     indicator_session_util_session_proxies_func   func,
                     GCancellable                                * cancellable,
                     gpointer                                      user_data)
{
  struct session_proxy_data * data;

  data = g_new0 (struct session_proxy_data, 1);
  data->callback = func;
  data->user_data = user_data;
  data->cancellable = g_object_ref (cancellable);

  data->pending++;
  accounts_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                              G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                              "org.freedesktop.Accounts",
                              "/org/freedesktop/Accounts",
                              data->cancellable,
                              on_accounts_proxy_ready, data);

  data->pending++;
  console_kit_manager_proxy_new_for_bus (
                               G_BUS_TYPE_SYSTEM,
                               G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                               "org.freedesktop.ConsoleKit",
                               "/org/freedesktop/ConsoleKit/Manager",
                               data->cancellable,
                               on_console_kit_manager_proxy_ready, data);

  data->pending++;
  display_manager_seat_proxy_new_for_bus (
                               G_BUS_TYPE_SYSTEM,
                               G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                               "org.freedesktop.DisplayManager",
                               g_getenv ("XDG_SEAT_PATH"),
                               data->cancellable,
                               on_display_manager_seat_proxy_ready, data);

}
