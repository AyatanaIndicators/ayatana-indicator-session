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

#include <glib.h>

#include "dbus-accounts.h"
#include "dbus-display-manager.h"
#include "dbus-user.h"
#include "dbus-consolekit-seat.h"
#include "dbus-consolekit-manager.h"
#include "dbus-consolekit-session.h"

#include "guest.h"

struct _IndicatorSessionGuestDbusPriv
{
  GCancellable * cancellable;

  Accounts * accounts;
  AccountsUser * guest;
  DisplayManagerSeat * display_manager_seat;

  ConsoleKitSeat * seat;
  ConsoleKitSession * active_session;
  guint active_uid;

  gboolean guest_is_active;
  gboolean guest_is_allowed;
};

typedef IndicatorSessionGuestDbusPriv priv_t;

G_DEFINE_TYPE (IndicatorSessionGuestDbus,
               indicator_session_guest_dbus,
               INDICATOR_TYPE_SESSION_GUEST)

/***
****
***/

static void
check_for_active_guest (IndicatorSessionGuestDbus * self)
{
  gboolean guest_is_active;
  priv_t * p = self->priv;

  guest_is_active = (p->active_uid)
                 && (p->guest != NULL)
                 && (p->active_uid == accounts_user_get_uid (p->guest));

  if (p->guest_is_active != guest_is_active)
    {
      p->guest_is_active = guest_is_active;

      indicator_session_guest_notify_active (INDICATOR_SESSION_GUEST(self));
    }
}

static void
set_active_uid (IndicatorSessionGuestDbus * self, guint uid)
{
  self->priv->active_uid = uid;

  check_for_active_guest (self);
}

static void
on_active_uid_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  guint uid;
  GError * err;
  IndicatorSessionGuestDbus * self;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  uid = 0;
  err = NULL;
  self = INDICATOR_SESSION_GUEST_DBUS (gself);
  console_kit_session_call_get_unix_user_finish (self->priv->active_session, &uid, res, &err);

  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else
    {
      set_active_uid (self, uid);
    }
}


static void
set_active_session (IndicatorSessionGuestDbus * self,
                    ConsoleKitSession         * session)
{
  priv_t * p = self->priv;

  if (p->active_session != NULL)
    {
      g_debug ("%s %s active_session refcount is %d before we unref", G_STRLOC, G_STRFUNC, G_OBJECT(self->priv->active_session)->ref_count);

      g_clear_object (&p->active_session);
    }

  if (session != NULL)
    {
      p->active_session = g_object_ref (session);

      console_kit_session_call_get_unix_user (session,
                                              p->cancellable,
                                              on_active_uid_ready,
                                              self);
    }
}

static void
on_active_session_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  ConsoleKitSession * session;

  err = NULL;
  session = console_kit_session_proxy_new_finish (res, &err);

  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
    }
  else
    {
      set_active_session (gself, session);
    }

  g_clear_object (&session);
}


static void
on_active_session_changed (ConsoleKitSeat            * seat  G_GNUC_UNUSED,
                           const gchar               * ssid,
                           IndicatorSessionGuestDbus * self)
{
  console_kit_session_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                         G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                                         "org.freedesktop.ConsoleKit",
                                         ssid,
                                         self->priv->cancellable,
                                         on_active_session_proxy_ready,
                                         self);
}

static void
set_seat (IndicatorSessionGuestDbus * self,
          ConsoleKitSeat            * seat)
{
  priv_t * p = self->priv;

  if (p->seat != NULL)
    {
g_debug ("%s %s guest-dbus disconnecting from %p", G_STRLOC, G_STRFUNC, (void*)p->seat);
      g_signal_handlers_disconnect_by_data (p->seat, self);
g_debug ("%s %s seat refcount is %d before our unref", G_STRLOC, G_STRFUNC, G_OBJECT(p->seat)->ref_count);

      g_clear_object (&p->seat);
    }

  if (seat != NULL)
    {
      p->seat = g_object_ref (seat);
g_debug ("%s %s guest-dbus connecting to %p", G_STRLOC, G_STRFUNC, (void*)p->seat);

      g_signal_connect (seat, "active-session-changed",
                        G_CALLBACK(on_active_session_changed), self);
    }
}

/***
****
***/

static void
set_guest (IndicatorSessionGuestDbus * self,
           AccountsUser              * guest)
{
  priv_t * p = self->priv;

  if (p->guest != NULL)
    {
      g_debug ("%s %s guest refcount is %d before we unref", G_STRLOC, G_STRFUNC, G_OBJECT(p->guest)->ref_count);

      g_clear_object (&p->guest);
    }

  if (guest != NULL)
    {
      p->guest = g_object_ref (guest);
    }

  g_debug ("%s %s guest proxy is now %p", G_STRLOC, G_STRFUNC, (void*)guest);
  indicator_session_guest_notify_logged_in (INDICATOR_SESSION_GUEST(self));

  check_for_active_guest (self);
}

