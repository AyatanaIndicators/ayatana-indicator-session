/*
 * Copyright 2013 Canonical Ltd.
 * Copyright 2023-2025 Robert Tari
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 *   Robert Tari <robert@tari.in>
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
#include <glib/gi18n.h>

#include <ayatana/common/utils.h>

#include "dbus-end-session-dialog.h"
#include "dbus-login1-manager.h"
#include "dbus-webcredentials.h"
#include "gnome-screen-saver.h"
#include "gnome-session-manager.h"
#include "lomiri-session.h"

#include "actions.h"

#include "../utils.h"

enum
{
  END_SESSION_TYPE_LOGOUT = 0,
  END_SESSION_TYPE_SHUTDOWN,
  END_SESSION_TYPE_REBOOT
};

struct _IndicatorSessionActionsDbusPrivate
{
  GCancellable * cancellable;

  GSettings * lockdown_settings;
  GSettings * indicator_settings;
  GnomeScreenSaver * screen_saver;
  GnomeSessionManager * session_manager;
  LomiriShellSession * lomiri_session;
  Login1Manager * login1_manager;
  GCancellable * login1_manager_cancellable;
  Login1Seat * login1_seat;
  DisplayManagerSeat * dm_seat;
  GCancellable * dm_seat_cancellable;
  Webcredentials * webcredentials;
  EndSessionDialog * end_session_dialog;
  gboolean can_suspend;
  gboolean can_hibernate;
  gboolean seat_allows_activation;
};

typedef IndicatorSessionActionsDbusPrivate priv_t;

G_DEFINE_TYPE_WITH_PRIVATE(IndicatorSessionActionsDbus, indicator_session_actions_dbus, INDICATOR_TYPE_SESSION_ACTIONS)

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


/***
****
***/

typedef enum
{
  PROMPT_NONE,
  PROMPT_WITH_UTILS,
  PROMPT_WITH_LOMIRI,
  PROMPT_WITH_MATE,
  PROMPT_WITH_BUDGIE,
  PROMPT_WITH_XFCE,
}
prompt_status_t;

static prompt_status_t
get_prompt_status (IndicatorSessionActionsDbus * self)
{
  prompt_status_t prompt = PROMPT_NONE;
  const priv_t * p = self->priv;

  if (!g_settings_get_boolean (p->indicator_settings, "suppress-logout-restart-shutdown"))
    {
      /* can we use the MATE prompt? */
      if ((prompt == PROMPT_NONE) && ayatana_common_utils_have_mate_program ("mate-session-save"))
          prompt = PROMPT_WITH_MATE;

        // can we use the BUDGIE (currently GNOME) prompt?
        if ((prompt == PROMPT_NONE) && ayatana_common_utils_have_budgie_program("gnome-session-quit"))
        {
            prompt = PROMPT_WITH_BUDGIE;
        }

      /* can we use the XFCE prompt? */
      if ((prompt == PROMPT_NONE) && ayatana_common_utils_have_xfce_program ("xfce4-session-logout"))
          prompt = PROMPT_WITH_XFCE;

      /* can we use Lomiri prompt? */
      if ((prompt == PROMPT_NONE) && p && p->end_session_dialog)
        {
          GDBusProxy * proxy = G_DBUS_PROXY (p->end_session_dialog);
          char * name = g_dbus_proxy_get_name_owner (proxy);
          if (name != NULL)
            prompt = PROMPT_WITH_LOMIRI;
          g_free (name);
        }

      if ((prompt == PROMPT_NONE) && p)
        prompt = PROMPT_WITH_UTILS;
    }

  return prompt;
}

/***
****
***/

static void
on_seat_notify_multi_session (IndicatorSessionActionsDbus * self)
{
  priv_t * p = self->priv;
  gboolean b;

  b = login1_seat_get_can_multi_session (p->login1_seat);

  if (p->seat_allows_activation != b)
    {
      p->seat_allows_activation = b;

      indicator_session_actions_notify_can_switch (INDICATOR_SESSION_ACTIONS(self));
    }
}

