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

#include "mock-consolekit-seat.h"

#include "mock-object.h"
#include "mock-consolekit-session.h"
#include "mock-user.h"

namespace
{
  const char * CONSOLEKIT_BUS_NAME = "org.freedesktop.ConsoleKit";

  std::string next_unique_sid ()
  {
    static int id = 1;

    char * tmp;
    std::string ret;

    tmp = g_strdup_printf ("/org/freedesktop/ConsoleKit/Seat%d", id++);
    ret = tmp;
    g_free (tmp);
    return ret;
  }
}

/***
****
***/

void
MockConsoleKitSeat :: activate_session (MockConsoleKitSession * session)
{
  g_assert (my_sessions.count(session) == 1);

  const char * ssid = session->ssid ();
  if (my_active_ssid != ssid)
    {
      my_active_ssid = ssid;
      console_kit_seat_emit_active_session_changed (my_skeleton, ssid);
    }
}

void
MockConsoleKitSeat :: switch_to_guest ()
{
  for (sessions_t::iterator it(my_sessions.begin()),
                           end(my_sessions.end()); it!=end; ++it)
    {
      MockConsoleKitSession * session (*it);

      if (session->user()->is_guest())
        {
          activate_session (*it);
          return;
        }
    }

  g_warn_if_reached ();
}

void
MockConsoleKitSeat :: switch_to_user (const char * username)
{
  for (sessions_t::iterator it(my_sessions.begin()),
                           end(my_sessions.end()); it!=end; ++it)
    {
      MockConsoleKitSession * session (*it);

      if (!g_strcmp0 (username, session->user()->username()))
        {
          activate_session (*it);
          return;
        }
    }

  g_warn_if_reached ();
}

/***
****
***/

MockConsoleKitSession *
MockConsoleKitSeat :: add_session_by_user (MockUser * mu)
{
  g_assert (mu != 0);

  MockConsoleKitSession * session;

  session = new MockConsoleKitSession (my_loop, my_bus_connection);
  session->set_user (mu);
  add_session (session);
  return session;
}

void
MockConsoleKitSeat :: add_session (MockConsoleKitSession * session)
{
  g_assert (my_sessions.count(session) == 0);

  my_sessions.insert (session);
  session->set_sid (path());
  console_kit_seat_emit_session_added (my_skeleton, session->ssid());
}

void
MockConsoleKitSeat :: remove_session (MockConsoleKitSession * session)
{
  g_assert (my_sessions.count(session) == 1);

  my_sessions.erase (session);
  session->set_sid ("");
  console_kit_seat_emit_session_removed (my_skeleton, session->ssid());
}

/***
****  Handlers
***/

gboolean
MockConsoleKitSeat :: on_can_activate_sessions (ConsoleKitSeat        * cks,
                                                GDBusMethodInvocation * inv,
                                                gpointer                gself)
{
  bool b = static_cast<MockConsoleKitSeat*>(gself)->my_can_activate_sessions;
  console_kit_seat_complete_can_activate_sessions (cks, inv, b);
  return true;
}

gboolean
MockConsoleKitSeat :: on_get_active_session (ConsoleKitSeat        * cks,
                                             GDBusMethodInvocation * invoke,
                                             gpointer                gself)
{
  std::string ssid = static_cast<MockConsoleKitSeat*>(gself)->my_active_ssid;
  console_kit_seat_complete_get_active_session (cks, invoke, ssid.c_str());
  return true;
}

gboolean
MockConsoleKitSeat :: on_get_sessions (ConsoleKitSeat         * cks,
                                       GDBusMethodInvocation  * inv,
                                       gpointer                 gself)
{
  int i;
  const char ** paths;
  sessions_t& sessions = static_cast<MockConsoleKitSeat*>(gself)->my_sessions;

  i = 0;
  paths = g_new0 (const char*, sessions.size() + 1);
  for (sessions_t::iterator it(sessions.begin()),
                           end(sessions.end()); it!=end; ++it)
   paths[i++] = (*it)->path();

  g_debug ("returning a list of %d sessions", i);
  console_kit_seat_complete_get_sessions (cks, inv, paths);
  g_free (paths);

  return true;
}

MockConsoleKitSession *
MockConsoleKitSeat :: find (const char * ssid)
{
  for (sessions_t::iterator it(my_sessions.begin()),
                           end(my_sessions.end()); it!=end; ++it)
    if (!g_strcmp0 ((*it)->path(), ssid))
      return *it;

  return 0;
}

/***
****  Life Cycle
***/

MockConsoleKitSeat :: MockConsoleKitSeat (GMainLoop       * loop,
                                          GDBusConnection * bus_connection,
                                          bool              can_activate_sessions):
  MockObject (loop, bus_connection, CONSOLEKIT_BUS_NAME, next_unique_sid()),
  my_skeleton (console_kit_seat_skeleton_new ()),
  my_can_activate_sessions (can_activate_sessions)
{
  g_signal_connect (my_skeleton, "handle-get-active-session",
                    G_CALLBACK(on_get_active_session), this);
  g_signal_connect (my_skeleton, "handle-get-sessions",
                    G_CALLBACK(on_get_sessions), this);
  g_signal_connect (my_skeleton, "handle-can-activate-sessions",
                    G_CALLBACK(on_can_activate_sessions), this);

  set_skeleton (G_DBUS_INTERFACE_SKELETON(my_skeleton));
}

MockConsoleKitSeat :: ~MockConsoleKitSeat ()
{
  for (sessions_t::iterator it(my_sessions.begin()),
                           end(my_sessions.end()); it!=end; ++it)
    delete *it;

  g_signal_handlers_disconnect_by_data (my_skeleton, this);
  g_clear_object (&my_skeleton);
}
