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

#ifndef INDICATOR_SERVICE_TEST_H
#define INDICATOR_SERVICE_TEST_H

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include <gio/gio.h>
#include <gtest/gtest.h>
#include <libdbustest/dbus-test.h>
#include <libdbusmenu-glib/client.h>

/***
****
***/

/**
 * Convenience class for looking at a DbusmenuClient's items for testing
 *
 * Examples:
 *
 * // confirm that there are N menuitems of type T
 * TEST_EQ (helper.count_type(T), N);
 *
 * // confirm that there are N visible menuitems
 * TEST_EQ (helper.count_property_bool(DBUSMENU_MENUITEM_PROP_VISIBLE,true), N);
 *
 * // get a sorted list of all the menuitems of type T
 * std::vector<DbusmenuMenuitem*> items = helper.find_type(T);
 */
class DbusmenuClientHelper
{
  public:

    DbusmenuClientHelper (DbusmenuClient * client_): client(client_) {
      g_object_ref (G_OBJECT(client));
    }
    ~DbusmenuClientHelper() {
      g_object_unref(G_OBJECT(client));
      client = NULL;
    }

  private:

    static void foreach_accumulate_func (DbusmenuMenuitem * mi, gpointer gset) {
      static_cast<std::vector<DbusmenuMenuitem*>*>(gset)->push_back (mi);
    }

  public:

    std::vector<DbusmenuMenuitem*>
    get_all_menuitems () const
    {
      std::vector<DbusmenuMenuitem*> items;

      DbusmenuMenuitem * root = dbusmenu_client_get_root (client);
      if (root != NULL)
        dbusmenu_menuitem_foreach (root, foreach_accumulate_func, &items);

      return items;
    }

  private:

    template<typename value_type> class PropertyPredicate:
        public std::unary_function<DbusmenuMenuitem*,bool> {
      protected:
        const std::string _key;
        const value_type _value;
        virtual value_type get_value(DbusmenuMenuitem * mi) const = 0;
      public:
        PropertyPredicate (const char * propertyName, value_type propertyValue):
          _key(propertyName), _value(propertyValue) { }
        bool operator()(const DbusmenuMenuitem* cmi) const {
          // FIXME: remove const_cast after the dbusmenu_menuitem_propperty_get*() functions are constified
          DbusmenuMenuitem * mi = const_cast<DbusmenuMenuitem*>(cmi);
          return dbusmenu_menuitem_property_exist (mi, _key.c_str()) && (_value == get_value(mi));
        }
    };

    class StringPropertyPredicate: public PropertyPredicate<std::string> {
      protected:
        virtual std::string get_value (DbusmenuMenuitem * mi) const {
          return dbusmenu_menuitem_property_get(mi, _key.c_str());
        }
      public:
        StringPropertyPredicate (const char * propName, const char * propValue):
          PropertyPredicate (propName, propValue) {}
    };

    class IntPropertyPredicate: public PropertyPredicate<int> {
      protected:
        virtual int get_value (DbusmenuMenuitem * mi) const {
          return dbusmenu_menuitem_property_get_int(mi, _key.c_str());
        }
      public:
        IntPropertyPredicate (const char * propName, int propValue):
          PropertyPredicate (propName, propValue) {}
    };

    class BoolPropertyPredicate: public PropertyPredicate<bool> {
      protected:
        virtual bool get_value (DbusmenuMenuitem * mi) const {
          return dbusmenu_menuitem_property_get_bool(mi, _key.c_str());
        }
      public:
        BoolPropertyPredicate (const char * propName, bool propValue):
          PropertyPredicate (propName, propValue) {}
    };

  public:

    typedef std::vector<DbusmenuMenuitem*> menuitems_t;

    void
    match_property (menuitems_t& items, const char * key, const char * value) const
    {
      const StringPropertyPredicate pred (key, value);
      items.erase (std::remove_if (items.begin(), items.end(), std::not1(pred)), items.end());
    }

    void
    match_property_int (menuitems_t& items, const char * key, int value) const
    {
      const IntPropertyPredicate pred (key, value);
      items.erase (std::remove_if (items.begin(), items.end(), std::not1(pred)), items.end());
    }