static void
set_login1_seat (IndicatorSessionActionsDbus * self, Login1Seat * seat)
{
  priv_t * p = self->priv;

  if (p->login1_seat != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->login1_seat, self);
      g_clear_object (&p->login1_seat);
    }

  if (seat != NULL)
    {
      p->login1_seat = g_object_ref (seat);

      g_signal_connect_swapped (seat, "notify::can-multi-session",
                                G_CALLBACK(on_seat_notify_multi_session), self);
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
      g_cancellable_cancel (p->dm_seat_cancellable);
      g_clear_object (&p->dm_seat_cancellable);
      g_clear_object (&p->dm_seat);
    }

  if (seat != NULL)
    {
      p->dm_seat = g_object_ref (seat);
      p->dm_seat_cancellable = g_cancellable_new ();
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
on_lomiri_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  LomiriShellSession * us;

  err = NULL;
  us = lomiri_shell_session_proxy_new_for_bus_finish (res, &err);
  if (err == NULL)
    {
      INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv->lomiri_session = us;
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_can_suspend_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  char * str;
  GError * err;

  str = NULL;
  err = NULL;
  login1_manager_call_can_suspend_finish (LOGIN1_MANAGER(o), &str, res, &err);
  if (err == NULL)
    {
      priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv;

      const gboolean b = !g_strcmp0 (str, "yes");

      if (p->can_suspend != b)
        {
          p->can_suspend = b;
          indicator_session_actions_notify_can_suspend (gself);
        }

      g_free (str);
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
on_can_hibernate_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  gchar * str;
  GError * err;

  str = NULL;
  err = NULL;
  login1_manager_call_can_hibernate_finish (LOGIN1_MANAGER(o), &str, res, &err);
  if (err == NULL)
    {
      priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(gself)->priv;

      const gboolean b = !g_strcmp0 (str, "yes");

      if (p->can_hibernate != b)
        {
          p->can_hibernate = b;
          indicator_session_actions_notify_can_hibernate (gself);
        }

      g_free (str);
    }

  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static void
set_login1_manager (IndicatorSessionActionsDbus * self,
                    Login1Manager               * login1_manager)
{
  priv_t * p = self->priv;

  if (p->login1_manager != NULL)
    {
      g_cancellable_cancel (p->login1_manager_cancellable);
      g_clear_object (&p->login1_manager_cancellable);
      g_clear_object (&p->login1_manager);
    }

  if (login1_manager != NULL)
    {
      p->login1_manager_cancellable = g_cancellable_new ();

      p->login1_manager = g_object_ref (login1_manager);

      login1_manager_call_can_suspend (p->login1_manager,
                                       p->login1_manager_cancellable,
                                       on_can_suspend_ready,
                                       self);

      login1_manager_call_can_hibernate (p->login1_manager,
                                         p->login1_manager_cancellable,
                                         on_can_hibernate_ready,
                                         self);
    }
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

      if (webcredentials_get_error_status (webcredentials))
        indicator_session_actions_notify_has_online_account_error (gself);
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

      g_signal_connect_swapped (end_session_dialog, "notify::g-name-owner",
                                G_CALLBACK(indicator_session_actions_notify_can_switch), gself);

      indicator_session_actions_notify_can_prompt (INDICATOR_SESSION_ACTIONS(gself));
      indicator_session_actions_notify_can_reboot (INDICATOR_SESSION_ACTIONS(gself));
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

  if (g_settings_get_boolean (p->indicator_settings, "suppress-logout-menuitem"))
    return FALSE;

  if (g_settings_get_boolean (p->lockdown_settings, "disable-log-out"))
    return FALSE;

  return TRUE;
}

static gboolean
my_can_reboot (IndicatorSessionActions * actions)
{
  IndicatorSessionActionsDbus * self = INDICATOR_SESSION_ACTIONS_DBUS(actions);
  priv_t * p = self->priv;

  if (g_settings_get_boolean (p->indicator_settings, "suppress-restart-menuitem"))
    return FALSE;

  if (g_settings_get_boolean (p->indicator_settings, "force-restart-menuitem"))
    return TRUE;

  /* Shutdown and Restart are the same dialog prompt in Unity,
     so disable the redundant 'Restart' menuitem in that mode */
  if (!g_settings_get_boolean (p->indicator_settings, "suppress-shutdown-menuitem"))
    if (ayatana_common_utils_is_unity() || ayatana_common_utils_is_lomiri())
      return FALSE;

  return TRUE;
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

  return p && p->can_suspend;
}

static gboolean
my_can_hibernate (IndicatorSessionActions * self)
{
  const priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  return p && p->can_hibernate;
}

static gboolean
my_can_prompt (IndicatorSessionActions * self)
{
  return get_prompt_status(INDICATOR_SESSION_ACTIONS_DBUS(self)) != PROMPT_NONE;
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

  g_return_if_fail (p->login1_manager != NULL);

  login1_manager_call_suspend (p->login1_manager,
                               FALSE,
                               p->login1_manager_cancellable,
                               NULL,
                               NULL);
}

static void
my_hibernate (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->login1_manager != NULL);

  login1_manager_call_hibernate (p->login1_manager,
                                 FALSE,
                                 p->login1_manager_cancellable,
                                 NULL,
                                 NULL);
}

/***
****  End Session Dialog
***/

static gboolean
is_owned_proxy (gpointer proxy)
{
  gboolean owned = FALSE;

  if ((proxy != NULL) && G_IS_DBUS_PROXY (proxy))
    {
      char * name_owner = g_dbus_proxy_get_name_owner (proxy);

      if (name_owner != NULL)
        {
          owned = TRUE;
          g_free (name_owner);
        }
    }

  return owned;
}

static void
on_gnome_logout_response (GObject * o,
                          GAsyncResult * res,
                          gpointer unused G_GNUC_UNUSED)
{
  GError * err = NULL;
  gnome_session_manager_call_logout_finish (GNOME_SESSION_MANAGER(o), res, &err);
  log_and_clear_error (&err, G_STRLOC, G_STRFUNC);
}

static gboolean
logout_now_gnome_session_manager (IndicatorSessionActionsDbus * self)
{
  gboolean logout_called = FALSE;
  priv_t * p = self->priv;

  if (is_owned_proxy (p->session_manager))
    {
      g_debug ("%s: calling gnome_session_manager_call_logout()", G_STRFUNC);
      gnome_session_manager_call_logout (p->session_manager,
                                         1, /* don't prompt */
                                         p->cancellable,
                                         on_gnome_logout_response,
                                         self);
      logout_called = TRUE;
    }

  return logout_called;
}

static void
on_lomiri_logout_response (GObject * o,
                           GAsyncResult * res,
                           gpointer gself)
{
  GError * error;

  error = NULL;
  lomiri_shell_session_call_request_logout_finish (LOMIRI_SHELL_SESSION(o), res, &error);

  if (error != NULL)
    {
      if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        {
          g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, error->message);
          logout_now_gnome_session_manager(gself);
        }

      g_clear_error (&error);
    }
}

static gboolean
logout_now_desktop (IndicatorSessionActionsDbus * self)
{
  priv_t * p = self->priv;
  gboolean called = FALSE;

  if (is_owned_proxy (p->lomiri_session))
    {
      called = TRUE;
      g_debug ("calling lomiri_shell_session_call_request_logout()");
      lomiri_shell_session_call_request_logout (p->lomiri_session,
                                          p->cancellable,
                                          on_lomiri_logout_response,
                                          self);
     }

  return called;
}

static void
logout_now (IndicatorSessionActionsDbus * self)
{
  if (!logout_now_desktop(self) && !logout_now_gnome_session_manager(self))
    {
      g_critical("%s can't logout: no Unity nor GNOME/MATE session proxy", G_STRFUNC);
    }
}

static void
on_reboot_response (GObject      * o,
                    GAsyncResult * res,
                    gpointer       unused G_GNUC_UNUSED)
{
  GError * err = NULL;
  login1_manager_call_reboot_finish (LOGIN1_MANAGER(o), res, &err);
  if (err != NULL)
    {
      g_warning ("Unable to reboot: %s", err->message);
      g_error_free (err);
    }
}

static void
reboot_now (IndicatorSessionActionsDbus * self)
{
  priv_t * p = self->priv;

  g_return_if_fail (p->login1_manager != NULL);

  login1_manager_call_reboot (p->login1_manager,
                              FALSE,
                              p->login1_manager_cancellable,
                              on_reboot_response,
                              NULL);
}

static void
power_off_now (IndicatorSessionActionsDbus * self)
{
  priv_t * p = self->priv;

  g_return_if_fail (p->login1_manager != NULL);

  login1_manager_call_power_off (p->login1_manager,
                                 FALSE,
                                 p->login1_manager_cancellable,
                                 NULL,
                                 NULL);
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
  if (err != NULL)
    {
      if (!g_error_matches (err, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        g_warning ("%s %s: %s", G_STRFUNC, G_STRLOC, err->message);

      /* Treat errors as user confirmation.
         Otherwise how will the user ever log out? */
      logout_now(INDICATOR_SESSION_ACTIONS_DBUS(gself));

      g_clear_error(&err);
    }
}

static void
show_lomiri_end_session_dialog (IndicatorSessionActionsDbus * self, int type)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;
  gpointer o = p->end_session_dialog;
  const char * inhibitor_paths[]  = { NULL };

  g_assert (o != NULL);

  g_signal_connect_swapped (o, "confirmed-logout", G_CALLBACK(logout_now), self);
  g_signal_connect_swapped (o, "confirmed-reboot", G_CALLBACK(reboot_now), self);
  g_signal_connect_swapped (o, "confirmed-shutdown", G_CALLBACK(power_off_now), self);
  g_signal_connect_swapped (o, "canceled", G_CALLBACK(on_end_session_dialog_canceled), self);
  g_signal_connect_swapped (o, "closed", G_CALLBACK(on_end_session_dialog_closed), self);

  end_session_dialog_call_open (p->end_session_dialog, type, 0, 0, inhibitor_paths,
                                p->cancellable,
                                on_open_end_session_dialog_ready,
                                self);
}

static void
my_bug (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  if (!ayatana_common_utils_open_url(get_distro_bts_url()))
    ayatana_common_utils_zenity_warning ("dialog-warning",
                    _("Warning"),
                    _("The operating system's bug tracker needs to be accessed with\na web browser.\n\nThe Ayatana Session Indicator could not find any web\nbrowser on your computer."));
}

static void
my_logout (IndicatorSessionActions * actions)
{
  IndicatorSessionActionsDbus * self = INDICATOR_SESSION_ACTIONS_DBUS (actions);

  switch (get_prompt_status (self))
    {
      case PROMPT_WITH_LOMIRI:
        show_lomiri_end_session_dialog (self, END_SESSION_TYPE_LOGOUT);
        break;

      case PROMPT_WITH_MATE:
        /* --logout-dialog presents Logout and (if available) Switch User options */
        ayatana_common_utils_execute_command ("mate-session-save --logout-dialog");
        break;
      case PROMPT_WITH_BUDGIE:
        ayatana_common_utils_execute_command("gnome-session-quit --logout");
        break;
      case PROMPT_WITH_XFCE:
        ayatana_common_utils_execute_command ("xfce4-session-logout");
        break;

      case PROMPT_NONE:
        logout_now (self);
        break;

      case PROMPT_WITH_UTILS:
        {
          const char * primary = _("Are you sure you want to close all programs and log out?");
          const char * secondary = _("Some software updates won't be applied until the computer next restarts.");
          char * text = g_strdup_printf ("<big><b>%s</b></big>\n \n%s", primary, secondary);

          gboolean confirmed = ayatana_common_utils_zenity_question ("system-log-out",
                                                                    _("Log Out"),
                                                                    text,
                                                                    _("Log Out"),
                                                                    _("Cancel"));
          g_free (text);

          if (confirmed)
            logout_now (self);
          break;
        }
    }
}

static void
my_reboot (IndicatorSessionActions * actions)
{
  IndicatorSessionActionsDbus * self = INDICATOR_SESSION_ACTIONS_DBUS (actions);

  switch (get_prompt_status (self))
    {
      case PROMPT_WITH_LOMIRI:
        show_lomiri_end_session_dialog (self, END_SESSION_TYPE_REBOOT);
        break;

      case PROMPT_WITH_MATE:
        /* --shutdown-dialog presents Restart, Shutdown and (if available) Suspend options */
        ayatana_common_utils_execute_command ("mate-session-save --shutdown-dialog");
        break;
      case PROMPT_WITH_BUDGIE:
        ayatana_common_utils_execute_command("gnome-session-quit --reboot");
        break;
      case PROMPT_WITH_XFCE:
        ayatana_common_utils_execute_command ("xfce4-session-logout");
        break;

      case PROMPT_NONE:
        reboot_now (self);
        break;

      case PROMPT_WITH_UTILS:
        if (ayatana_common_utils_zenity_question ("system-restart",
                                                _("Restart"),
                                                _("Are you sure you want to close all programs and restart the computer?"),
                                                _("Restart"),
                                                _("Cancel")))
        {
            reboot_now (self);
        }

        break;
    }
}

static void
my_power_off (IndicatorSessionActions * actions)
{
  IndicatorSessionActionsDbus * self = INDICATOR_SESSION_ACTIONS_DBUS (actions);

  switch (get_prompt_status (self))
    {
      case PROMPT_WITH_LOMIRI:
        /* NB: TYPE_REBOOT instead of TYPE_SHUTDOWN because
           the latter adds lock & logout options in Unity and Lomiri... */
        if (ayatana_common_utils_is_unity() || ayatana_common_utils_is_lomiri())
          show_lomiri_end_session_dialog (self, END_SESSION_TYPE_REBOOT);
        else
          show_lomiri_end_session_dialog (self, END_SESSION_TYPE_SHUTDOWN);
        break;

      case PROMPT_WITH_MATE:
        /* --shutdown-dialog presents Restart, Shutdown and (if available) Suspend options */
        ayatana_common_utils_execute_command ("mate-session-save --shutdown-dialog");
        break;
      case PROMPT_WITH_BUDGIE:
        ayatana_common_utils_execute_command("gnome-session-quit --power-off");
        break;
      case PROMPT_WITH_XFCE:
        ayatana_common_utils_execute_command ("xfce4-session-logout");
        break;

      case PROMPT_WITH_UTILS:
        if (ayatana_common_utils_zenity_question ("system-shutdown",
                                                _("Shut Down"),
                                                _("Are you sure you want to close all programs and shut down the computer?"),
                                                _("Shut Down"),
                                                _("Cancel")))
        {
            power_off_now (self);
        }

        break;

      case PROMPT_NONE:
        power_off_now (self);
        break;
    }
}

/***
****
***/

static void
my_desktop_help (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  if (g_strcmp0(get_desktop_session(), "ubuntu-touch") == 0)
    ayatana_common_utils_open_url("https://ubuntu-touch.io/");
  else if (ayatana_common_utils_have_budgie_program ("yelp"))
    ayatana_common_utils_execute_command ("yelp help:gnome-user-guide");
  else if (ayatana_common_utils_is_lomiri())
    ayatana_common_utils_open_url("https://lomiri.com");
  else if (ayatana_common_utils_have_gnome_program ("yelp"))
    ayatana_common_utils_execute_command ("yelp help:gnome-user-guide");
  else if (ayatana_common_utils_have_mate_program ("yelp"))
    ayatana_common_utils_execute_command ("yelp help:mate-user-guide");
  else if (ayatana_common_utils_is_xfce()) {
    if (!ayatana_common_utils_open_url("https://docs.xfce.org/"))
      ayatana_common_utils_zenity_warning ("dialog-warning",
                      _("Warning"),
                      _("The XFCE desktop's user guide needs to be accessed with\na web browser.\n\nThe Ayatana Session Indicator could not find any web\nbrowser on your computer."));
  }
  else
    ayatana_common_utils_zenity_warning ("dialog-warning",
                    _("Warning"),
                    _("The Ayatana Session Indicator does not know yet, how to show\nthe currently running desktop's user guide or help center.\n\nPlease report this to the developers at:\nhttps://github.com/ArcticaProject/ayatana-indicator-session/issues"));
}

static void
my_distro_help (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  if (!ayatana_common_utils_open_url(get_distro_url()))
    ayatana_common_utils_zenity_warning ("dialog-warning",
                    _("Warning"),
                    g_strdup_printf(_("Displaying information on %s  requires\na web browser.\n\nThe Ayatana Session Indicator could not find any web\nbrowser on your computer."), get_distro_name()));
}

static void
my_settings (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  if (ayatana_common_utils_is_lomiri())
    ayatana_common_utils_open_url("settings:///system");
  else if (ayatana_common_utils_have_unity_program("unity-control-center"))
    ayatana_common_utils_execute_command ("unity-control-center");
  else if (ayatana_common_utils_have_gnome_program("gnome-control-center"))
    ayatana_common_utils_execute_command ("gnome-control-center");
  else if (ayatana_common_utils_have_mate_program ("mate-control-center"))
    ayatana_common_utils_execute_command ("mate-control-center");
  else if (ayatana_common_utils_have_xfce_program ("xfce4-settings-manager"))
    ayatana_common_utils_execute_command ("xfce4-settings-manager");
  else
    ayatana_common_utils_zenity_warning ("dialog-warning",
                    _("Warning"),
                    _("The Ayatana Session Indicator does not support evoking the system\nsettings application for your desktop environment, yet.\n\nPlease report this to the developers at:\nhttps://github.com/ArcticaProject/ayatana-indicator-session/issues"));
}

static void
my_online_accounts (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  if (ayatana_common_utils_is_lomiri())
    ayatana_common_utils_open_url("settings:///system/online-accounts");
  else if (ayatana_common_utils_have_unity_program("unity-control-center"))
    ayatana_common_utils_execute_command ("unity-control-center credentials");
  else if (ayatana_common_utils_have_gnome_program("gnome-control-center"))
    ayatana_common_utils_execute_command ("gnome-control-center credentials");
  else
    ayatana_common_utils_zenity_warning ("dialog-warning",
                    _("Warning"),
                    _("The Ayatana Session Indicator does not support password changes\nfor your desktop environment, yet.\n\nPlease report this to the developers at:\nhttps://github.com/ArcticaProject/ayatana-indicator-session/issues"));
}

static void
my_about (IndicatorSessionActions * self G_GNUC_UNUSED)
{
  if (ayatana_common_utils_is_lomiri())
    ayatana_common_utils_open_url("settings:///system/system-information");
  else if (ayatana_common_utils_have_unity_program("unity-control-center"))
    ayatana_common_utils_execute_command ("unity-control-center info");
  else if (ayatana_common_utils_have_budgie_program("gnome-control-center"))
    ayatana_common_utils_execute_command ("gnome-control-center info-overview");
  else if (ayatana_common_utils_have_gnome_program("gnome-control-center"))
    ayatana_common_utils_execute_command ("gnome-control-center info");
  else if (ayatana_common_utils_have_mate_program ("mate-system-monitor"))
    ayatana_common_utils_execute_command ("mate-system-monitor --show-system-tab");
  else if (ayatana_common_utils_have_xfce_program ("xfce4-about"))
    ayatana_common_utils_execute_command ("xfce4-about");
  else
    ayatana_common_utils_zenity_warning ("dialog-warning",
                    _("Warning"),
                    _("The Ayatana Session Indicator does not know yet, how to show\ninformation of the currently running desktop environment.\n\nPlease report this to the developers at:\nhttps://github.com/ArcticaProject/ayatana-indicator-session/issues"));
}

/***
****
***/

static void
lock_current_session (IndicatorSessionActions * self, gboolean immediate)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  if (is_owned_proxy (p->lomiri_session))
    {
      if (immediate)
        {
          lomiri_shell_session_call_prompt_lock (p->lomiri_session, p->cancellable, NULL, NULL);
        }
      else
        {
          lomiri_shell_session_call_lock (p->lomiri_session, p->cancellable, NULL, NULL);
        }
    }
  else if (ayatana_common_utils_have_mate_program ("mate-screensaver-command"))
    {
      ayatana_common_utils_execute_command ("mate-screensaver-command --lock");
    }
  else if (ayatana_common_utils_have_budgie_program("gnome-screensaver-command"))
    {
        ayatana_common_utils_execute_command("gnome-screensaver-command --lock");
    }
  else if (ayatana_common_utils_have_xfce_program ("xflock4"))
    {
      ayatana_common_utils_execute_command ("xflock4");
    }
  else
    {
      g_return_if_fail (p->screen_saver != NULL);

      gnome_screen_saver_call_lock (p->screen_saver, p->cancellable, NULL, NULL);
    }
}

static void
my_switch_to_screensaver (IndicatorSessionActions * self)
{
  lock_current_session (self, TRUE);
}

static void
my_switch_to_greeter (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->dm_seat != NULL);

  lock_current_session (self, TRUE);

  display_manager_seat_call_switch_to_greeter (p->dm_seat,
                                               p->dm_seat_cancellable,
                                               NULL, NULL);
}

