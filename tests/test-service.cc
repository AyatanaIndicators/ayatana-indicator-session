/*
Copyright 2012 Canonical Ltd.

Authors:
    Charles Kerr <charles.kerr@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib-object.h>
#include <gio/gio.h>
#include <glib.h>

#include <gtest/gtest.h>

#include "shared-names.h"

/***
****
***/

#define INDICATOR_SERVICE_OBJECT_PATH "/org/ayatana/indicator/service"
#define INDICATOR_SERVICE_INTERFACE_NAME "org.ayatana.indicator.service"

class ClientTest : public ::testing::Test
{
  protected:

    GTestDBus * test_dbus;
    GDBusConnection * session_bus;
    GMainLoop * main_loop;

    virtual void SetUp()
    {
      test_dbus = NULL;
      session_bus = NULL;
      main_loop = NULL;

      static bool ran_once_init = false;
      if (!ran_once_init)
        {
          g_type_init();
          g_setenv ("INDICATOR_SERVICE_SHUTDOWN_TIMEOUT", "1000", TRUE);
          g_unsetenv ("INDICATOR_ALLOW_NO_WATCHERS");
          g_unsetenv ("INDICATOR_SERVICE_REPLACE_MODE");
          ran_once_init = true;
        }

      main_loop = g_main_loop_new (NULL, FALSE);
      // pull up a test dbus that's pointed at our test .service file
      test_dbus = g_test_dbus_new (G_TEST_DBUS_NONE);
      g_debug (G_STRLOC" service dir path is \"%s\"", INDICATOR_SERVICE_DIR);
      g_test_dbus_add_service_dir (test_dbus, INDICATOR_SERVICE_DIR);

      // allow the service to exist w/o a sync indicator
      g_setenv ("INDICATOR_ALLOW_NO_WATCHERS", "1", TRUE);

      g_test_dbus_up (test_dbus);
      g_debug (G_STRLOC" this test bus' address is \"%s\"", g_test_dbus_get_bus_address(test_dbus));
      session_bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
      g_debug (G_STRLOC" the dbus connection %p unique name is \"%s\"", session_bus, g_dbus_connection_get_unique_name(session_bus));
      g_debug (G_STRLOC" the dbus connection %p refcount is %d", session_bus, G_OBJECT(session_bus)->ref_count);
    }

    // undo SetUp
    virtual void TearDown()
    {
      g_clear_object (&session_bus);
      g_debug (G_STRLOC" tearing down the bus");
      g_test_dbus_down (test_dbus);
      g_clear_object (&test_dbus);
      g_clear_pointer (&main_loop, g_main_loop_unref);
    }
};

/***
****
***/

/**
 * This is a basic test to see if we can launch indicator-session-service
 * and call its "GetUserRealName" method. The test succeeds if we can launch
 * the service, call the method, and get response that equals g_get_real_name().
 *
 * (You may be wondering why GetUserRealName() exists at all, instead of clients
 * using g_get_real_name(). It's because the former updates itslef when the user
 * edits his real name, while the latter returns its cached copy of the old name.)
 */
TEST_F (ClientTest, TestCanStartService)
{
  GError * error;
  GVariant * result;
  const gchar * name;

  // call GetUserRealName(), which as a side effect should activate
  // indicator-session-service via the .service file in the tests/ directory
  error = NULL;
g_message ("calling g_dbus_connection_call_sync");
  result = g_dbus_connection_call_sync (session_bus,
                                        INDICATOR_SESSION_DBUS_NAME,
                                        INDICATOR_SESSION_SERVICE_DBUS_OBJECT,
                                        INDICATOR_SESSION_SERVICE_DBUS_IFACE,
                                        "GetUserRealName",
                                        NULL,
                                        G_VARIANT_TYPE("(s)"),
                                        G_DBUS_CALL_FLAGS_NONE,
                                        -1,
                                        NULL,
                                        &error);
g_message ("done calling g_dbus_connection_call_sync");

  EXPECT_TRUE (error == NULL);
  ASSERT_TRUE (result != NULL);

  if (error != NULL)
    {
      g_warning ("GetUserRealName failed: %s", error->message);
      g_clear_error (&error);
    }

  name = NULL;
  g_variant_get (result, "(&s)", &name);
  ASSERT_STREQ (g_get_real_name(), name);
  g_clear_pointer (&result, g_variant_unref);

  // call IndicatorService's Shutdown() method for a clean exit
g_message ("calling g_dbus_connection_call_sync");
  result = g_dbus_connection_call_sync (session_bus,
                                        INDICATOR_SESSION_DBUS_NAME,
                                        "/org/ayatana/indicator/service",
                                        "org.ayatana.indicator.service",
                                        "Shutdown", NULL,
                                        NULL,
                                        G_DBUS_CALL_FLAGS_NONE,
                                        -1, NULL, NULL);
g_message ("calling g_clear_pointer");
  g_clear_pointer (&result, g_variant_unref);
}