static void
on_user_deleted (IndicatorSessionGuestDbus * self,
                 const gchar               * path)
{
  AccountsUser * guest = self->priv->guest;
  g_debug ("%s %s %s", G_STRLOC, G_STRFUNC, path);

  if (guest != NULL)
    if (!g_strcmp0 (path, g_dbus_proxy_get_object_path (G_DBUS_PROXY(guest))))
      set_guest (self, NULL);
}

static gboolean
is_guest (AccountsUser * user)
{
  /* a guest will look like this:
     username:[guest-jjbEVV] realname:[Guest] system:[1] */
  return IS_ACCOUNTS_USER(user)
      && accounts_user_get_system_account (user)
      && !g_ascii_strcasecmp (accounts_user_get_real_name(user), "Guest");
}

static void
on_user_proxy_ready (GObject       * o     G_GNUC_UNUSED,
                     GAsyncResult  * res,
                     gpointer        self)
{
  GError * err;
  AccountsUser * user;

  err = NULL;
  user = accounts_user_proxy_new_for_bus_finish (res, &err);

  if (err != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, err->message);
      g_error_free (err);
    }
  else if (is_guest (user))
    {
      g_debug ("%s %s got guest", G_STRLOC, G_STRFUNC);
      set_guest (INDICATOR_SESSION_GUEST_DBUS(self), user);
    }

  g_clear_object (&user);
}

static void
create_user_proxy_for_path (IndicatorSessionGuestDbus * self,
                            const char                * path)
{
  const char * name = "org.freedesktop.Accounts";
  const GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES;
  g_debug ("%s %s creating proxy for %s", G_STRLOC, G_STRFUNC, path);

  accounts_user_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                   flags, name, path,
                                   self->priv->cancellable,
                                   on_user_proxy_ready, self);
}

static void
on_user_list_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  GError * err;
  gchar ** paths;

  err = NULL;
  paths = NULL;
  accounts_call_list_cached_users_finish (ACCOUNTS(o), &paths, res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else
    {
      int i;

      for (i=0; paths && paths[i]; ++i)
        create_user_proxy_for_path (gself, paths[i]);

      g_strfreev (paths);
    }
}

static void
set_account_manager (IndicatorSessionGuestDbus * self,
                     Accounts                  * a)
{
  g_debug ("%s %s setting account manager to %p", G_STRLOC, G_STRFUNC, (void*)a);

  if (self->priv->accounts != NULL)
    {
g_debug ("%s %s guest-dbus disconnecting from %p", G_STRLOC, G_STRFUNC, (void*)self->priv->accounts);
      g_signal_handlers_disconnect_by_data (self->priv->accounts, self);
g_debug ("%s %s account manager refcount is %d before our unref", G_STRLOC, G_STRFUNC, G_OBJECT(self->priv->accounts)->ref_count);
      g_clear_object (&self->priv->accounts);
    }

  if (a != NULL)
    {
      self->priv->accounts = g_object_ref (a);

g_debug ("%s %s guest-dbus connecting to %p", G_STRLOC, G_STRFUNC, (void*)self->priv->accounts);
      g_signal_connect_swapped (a, "user-added",
                                G_CALLBACK(create_user_proxy_for_path), self);

      g_signal_connect_swapped (a, "user-deleted",
                                G_CALLBACK(on_user_deleted), self);

      accounts_call_list_cached_users (a,
                                       self->priv->cancellable,
                                       on_user_list_ready,
                                       self);
    }
}

static void
set_guest_is_allowed (IndicatorSessionGuestDbus * self, gboolean guest_is_allowed)
{
  priv_t * p = self->priv;
  g_debug ("%s %s guest_is_allowed: %d", G_STRLOC, G_STRFUNC, (int)guest_is_allowed);

  if (p->guest_is_allowed != guest_is_allowed)
    {
      p->guest_is_allowed = guest_is_allowed;

      indicator_session_guest_notify_allowed (INDICATOR_SESSION_GUEST (self));
    }
}

static void
on_notify_has_guest_account (GObject * seat, GParamSpec * pspec G_GNUC_UNUSED, gpointer gself)
{
  set_guest_is_allowed (INDICATOR_SESSION_GUEST_DBUS (gself),
                        display_manager_seat_get_has_guest_account (DISPLAY_MANAGER_SEAT(seat)));
}

static void
set_display_manager_seat (IndicatorSessionGuestDbus * self, DisplayManagerSeat * seat)
{
  priv_t * p = self->priv;

  if (p->display_manager_seat != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->display_manager_seat, self);
      g_debug ("%s %s before we unref, dm seat's refcount is %d", G_STRLOC, G_STRFUNC, G_OBJECT(p->display_manager_seat)->ref_count);
      g_clear_object (&p->display_manager_seat);
    }

  if (seat != NULL)
    {
      p->display_manager_seat = g_object_ref (seat);

      g_signal_connect (seat, "notify::has-guest-account", G_CALLBACK(on_notify_has_guest_account), self);

      on_notify_has_guest_account (G_OBJECT(seat), NULL, self);
    }
}