static void
my_switch_to_guest (IndicatorSessionActions * self)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->dm_seat != NULL);

  lock_current_session (self, TRUE);

  display_manager_seat_call_switch_to_guest (p->dm_seat, "",
                                             p->dm_seat_cancellable,
                                             NULL, NULL);
}

static void
my_switch_to_username (IndicatorSessionActions * self, const char * username)
{
  priv_t * p = INDICATOR_SESSION_ACTIONS_DBUS(self)->priv;

  g_return_if_fail (p->dm_seat != NULL);

  lock_current_session (self, TRUE);

  display_manager_seat_call_switch_to_user (p->dm_seat, username, "",
                                            p->dm_seat_cancellable,
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

  if (p->indicator_settings != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->indicator_settings, self);
      g_clear_object (&p->indicator_settings);
    }

  if (p->lockdown_settings != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->lockdown_settings, self);
      g_clear_object (&p->lockdown_settings);
    }

  if (p->webcredentials != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->webcredentials, self);
      g_clear_object (&p->webcredentials);
    }

  if (p->end_session_dialog != NULL)
    {
      stop_listening_to_dialog (self);
      g_clear_object (&p->end_session_dialog);
    }

  g_clear_object (&p->screen_saver);
  g_clear_object (&p->session_manager);
  g_clear_object (&p->lomiri_session);
  set_dm_seat (self, NULL);
  set_login1_manager (self, NULL);
  set_login1_seat (self, NULL);

  G_OBJECT_CLASS (indicator_session_actions_dbus_parent_class)->dispose (o);
}