    void
    match_property_bool (menuitems_t& items, const char * key, bool value) const
    {
      const BoolPropertyPredicate pred (key, value);
      items.erase (std::remove_if (items.begin(), items.end(), std::not1(pred)), items.end());
    }
      
    menuitems_t find_property (const char * prop_name, const char * prop_value) const
    {
      menuitems_t items;
      g_return_val_if_fail (prop_name!=NULL, items);
      g_return_val_if_fail (prop_value!=NULL, items);

      items = get_all_menuitems ();
      match_property (items, prop_name, prop_value);
      return items;
    }
 
    menuitems_t find_property_int (const char * prop_name, int prop_value) const
    {
      std::vector<DbusmenuMenuitem*> items;
      g_return_val_if_fail (prop_name!=NULL, items);

      items = get_all_menuitems ();
      match_property_int (items, prop_name, prop_value);
      return items;
    }

    menuitems_t find_property_bool (const char * prop_name, bool prop_value) const
    {
      std::vector<DbusmenuMenuitem*> items;
      g_return_val_if_fail (prop_name!=NULL, items);

      items = get_all_menuitems ();
      match_property_bool (items, prop_name, prop_value);
      return items;
    }

    menuitems_t find_type (const char * type) const
    {
      return find_property (DBUSMENU_MENUITEM_PROP_TYPE, type);
    }

    int count_property (const char * propName, const char * propValue) const
    {
      return find_property (propName, propValue).size();
    }

    int count_type (const char * type) const
    {
      return count_property (DBUSMENU_MENUITEM_PROP_TYPE, type);
    }

    int count_property_int (const char * propName, int propValue) const
    {
      return find_property_int (propName, propValue).size();
    }

    int count_property_bool (const char * propName, bool propValue) const
    {
      return find_property_bool (propName, propValue).size();
    }

  private:

    DbusmenuClient * client;
};

/**
 * Fixture class for using Google Test on an indicator-service's
 * com.canonical.dbusmenu interface.
 *
 * The SetUp() function starts the service up, waits for it to
 * be visible on the bus, then creates a DbusmenuClient and waits
 * for its layout-changed signal. This way the test function
 * is reached after the menu is available and populated.
 *
 * TearDown() cleans up the DBus scaffolding and stops the service.
 */
class IndicatorServiceTest : public ::testing::Test
{
  public:

    IndicatorServiceTest(const char * service_name_,
                         const char * menu_object_path_,
                         const char * executable_):
      menu_client(0),
      menu_helper(0),
      test_service(0),
      indicator_service_proxy(0),
      handler_id(0),
      executable(executable_),
      service_name(service_name_),
      menu_object_path(menu_object_path_)
    {
      // glib one-time init stuff
      g_type_init();
      g_assert (g_thread_supported());
      mainloop = g_main_loop_new (NULL, FALSE);
    }

  private:

    static void
    on_layout_updated_static (DbusmenuClient * client, IndicatorServiceTest * self)
    {
      g_debug ("LAYOUT UPDATED");
      self->on_layout_updated (client);
    }
    void
    on_layout_updated (DbusmenuClient * client)
    {
      ASSERT_EQ (client, menu_client);
      ASSERT_NE (handler_id, 0);
      ASSERT_TRUE (g_signal_handler_is_connected (client, handler_id));

      // stop listening for this event
      g_signal_handler_disconnect (client, handler_id);
      handler_id = 0;

      ready();
    }

  private:

    static gboolean
    on_timeout_static (gpointer self)
    {
      static_cast<IndicatorServiceTest*>(self)->on_timeout();
      return false;
    }
    void
    on_timeout()
    {
      ASSERT_NE (handler_id, 0ul);
      g_source_remove (handler_id);
      handler_id = 0;
      ready();
    }

  protected:

