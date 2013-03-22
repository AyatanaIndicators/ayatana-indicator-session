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

#include "mock-upower.h"


gboolean
MockUPower :: handle_suspend (UPower                * upower,
                              GDBusMethodInvocation * inv,
                              gpointer                gself)
{
  static_cast<MockUPower*>(gself)->my_last_action = Suspend;
  upower_complete_suspend (upower, inv);
  return true;
}

gboolean
MockUPower :: handle_hibernate (UPower                * upower,
                                GDBusMethodInvocation * inv,
                                gpointer                gself)
{
  static_cast<MockUPower*>(gself)->my_last_action = Hibernate;
  upower_complete_hibernate (upower, inv);
  return true;
}

gboolean
MockUPower :: handle_suspend_allowed (UPower                * upower,
                                      GDBusMethodInvocation * inv,
                                      gpointer                gself)
{
  const bool allowed = static_cast<MockUPower*>(gself)->my_can_suspend;
  upower_complete_suspend_allowed (upower, inv, allowed);
  return true;
}

gboolean
MockUPower :: handle_hibernate_allowed (UPower                * upower,
                                        GDBusMethodInvocation * inv,
                                        gpointer                gself)
{
  const bool allowed = static_cast<MockUPower*>(gself)->my_can_hibernate;
  upower_complete_hibernate_allowed (upower, inv, allowed);
  return true;
}

/***
****
***/

namespace
{
  const char * const UPOWER_NAME = "org.freedesktop.UPower";
  const char * const UPOWER_PATH = "/org/freedesktop/UPower";

}

MockUPower :: MockUPower (GMainLoop       * loop,
                          GDBusConnection * bus_connection):
  MockObject (loop, bus_connection, UPOWER_NAME, UPOWER_PATH),
  my_skeleton (upower_skeleton_new ()),
  my_can_suspend (true),
  my_can_hibernate (true),
  my_suspend_allowed (true),
  my_hibernate_allowed (true),
  my_last_action (None)
{
  //set_can_hibernate (false);
  //set_can_suspend (true);

  g_signal_connect (my_skeleton, "handle-suspend",
                    G_CALLBACK(handle_suspend), this);
  g_signal_connect (my_skeleton, "handle-suspend-allowed",
                    G_CALLBACK(handle_suspend_allowed), this);

  g_signal_connect (my_skeleton, "handle-hibernate",
                    G_CALLBACK(handle_hibernate), this);
  g_signal_connect (my_skeleton, "handle-hibernate-allowed",
                    G_CALLBACK(handle_hibernate_allowed), this);

  set_skeleton (G_DBUS_INTERFACE_SKELETON(my_skeleton));
}

MockUPower :: ~MockUPower ()
{
  g_clear_object (&my_skeleton);
}