static void
my_finalize (GObject * o)
{

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
  object_class->finalize = my_finalize;

  actions_class = INDICATOR_SESSION_ACTIONS_CLASS (klass);
  actions_class->can_lock = my_can_lock;
  actions_class->can_logout = my_can_logout;
  actions_class->can_reboot = my_can_reboot;
  actions_class->can_switch = my_can_switch;
  actions_class->can_suspend = my_can_suspend;
  actions_class->can_hibernate = my_can_hibernate;
  actions_class->can_prompt = my_can_prompt;
  actions_class->has_online_account_error = my_has_online_account_error;
  actions_class->logout = my_logout;
  actions_class->suspend = my_suspend;
  actions_class->hibernate = my_hibernate;
  actions_class->reboot = my_reboot;
  actions_class->power_off = my_power_off;
  actions_class->settings = my_settings;
  actions_class->online_accounts = my_online_accounts;
  actions_class->desktop_help = my_desktop_help;
  actions_class->distro_help = my_distro_help;
  actions_class->bug = my_bug;
  actions_class->about = my_about;
  actions_class->switch_to_screensaver = my_switch_to_screensaver;
  actions_class->switch_to_greeter = my_switch_to_greeter;
  actions_class->switch_to_guest = my_switch_to_guest;
  actions_class->switch_to_username = my_switch_to_username;
}

