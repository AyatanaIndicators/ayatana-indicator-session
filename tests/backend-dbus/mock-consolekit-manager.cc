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

#include "mock-consolekit-manager.h"
#include "mock-consolekit-seat.h"
#include "mock-consolekit-session.h"

namespace
{
  const char * CONSOLEKIT_MANAGER_NAME = "org.freedesktop.ConsoleKit";
  
  const char * CONSOLEKIT_MANAGER_PATH = "/org/freedesktop/ConsoleKit/Manager";

  void on_active_session_changed (ConsoleKitSeat  * o          G_GNUC_UNUSED,
                                  const gchar     * new_ssid,
                                  gpointer          ssid)
  {
    *static_cast<std::string*>(ssid) = new_ssid;
  }
}

/***
****
***/

gboolean
MockConsoleKitManager :: on_get_current_session (ConsoleKitManager     * m,
                                                 GDBusMethodInvocation * inv,
                                                 gpointer                gself)
{
  MockConsoleKitManager * self = static_cast<MockConsoleKitManager*>(gself);
  const std::string& ssid = self->my_current_ssid;
  console_kit_manager_complete_get_current_session (m, inv, ssid.c_str());
  return true;
}

gboolean
MockConsoleKitManager :: on_get_seats (ConsoleKitManager     * m,
                                       GDBusMethodInvocation * inv,
                                       gpointer                gself)
{
  int i;
  char ** sids;
  const seats_t& seats = static_cast<MockConsoleKitManager*>(gself)->my_seats;

  i = 0;
  sids = g_new0 (char*, seats.size()+1);
  for (seats_t::const_iterator it(seats.begin()),
                              end(seats.end()); it!=end; ++it)
    sids[i++] = (char*) (*it)->path();
  console_kit_manager_complete_get_seats (m, inv, sids);
  g_strfreev (sids);

  return true;
}

gboolean
MockConsoleKitManager :: handle_restart (ConsoleKitManager     * ckm,
                                         GDBusMethodInvocation * inv,
                                         gpointer                gself)
{
  static_cast<MockConsoleKitManager*>(gself)->my_last_action = Restart;
  console_kit_manager_complete_restart (ckm, inv);
  return true;
}

gboolean
MockConsoleKitManager :: handle_stop (ConsoleKitManager     * ckm,
                                      GDBusMethodInvocation * inv,
                                      gpointer                gself)
{
  static_cast<MockConsoleKitManager*>(gself)->my_last_action = Shutdown;
  console_kit_manager_complete_stop (ckm, inv);
  return true;
}

/***
****
***/

MockConsoleKitSession *
MockConsoleKitManager :: current_session ()
{
  MockConsoleKitSession * ret = 0;

  for (seats_t::iterator it(my_seats.begin()),
                        end(my_seats.end()); it!=end; ++it)
    if ((ret = (*it)->find (my_current_ssid.c_str())))
      break;

  return ret;
}

void
MockConsoleKitManager :: add_seat (MockConsoleKitSeat * seat)
{
  g_assert (my_seats.count(seat) == 0);

  my_seats.insert (seat);

  console_kit_manager_emit_seat_added (my_skeleton, seat->sid());

  g_signal_connect (seat->skeleton(), "active-session-changed",
                    G_CALLBACK(on_active_session_changed), &my_current_ssid);
}

/***
****
***/

MockConsoleKitManager :: MockConsoleKitManager (GMainLoop       * loop,
                                                GDBusConnection * conn):
  MockObject (loop, conn, CONSOLEKIT_MANAGER_NAME, CONSOLEKIT_MANAGER_PATH),
  my_skeleton (console_kit_manager_skeleton_new ()),
  my_last_action (None)
{
  g_signal_connect (my_skeleton, "handle-get-current-session",
                    G_CALLBACK(on_get_current_session), this);
  g_signal_connect (my_skeleton, "handle-get-seats",
                    G_CALLBACK(on_get_seats), this);
  g_signal_connect (my_skeleton, "handle-stop",
                    G_CALLBACK(handle_stop), this);
  g_signal_connect (my_skeleton, "handle-restart",
                    G_CALLBACK(handle_restart), this);

  set_skeleton (G_DBUS_INTERFACE_SKELETON(my_skeleton));
}

MockConsoleKitManager :: ~MockConsoleKitManager ()
{
  for (seats_t::iterator it(my_seats.begin()); it!=my_seats.end(); ++it)
    {
      MockConsoleKitSeat * seat = *it;
      g_signal_handlers_disconnect_by_data (seat->skeleton(), &my_current_ssid);
      delete seat;
    }

  g_signal_handlers_disconnect_by_data (my_skeleton, this);
  g_clear_object (&my_skeleton);
}
