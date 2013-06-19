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

#include "dbus-end-session-dialog.h"
#include "dbus-upower.h"
#include "dbus-webcredentials.h"
#include "gnome-screen-saver.h"
#include "gnome-session-manager.h"

#include "actions.h"

enum
{
  END_SESSION_TYPE_LOGOUT = 0,
  END_SESSION_TYPE_SHUTDOWN,
  END_SESSION_TYPE_REBOOT
};

struct _IndicatorSessionActionsDbusPriv
{
  GCancellable * cancellable;

  GSettings * lockdown_settings;
  UPower * upower;
  GnomeScreenSaver * screen_saver;
  GnomeSessionManager * session_manager;
  ConsoleKitManager * ck_manager;
  ConsoleKitSeat * ck_seat;
  DisplayManagerSeat * dm_seat;
  Webcredentials * webcredentials;
  EndSessionDialog * end_session_dialog;

  gboolean suspend_allowed;
  gboolean hibernate_allowed;
  gboolean seat_allows_activation;
};

typedef IndicatorSessionActionsDbusPriv priv_t;

G_DEFINE_TYPE (IndicatorSessionActionsDbus,
               indicator_session_actions_dbus,
               INDICATOR_TYPE_SESSION_ACTIONS)

/***
****
***/

static void
log_and_clear_error (GError ** err, const char * loc, const char * func)
{
  if (*err)
    {
      if (!g_error_matches (*err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        g_warning ("%s %s: %s", loc, func, (*err)->message);

      g_clear_error (err);
    }
}

static void
on_can_activate_sessions (GObject * o, GAsyncResult * res, gpointer gself)
{
  GError * err;
  gboolean can_activate_sessions;

  err = NULL;
  can_activate_sessions = FALSE;
  console_kit_seat_call_can_activate_sessions_finish (CONSOLE_KIT_SEAT(o),
                                                      &can_activate_sessions,
                                                      res,
                                                      &err);
  if (err == NULL)
    {
      priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv;
      p->seat_allows_activation = can_activate_sessions;
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
set_ck_seat (IndicatorSessionActionsDbus * self, ConsoleKitSeat * seat)
{
  priv_t * p = self->priv;

  g_clear_object (&p->ck_seat);

  if (seat != NULL)
    {
      p->ck_seat = g_object_ref (seat);

      console_kit_seat_call_can_activate_sessions (seat,
                                                   p->cancellable,
                                                   on_can_activate_sessions,
                                                   self);
    }
}

/***
****
***/

static void
set_dm_seat (IndicatorSessionActionsDbus * self, DisplayManagerSeat * seat)
{
  priv_t * p = self->priv;

  if (p->dm_seat != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->dm_seat, self);
      g_clear_object (&p->dm_seat);
    }

  if (seat != NULL)
    {
      p->dm_seat = g_object_ref (seat);
      /*g_signal_connect (seat, "notify::has-actions-account", G_CALLBACK(on_notify_has_actions_account), self);*/
    }
}

static void
on_screensaver_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  GnomeScreenSaver * ss;

  err = NULL;
  ss = gnome_screen_saver_proxy_new_for_bus_finish (res, &err);
  if (err == NULL)
    {
      INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv->screen_saver = ss;
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_suspend_allowed_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  GError * err;
  gboolean allowed = FALSE;

  err = NULL;
  upower_call_suspend_allowed_finish (UPOWER(o), &allowed, res, &err);
  if (err == NULL)
    {
      priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv;

      if (p->suspend_allowed != allowed)
        {
          p->suspend_allowed = allowed;
          indicator_session_actions_notify_can_suspend (gself);
        }
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_hibernate_allowed_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  GError * err;
  gboolean allowed = FALSE;

  err = NULL;
  upower_call_hibernate_allowed_finish (UPOWER(o), &allowed, res, &err);
  if (err == NULL)
    {
      priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv;

      if (p->hibernate_allowed != allowed)
        {
          p->hibernate_allowed = allowed;
          indicator_session_actions_notify_can_hibernate (gself);
        }
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_upower_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  UPower * upower;

  err = NULL;
  upower = upower_proxy_new_for_bus_finish (res, &err);
  if (err == NULL)
    {
      priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv;

      p->upower = upower;

      g_signal_connect_swapped (upower, "notify::can-suspend",
                                G_CALLBACK(indicator_session_actions_notify_can_suspend), gself);

      g_signal_connect_swapped (upower, "notify::can-hibernate",
                                G_CALLBACK(indicator_session_actions_notify_can_hibernate), gself);

      upower_call_suspend_allowed (upower, p->cancellable, on_suspend_allowed_ready, gself);

      upower_call_hibernate_allowed (upower, p->cancellable, on_hibernate_allowed_ready, gself);
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_session_manager_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  GnomeSessionManager * sm;

  err = NULL;
  sm = gnome_session_manager_proxy_new_for_bus_finish (res, &err);
  if (err == NULL)
    {
      INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv->session_manager = sm;
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_webcredentials_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  Webcredentials * webcredentials;

  err = NULL;
  webcredentials = webcredentials_proxy_new_for_bus_finish (res, &err);
  if (err == NULL)
    {
      INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv->webcredentials = webcredentials;

      g_signal_connect_swapped (webcredentials, "notify::error-status",
                                G_CALLBACK(indicator_session_actions_notify_has_online_account_error), gself);
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_end_session_dialog_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  EndSessionDialog * end_session_dialog;

  err = NULL;
  end_session_dialog = end_session_dialog_proxy_new_for_bus_finish (res, &err);
  if (err == NULL)
    {
      INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv->end_session_dialog = end_session_dialog;

      indicator_session_actions_notify_can_prompt (INDICATOR_SESSION_ACTIONS(gself));
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

/***
****  Virtual Functions
***/

static gboolean
my_can_lock (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return !g_settings_get_boolean (p->lockdown_settings, "disable-lock-screen");
}

static gboolean
my_can_logout (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return !g_settings_get_boolean (p->lockdown_settings, "disable-log-out");
}

static gboolean
my_can_switch (IndicatorSessionActions * self)
{
  const priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return p->seat_allows_activation
     && !g_settings_get_boolean (p->lockdown_settings, "disable-user-switching");
}

static gboolean
my_can_suspend (IndicatorSessionActions * self)
{
  const priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return p && p->upower && p->suspend_allowed && upower_get_can_suspend (p->upower);
}

static gboolean
my_can_hibernate (IndicatorSessionActions * self)
{
  const priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return p && p->upower && p->hibernate_allowed && upower_get_can_hibernate (p->upower);
}

static gboolean
my_can_prompt (IndicatorSessionActions * self)
{
  const priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return (p != NULL)
      && (p->end_session_dialog != NULL)
      && (g_dbus_proxy_get_name_owner (G_DBUS_PROXY(p->end_session_dialog)) != NULL);
}

static gboolean
my_has_online_account_error (IndicatorSessionActions * self)
{
  const priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return p && (p->webcredentials) && (webcredentials_get_error_status (p->webcredentials));
}

static void
my_suspend (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->upower != NULL);

  upower_call_suspend (p->upower, p->cancellable, NULL, NULL);
}

static void
my_hibernate (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->upower != NULL);

  upower_call_hibernate (p->upower, p->cancellable, NULL, NULL);
}

/***
****  End Session Dialog
***/

static void
logout_now (IndicatorSessionActions * self, gboolean try_to_prompt)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;
  const int type = try_to_prompt ? 0 : 1;

  g_return_if_fail (p->session_manager != NULL);

  gnome_session_manager_call_logout (p->session_manager,
                                     type,
                                     p->cancellable,
                                     NULL,
                                     NULL);
}

static void
logout_now_with_prompt (IndicatorSessionActions * self)
{
  logout_now (self, TRUE);
}

static void
logout_now_quietly (IndicatorSessionActions * self)
{
  logout_now (self, FALSE);
}

static void
restart_now (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->ck_manager != NULL);

  console_kit_manager_call_restart (p->ck_manager, p->cancellable, NULL, NULL);
}

static void
shutdown_now (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->ck_manager != NULL);

  console_kit_manager_call_stop (p->ck_manager, p->cancellable, NULL, NULL);
}

static void
stop_listening_to_dialog (IndicatorSessionActionsDbus * self)
{
  g_signal_handlers_disconnect_by_data (self->priv->end_session_dialog, self);
}
static void
on_end_session_dialog_canceled (IndicatorSessionActionsDbus * self)
{
  stop_listening_to_dialog (self);
}
static void
on_end_session_dialog_closed (IndicatorSessionActionsDbus * self)
{
  stop_listening_to_dialog (self);
}

static void
on_open_end_session_dialog_ready (GObject      * o,
                                  GAsyncResult * res,
                                  gpointer       gself G_GNUC_UNUSED)
{
  GError * err = NULL;
  end_session_dialog_call_open_finish (END_SESSION_DIALOG(o), res, &err);
  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
show_end_session_dialog (IndicatorSessionActionsDbus * self, int type)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;
  gpointer o = p->end_session_dialog;
  const char * inhibitor_paths[]  = { NULL };

  g_assert (o != NULL);

  g_signal_connect_swapped (o, "confirmed-logout", G_CALLBACK(logout_now_quietly), self);
  g_signal_connect_swapped (o, "confirmed-reboot", G_CALLBACK(restart_now), self);
  g_signal_connect_swapped (o, "confirmed-shutdown", G_CALLBACK(shutdown_now), self);
  g_signal_connect_swapped (o, "canceled", G_CALLBACK(on_end_session_dialog_canceled), self);
  g_signal_connect_swapped (o, "closed", G_CALLBACK(on_end_session_dialog_closed), self);

  end_session_dialog_call_open (p->end_session_dialog, type, 0, 0, inhibitor_paths,
                                p->cancellable,
                                on_open_end_session_dialog_ready,
                                self);
}

static void
my_logout (IndicatorSessionActions * self)
{
  if (my_can_prompt (self))
    show_end_session_dialog (INDICATOR_SESSION_ACTIONS_DBUS(self), END_SESSION_TYPE_LOGOUT);
  else
    logout_now_with_prompt (self);
}


static void
my_restart (IndicatorSessionActions * self)
{
  if (my_can_prompt (self))
    show_end_session_dialog (INDICATOR_SESSION_ACTIONS_DBUS(self), END_SESSION_TYPE_REBOOT);
  else
    restart_now (self);
}

static void
my_shutdown (IndicatorSessionActions * self)
{
  /* NB: TYPE_REBOOT instead of TYPE_SHUTDOWN because
     the latter adds lock & logout options in Unity... */
  if (my_can_prompt (self))
    show_end_session_dialog (INDICATOR_SESSION_ACTIONS_DBUS(self), END_SESSION_TYPE_REBOOT);
  else
    shutdown_now (self);
}

/***
****
***/

static void
run_outside_app (const char * cmd)
{
  GError * err = NULL;
  g_debug ("%s calling \"%s\"", G_STRFUNC, cmd);
  g_spawn_command_line_async (cmd, &err);
  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
my_help (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  run_outside_app ("yelp");
}

static void
my_settings (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  run_outside_app ("gnome-control-center");
}

static void
my_about (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  run_outside_app ("gnome-control-center info");
}

/***
****
***/

static void
my_switch_to_screensaver (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->screen_saver != NULL);

  gnome_screen_saver_call_lock (p->screen_saver, p->cancellable, NULL, NULL);
}

static void
my_switch_to_greeter (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->dm_seat != NULL);

  display_manager_seat_call_switch_to_greeter (p->dm_seat, p->cancellable,
                                               NULL, NULL);
}

static void
my_switch_to_guest (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->dm_seat != NULL);

  display_manager_seat_call_switch_to_guest (p->dm_seat, "",
                                             p->cancellable,
                                             NULL, NULL);
}

static void
my_switch_to_username (IndicatorSessionActions * self, const char * username)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->dm_seat != NULL);

  display_manager_seat_call_switch_to_user (p->dm_seat, username, "",
                                            p->cancellable,
                                            NULL, NULL);
}

static void
my_dispose (GObject * o)
{
  IndicatorSessionActionsDbus * self = INDICATOR_SESSION_ACTIONS_DBUS (o);
  priv_t * p = self->priv;

  if (p->cancellable != NULL)
    {
      g_cancellable_cancel (p->cancellable);
      g_clear_object (&p->cancellable);
    }

  g_clear_object (&p->lockdown_settings);
  g_clear_object (&p->ck_manager);
  g_clear_object (&p->upower);
  g_clear_object (&p->screen_saver);
  g_clear_object (&p->session_manager);
  g_clear_object (&p->webcredentials);
  g_clear_object (&p->end_session_dialog);
  set_dm_seat (self, NULL);
  set_ck_seat (self, NULL);

  G_OBJECT_CLASS (indicator_session_actions_dbus_parent_class)->dispose (o);
}

/***
****  GObject Boilerplate
***/

static void
/* cppcheck-suppress unusedFunction */
indicator_session_actions_dbus_class_init (IndicatorSessionActionsDbusClass * klass)
{
  GObjectClass * object_class;
  IndicatorSessionActionsClass * actions_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = my_dispose;

  actions_class = INDICATOR_SESSION_ACTIONS_CLASS (klass);
  actions_class->can_lock = my_can_lock;
  actions_class->can_logout = my_can_logout;
  actions_class->can_switch = my_can_switch;
  actions_class->can_suspend = my_can_suspend;
  actions_class->can_hibernate = my_can_hibernate;
  actions_class->can_prompt = my_can_prompt;
  actions_class->has_online_account_error = my_has_online_account_error;
  actions_class->logout = my_logout;
  actions_class->suspend = my_suspend;
  actions_class->hibernate = my_hibernate;
  actions_class->restart = my_restart;
  actions_class->shutdown = my_shutdown;
  actions_class->settings = my_settings;
  actions_class->help = my_help;
  actions_class->about = my_about;
  actions_class->switch_to_screensaver = my_switch_to_screensaver;
  actions_class->switch_to_greeter = my_switch_to_greeter;
  actions_class->switch_to_guest = my_switch_to_guest;
  actions_class->switch_to_username = my_switch_to_username;

  g_type_class_add_private (klass, sizeof (IndicatorSessionActionsDbusPriv));
}

static void
/* cppcheck-suppress unusedFunction */
indicator_session_actions_dbus_init (IndicatorSessionActionsDbus * self)
{
  priv_t * p;
  GSettings * s;

  p = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                   INDICATOR_TYPE_SESSION_ACTIONS_DBUS,
                                   IndicatorSessionActionsDbusPriv);
  p->cancellable = g_cancellable_new ();
  p->seat_allows_activation = TRUE;
  self->priv = p;

  s = g_settings_new ("org.gnome.desktop.lockdown");
  g_signal_connect_swapped (s, "changed::disable-lock-screen",
                            G_CALLBACK(indicator_session_actions_notify_can_lock), self);
  g_signal_connect_swapped (s, "changed::disable-log-out",
                            G_CALLBACK(indicator_session_actions_notify_can_logout), self);
  g_signal_connect_swapped (s, "changed::disable-user-switching",
                            G_CALLBACK(indicator_session_actions_notify_can_switch), self);
  p->lockdown_settings = s;

  gnome_screen_saver_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        "org.gnome.ScreenSaver",
                                        "/org/gnome/ScreenSaver",
                                        p->cancellable,
                                        on_screensaver_proxy_ready,
                                        self);

  upower_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                            G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                            "org.freedesktop.UPower",
                            "/org/freedesktop/UPower",
                             p->cancellable,
                             on_upower_proxy_ready,
                             self);

  gnome_session_manager_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                           G_DBUS_PROXY_FLAGS_NONE,
                                           "org.gnome.SessionManager",
                                           "/org/gnome/SessionManager",
                                           p->cancellable,
                                           on_session_manager_proxy_ready,
                                           self);

  webcredentials_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                    "com.canonical.indicators.webcredentials",
                                    "/com/canonical/indicators/webcredentials",
                                    p->cancellable,
                                    on_webcredentials_proxy_ready,
                                    self);

  end_session_dialog_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                        "com.canonical.Unity",
                                        "/org/gnome/SessionManager/EndSessionDialog",
                                        p->cancellable,
                                        on_end_session_dialog_proxy_ready,
                                        self);
}

/***
****  Public
***/

IndicatorSessionActions *
indicator_session_actions_dbus_new (void)
{
  gpointer o = g_object_new (INDICATOR_TYPE_SESSION_ACTIONS_DBUS, NULL);

  return INDICATOR_SESSION_ACTIONS (o);
}

void
indicator_session_actions_dbus_set_proxies (IndicatorSessionActionsDbus * self,
                                            ConsoleKitManager           * ck_manager,
                                            DisplayManagerSeat          * dm_seat,
                                            ConsoleKitSeat              * ck_seat)
{
  g_return_if_fail (INDICATOR_IS_SESSION_ACTIONS_DBUS(self));

  self->priv->ck_manager = g_object_ref (ck_manager);

  set_dm_seat (self, dm_seat);

  set_ck_seat (self, ck_seat);
}