static void
/* cppcheck-suppress unusedFunction */
indicator_session_actions_dbus_init (IndicatorSessionActionsDbus * self)
{
  priv_t * p;
  GSettings * s = null;

  p = indicator_session_actions_dbus_get_instance_private (self);
  p->cancellable = g_cancellable_new ();
  p->seat_allows_activation = TRUE;
  self->priv = p;

  if (ayatana_common_utils_is_gnome() || ayatana_common_utils_is_budgie())
    {
      s = g_settings_new ("org.gnome.desktop.lockdown");
    }
  elif (ayatana_common_utils_is_mate())
    {
      s = g_settings_new ("org.mate.desktop.lockdown");
    }

  if (s)
    {
      g_signal_connect_swapped (s, "changed::disable-lock-screen",
                                G_CALLBACK(indicator_session_actions_notify_can_lock), self);
      g_signal_connect_swapped (s, "changed::disable-log-out",
                                G_CALLBACK(indicator_session_actions_notify_can_logout), self);
      g_signal_connect_swapped (s, "changed::disable-user-switching",
                                G_CALLBACK(indicator_session_actions_notify_can_switch), self);
      p->lockdown_settings = s;
    }

  s = g_settings_new ("org.ayatana.indicator.session");
  g_signal_connect_swapped (s, "changed::suppress-logout-restart-shutdown",
                            G_CALLBACK(indicator_session_actions_notify_can_prompt), self);
  g_signal_connect_swapped (s, "changed::suppress-logout-restart-shutdown",
                            G_CALLBACK(indicator_session_actions_notify_can_reboot), self);
  g_signal_connect_swapped (s, "changed::suppress-restart-menuitem",
                            G_CALLBACK(indicator_session_actions_notify_can_reboot), self);
  g_signal_connect_swapped (s, "changed::suppress-shutdown-menuitem",
                            G_CALLBACK(indicator_session_actions_notify_can_reboot), self);
  g_signal_connect_swapped (s, "changed::force-restart-menuitem",
                            G_CALLBACK(indicator_session_actions_notify_can_reboot), self);
  p->indicator_settings = s;

  gnome_screen_saver_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        "org.gnome.ScreenSaver",
                                        "/org/gnome/ScreenSaver",
                                        p->cancellable,
                                        on_screensaver_proxy_ready,
                                        self);

  lomiri_shell_session_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    "com.lomiri.Shell",
                                    "/com/lomiri/Shell/Session",
                                    p->cancellable,
                                    on_lomiri_proxy_ready,
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
                                    "com.lomiri.indicators.webcredentials",
                                    "/com/lomiri/indicators/webcredentials",
                                    p->cancellable,
                                    on_webcredentials_proxy_ready,
                                    self);

  end_session_dialog_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                        "com.lomiri.Shell",
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
                                            Login1Manager               * login1_manager,
                                            Login1Seat                  * login1_seat,
                                            DisplayManagerSeat          * dm_seat)
{
  g_return_if_fail (INDICATOR_IS_SESSION_ACTIONS_DBUS(self));

  set_login1_manager (self, login1_manager);
  set_login1_seat (self, login1_seat);
  set_dm_seat (self, dm_seat);
}