    virtual void
    SetUp()
    {
      ASSERT_EQ (NULL, test_service);
      ASSERT_EQ (NULL, menu_helper);
      ASSERT_EQ (NULL, indicator_service_proxy);

      test_service = dbus_test_service_new (NULL);

      // Start the executable and wait until it shows up on the bus.
      // Unset the NO_WATCHERS env var to ensure that the service
      // will shut down when there are no watchers... otherwise
      // this task will never finish
      g_unsetenv("INDICATOR_ALLOW_NO_WATCHERS");
      DbusTestProcess * indicator_service_task = dbus_test_process_new (executable.c_str());
      dbus_test_service_add_task (test_service, DBUS_TEST_TASK(indicator_service_task));
      g_object_unref (G_OBJECT(indicator_service_task));

      // create a menu task that waits for our service before it runs
      DbusTestTask * wait_task = dbus_test_task_new ();
      dbus_test_task_set_wait_for (wait_task, service_name.c_str());
      dbus_test_service_add_task (test_service, wait_task);
      g_object_unref (G_OBJECT(wait_task));

      g_debug ("starting tasks");
      dbus_test_service_start_tasks(test_service);

      // at this point the indicator service is running, let's Watch it
      // to ensure it stays alive for the duration of the test
      GError * error = NULL;
      GDBusConnection * bus_connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
      indicator_service_proxy = g_dbus_proxy_new_sync (bus_connection,
                                                       G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                                       NULL,
                                                       service_name.c_str(),
                                                       "/org/ayatana/indicator/service",
                                                       "org.ayatana.indicator.service",
                                                       NULL,
                                                       &error);
      GVariant * result = g_dbus_proxy_call_sync (indicator_service_proxy, "Watch",
                                                  NULL, G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                                  -1, NULL, &error);
      guint a, b;
      g_variant_get (result, "(uu)", &a, &b);
      g_debug ("Sending 'Watch' to proxy %p yielded %u %u", indicator_service_proxy, a, b);
      g_variant_unref (result);
      EXPECT_EQ(NULL, error);
      if (error != NULL) {
        g_message ("%s Unable to Watch indicator-service : %s", G_STRFUNC, error->message);
        g_clear_error (&error);
      }
      g_object_unref (G_OBJECT(bus_connection));

      menu_client = dbusmenu_client_new (service_name.c_str(), menu_object_path.c_str());
      menu_helper = new DbusmenuClientHelper (menu_client);

      // wait for a "layout-updated" signal before we let SetUp finish
      wait_for_layout_update();
    }

    virtual void
    TearDown()
    {
      ASSERT_EQ (handler_id, 0);

      // tear down the mainloop
      ASSERT_TRUE (mainloop != NULL);
      g_main_loop_unref (mainloop);
      mainloop = NULL;

      // tear down the menu client
      if (menu_helper != NULL) {
        delete menu_helper;
        menu_helper = NULL;
      }
      if (menu_client != NULL) {
        g_object_unref(G_OBJECT(menu_client));
        menu_client = NULL;
      }

      // tear down the indicator proxy
      EXPECT_TRUE (G_IS_DBUS_PROXY(indicator_service_proxy));
      g_object_unref (G_OBJECT(indicator_service_proxy));
      indicator_service_proxy = NULL;
    }

    void wait_for_layout_update()
    {
      ASSERT_EQ (handler_id, 0ul);
      handler_id = g_signal_connect (menu_client,
                                     DBUSMENU_CLIENT_SIGNAL_LAYOUT_UPDATED,
                                     G_CALLBACK(on_layout_updated_static),
                                     this);
      g_debug ("waiting for layout update...");
      g_main_loop_run (mainloop);
    }

    void wait_seconds (int seconds)
    {
      ASSERT_EQ (handler_id, 0ul); 
      handler_id = g_timeout_add_seconds (seconds, on_timeout_static, this);
      g_debug ("waiting %d seconds...", seconds);
      g_main_loop_run (mainloop);
    }

  protected:

    DbusmenuClient * menu_client;
    DbusmenuClientHelper * menu_helper;

  private:

    void ready()
    {
      g_debug("done waiting");
      g_main_loop_quit (mainloop);
    }

    GMainLoop * mainloop;
    DbusTestService * test_service;
    GDBusProxy * indicator_service_proxy;
    gulong handler_id;
    const std::string executable;
    const std::string service_name;
    const std::string menu_object_path;
};

#endif // #ifndef INDICATOR_SERVICE_TEST_H
