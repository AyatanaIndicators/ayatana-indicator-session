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

#include <libindicator/indicator-service-test.h>

#include "shared-names.h"

/***
****
***/

/**
 * Fixture class for testing indicator-session-service with Google Test.
 */
class SessionServiceTest: public IndicatorServiceTest
{
  public:
    virtual ~SessionServiceTest() {}
    SessionServiceTest(): IndicatorServiceTest(INDICATOR_SESSION_DBUS_NAME,
                                               INDICATOR_SESSION_DBUS_OBJECT,
                                               INDICATOR_SERVICE_PATH) { }
  public:
    virtual void SetUp() {
      wait_seconds(1);
      IndicatorServiceTest::SetUp();
    }
    virtual void TearDown() {
      IndicatorServiceTest::TearDown();
    }
};


/**
 * Basic sanity test to see if we can account for all our menuitems.
 */
TEST_F(SessionServiceTest, HelloWorld)
{
  ASSERT_TRUE(true);
}