static void
on_switch_to_guest_done (GObject * o, GAsyncResult * res, gpointer unused G_GNUC_UNUSED)
{
  GError * err;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  err = NULL;
  display_manager_seat_call_switch_to_guest_finish (DISPLAY_MANAGER_SEAT(o), res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
}

/***
****  Virtual Functions
***/

static void
my_dispose (GObject * o)
{
  IndicatorSessionGuestDbus * self = INDICATOR_SESSION_GUEST_DBUS (o);

  if (self->priv->cancellable != NULL)
    {
      g_cancellable_cancel (self->priv->cancellable);
      g_clear_object (&self->priv->cancellable);
    }

  set_seat (self, NULL);
  set_active_session (self, NULL);
  set_account_manager (self, NULL);
  set_display_manager_seat (self, NULL);
  g_clear_object (&self->priv->guest);

  G_OBJECT_CLASS (indicator_session_guest_dbus_parent_class)->dispose (o);
}

static void
my_finalize (GObject * o)
{
  /*IndicatorSessionGuestDbus * u = INDICATOR_SESSION_GUEST_DBUS (o);*/

  G_OBJECT_CLASS (indicator_session_guest_dbus_parent_class)->finalize (o);
}

static gboolean
my_is_allowed (IndicatorSessionGuest * self)
{
  g_return_val_if_fail (INDICATOR_IS_SESSION_GUEST_DBUS(self), FALSE);

  return INDICATOR_SESSION_GUEST_DBUS(self)->priv->guest_is_allowed;
}

static gboolean
my_is_logged_in (IndicatorSessionGuest * self)
{
  g_return_val_if_fail (INDICATOR_IS_SESSION_GUEST_DBUS(self), FALSE);

  return INDICATOR_SESSION_GUEST_DBUS(self)->priv->guest != NULL;
}

static gboolean
my_is_active (IndicatorSessionGuest * self)
{
  g_return_val_if_fail (INDICATOR_IS_SESSION_GUEST_DBUS(self), FALSE);

  return INDICATOR_SESSION_GUEST_DBUS(self)->priv->guest_is_active;
}

static void
my_switch_to_guest (IndicatorSessionGuest * self)
{
  priv_t * p;
  g_debug ("%s %s", G_STRLOC, G_STRFUNC);

  g_return_if_fail (INDICATOR_IS_SESSION_GUEST_DBUS(self));

  p = INDICATOR_SESSION_GUEST_DBUS(self)->priv;

  if (p->display_manager_seat != NULL)
    {
      display_manager_seat_call_switch_to_guest (p->display_manager_seat,
                                                 "",
                                                 p->cancellable,
                                                 on_switch_to_guest_done,
                                                 self);
    }
}

/***
****  GObject Boilerplate
***/

static void
/* cppcheck-suppress unusedFunction */
indicator_session_guest_dbus_class_init (IndicatorSessionGuestDbusClass * klass)
{
  GObjectClass * object_class;
  IndicatorSessionGuestClass * guest_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = my_dispose;
  object_class->finalize = my_finalize;

  guest_class = INDICATOR_SESSION_GUEST_CLASS (klass);
  guest_class->is_allowed = my_is_allowed;
  guest_class->is_logged_in = my_is_logged_in;
  guest_class->is_active = my_is_active;
  guest_class->switch_to_guest = my_switch_to_guest;

  g_type_class_add_private (klass, sizeof (IndicatorSessionGuestDbusPriv));
}

static void
/* cppcheck-suppress unusedFunction */
indicator_session_guest_dbus_init (IndicatorSessionGuestDbus * self)
{
  priv_t * p;

  p = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                   INDICATOR_TYPE_SESSION_GUEST_DBUS,
                                   IndicatorSessionGuestDbusPriv);
  p->cancellable = g_cancellable_new ();
  self->priv = p;
}

/***
****  Public
***/

IndicatorSessionGuest *
indicator_session_guest_dbus_new (void)
{
  gpointer o = g_object_new (INDICATOR_TYPE_SESSION_GUEST_DBUS, NULL);

  return INDICATOR_SESSION_GUEST (o);
}

void
indicator_session_guest_dbus_set_proxies (IndicatorSessionGuestDbus * self,
                                          Accounts                  * accounts,
                                          DisplayManagerSeat        * dm_seat,
                                          ConsoleKitSeat            * seat,
                                          ConsoleKitSession         * session)
{
  g_return_if_fail (INDICATOR_IS_SESSION_GUEST_DBUS(self));
  g_debug ("%s %s accounts %p seat %p session %p", G_STRLOC, G_STRFUNC, (void*)accounts, (void*)seat, (void*)session);

  set_account_manager (self, accounts);
  set_display_manager_seat (self, dm_seat);
  set_seat (self, seat);
  set_active_session (self, session);
}
