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

#include "mock-login1-manager.h"
#include "mock-login1-seat.h"
#include "mock-user.h"

namespace
{
  const char * const BUS_NAME = "org.freedesktop.login1";
  
  const char * const BUS_PATH = "/org/freedesktop/login1";

#if 0
  void on_active_session_changed (Login1Seat  * o          G_GNUC_UNUSED,
                                  const gchar     * new_ssid,
                                  gpointer          ssid)
  {
    *static_cast<std::string*>(ssid) = new_ssid;
  }
#endif
}

/***
****
***/

#if 0
gboolean
MockLogin1Manager :: on_get_current_session (Login1Manager     * m,
                                                 GDBusMethodInvocation * inv,
                                                 gpointer                gself)
{
  MockLogin1Manager * self = static_cast<MockLogin1Manager*>(gself);
  const std::string& ssid = self->my_current_ssid;
  console_kit_manager_complete_get_current_session (m, inv, ssid.c_str());
  return true;
}

gboolean
MockLogin1Manager :: on_get_seats (Login1Manager     * m,
                                       GDBusMethodInvocation * inv,
                                       gpointer                gself)
{
  int i;
  char ** sids;
  const seats_t& seats = static_cast<MockLogin1Manager*>(gself)->my_seats;

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
MockLogin1Manager :: handle_restart (Login1Manager     * ckm,
                                         GDBusMethodInvocation * inv,
                                         gpointer                gself)
{
  static_cast<MockLogin1Manager*>(gself)->my_last_action = Restart;
  console_kit_manager_complete_restart (ckm, inv);
  return true;
}

gboolean
MockLogin1Manager :: handle_stop (Login1Manager     * ckm,
                                      GDBusMethodInvocation * inv,
                                      gpointer                gself)
{
  static_cast<MockLogin1Manager*>(gself)->my_last_action = Shutdown;
  console_kit_manager_complete_stop (ckm, inv);
  return true;
}
#endif

/***
****
***/

#if 0
MockLogin1Session *
MockLogin1Manager :: current_session ()
{
  MockLogin1Session * ret = 0;

  for (seats_t::iterator it(my_seats.begin()),
                        end(my_seats.end()); it!=end; ++it)
    if ((ret = (*it)->find (my_current_ssid.c_str())))
      break;

  return ret;
}
#endif

void
MockLogin1Manager :: emit_session_new (MockLogin1Seat * seat, int tag) const
{
  std::string id;
  std::string path;

  seat->get_session_id_and_path_for_tag (tag, id, path);

g_message ("%s %s emitting session-new [%s][%s]", G_STRLOC, G_STRFUNC, id.c_str(), path.c_str());
  login1_manager_emit_session_new (my_skeleton, id.c_str(), path.c_str());
}

int
MockLogin1Manager :: add_session (MockLogin1Seat * seat, MockUser * user)
{
  g_assert (my_seats.count(seat) == 1);

  const int tag = seat->add_session (user);
  emit_session_new (seat, tag);
  return tag;
}

void
MockLogin1Manager :: add_seat (MockLogin1Seat * seat)
{
  g_assert (my_seats.count(seat) == 0);

  my_seats.insert (seat);
  std::set<int> sessions = seat->sessions ();
  for (auto tag : sessions)
    emit_session_new (seat, tag);
}

/***
****
***/
         
GVariant *
MockLogin1Manager :: list_sessions () const
{
  GVariantBuilder b;

  g_variant_builder_init (&b, G_VARIANT_TYPE("a(susso)"));

  for (auto seat : my_seats)
    {
      GVariant * seat_sessions = seat->list_sessions ();

      GVariantIter iter;
      g_variant_iter_init (&iter, seat_sessions);
      GVariant * child;
      while ((child = g_variant_iter_next_value (&iter)))
        {
          g_variant_builder_add_value (&b, child);
          g_variant_unref (child);
        }
    }

  GVariant * v = g_variant_builder_end (&b);
  g_message ("%s %s returning %s", G_STRLOC, G_STRFUNC, g_variant_print (v, true));
  return v;
}

/***
****  Skeleton Handlers
***/

gboolean
MockLogin1Manager :: handle_list_sessions (Login1Manager       * m,
                                         GDBusMethodInvocation * inv,
                                         gpointer                gself)
{
  GVariant * sessions = static_cast<MockLogin1Manager*>(gself)->list_sessions();
  login1_manager_complete_list_sessions (m, inv, sessions);
  return true;
}

gboolean
MockLogin1Manager :: handle_can_suspend (Login1Manager         * m,
                                         GDBusMethodInvocation * inv,
                                         gpointer                gself)
{
  const std::string& s = static_cast<MockLogin1Manager*>(gself)->can_suspend();
  login1_manager_complete_can_suspend (m, inv, s.c_str());
  return true;
}

gboolean
MockLogin1Manager :: handle_can_hibernate (Login1Manager         * m,
                                           GDBusMethodInvocation * inv,
                                           gpointer                gself)
{
  const std::string& s = static_cast<MockLogin1Manager*>(gself)->can_hibernate();
  login1_manager_complete_can_hibernate (m, inv, s.c_str());
  return true;
}

gboolean
MockLogin1Manager :: handle_reboot (Login1Manager         * m,
                                    GDBusMethodInvocation * inv,
                                    gboolean                interactive G_GNUC_UNUSED,
                                    gpointer                gself)
{
  static_cast<MockLogin1Manager*>(gself)->my_last_action = "reboot";
  login1_manager_complete_reboot (m, inv);
  return true;
}

gboolean
MockLogin1Manager :: handle_power_off (Login1Manager         * m,
                                       GDBusMethodInvocation * inv,
                                       gboolean                interactive G_GNUC_UNUSED,
                                       gpointer                gself)
{
  static_cast<MockLogin1Manager*>(gself)->my_last_action = "power-off";
  login1_manager_complete_power_off (m, inv);
  return true;
}

gboolean
MockLogin1Manager :: handle_suspend (Login1Manager         * m,
                                     GDBusMethodInvocation * inv,
                                     gboolean                interactive G_GNUC_UNUSED,
                                     gpointer                gself)
{
  static_cast<MockLogin1Manager*>(gself)->my_last_action = "suspend";
  login1_manager_complete_suspend (m, inv);
  return true;
}

gboolean
MockLogin1Manager :: handle_hibernate (Login1Manager         * m,
                                       GDBusMethodInvocation * inv,
                                       gboolean                interactive G_GNUC_UNUSED,
                                       gpointer                gself)
{
  static_cast<MockLogin1Manager*>(gself)->my_last_action = "hibernate";
  login1_manager_complete_hibernate (m, inv);
  return true;
}

/***
****
***/

const std::string&
MockLogin1Manager :: can_suspend () const
{
  return my_can_suspend;
}

const std::string&
MockLogin1Manager :: can_hibernate () const
{
  return my_can_hibernate;
}

/***
****
***/

MockLogin1Manager :: MockLogin1Manager (GMainLoop       * loop,
                                        GDBusConnection * conn):
  MockObject (loop, conn, BUS_NAME, BUS_PATH),
  my_skeleton (login1_manager_skeleton_new ()),
  my_can_suspend ("yes"),
  my_can_hibernate ("yes")
{
  g_signal_connect (my_skeleton, "handle-can-suspend",
                    G_CALLBACK(handle_can_suspend), this);
  g_signal_connect (my_skeleton, "handle-can-hibernate",
                    G_CALLBACK(handle_can_hibernate), this);
  g_signal_connect (my_skeleton, "handle_reboot",
                    G_CALLBACK(handle_reboot), this);
  g_signal_connect (my_skeleton, "handle-power-off",
                    G_CALLBACK(handle_power_off), this);
  g_signal_connect (my_skeleton, "handle-suspend",
                    G_CALLBACK(handle_suspend), this);
  g_signal_connect (my_skeleton, "handle-hibernate",
                    G_CALLBACK(handle_hibernate), this);
  g_signal_connect (my_skeleton, "handle-list-sessions",
                    G_CALLBACK(handle_list_sessions), this);

#if 0
  g_signal_connect (my_skeleton, "handle-get-seats",
                    G_CALLBACK(on_get_seats), this);
  g_signal_connect (my_skeleton, "handle-stop",
                    G_CALLBACK(handle_stop), this);
  g_signal_connect (my_skeleton, "handle-restart",
                    G_CALLBACK(handle_restart), this);
#endif

  set_skeleton (G_DBUS_INTERFACE_SKELETON(my_skeleton));
}

MockLogin1Manager :: ~MockLogin1Manager ()
{
  for (auto seat : my_seats)
    delete seat;

  g_signal_handlers_disconnect_by_data (my_skeleton, this);
  g_clear_object (&my_skeleton);
}
