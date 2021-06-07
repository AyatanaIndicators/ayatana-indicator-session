<!--
 Copyright (C) 2013 Canonical Ltd
 Copyright (C) 2017-2021, Mike Gabriel <mike.gabriel@das-netzwerkteam.de>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->

# Build and installation instructions

## Compile-time build dependencies

 - gettext (>= 0.18.1.1-10ubuntu3)
 - glib-2.0 (>= 2.35.4)
 - libayatana-common (>= 0.9.1)
 - cmake (>= 3.13)
 - gcovr (>= 2.4)
 - lcov (>= 1.9)
 - gtest (>= 1.6.0)
 - cppcheck

## Runtime DBus dependencies

 - org.ayatana.indicators.webcredentials
 - org.freedesktop.Accounts
 - org.freedesktop.Accounts.User
 - org.freedesktop.DisplayManager.Seat
 - org.freedesktop.login1.Manager
 - org.freedesktop.login1.Seat
 - org.freedesktop.login1.User
 - org.gnome.ScreenSaver
 - org.gnome.SessionManager
 - org.gnome.SessionManager.EndSessionDialog

## For end-users and packagers

```
cd ayatana-indicator-session-X.Y.Z
mkdir build
cd build
cmake ..
make
sudo make install
```

**The install prefix defaults to `/usr`, change it with `-DCMAKE_INSTALL_PREFIX=/some/path`**

## For testers - unit tests only

```
cd ayatana-indicator-session-X.Y.Z
mkdir build
cd build
cmake .. -DENABLE_TESTS=ON
make
make test
make cppcheck
```

## For testers - both unit tests and code coverage

```
cd ayatana-indicator-session-X.Y.Z
mkdir build-coverage
cd build-coverage
cmake .. -DENABLE_COVERAGE=ON
make
make coverage-html
```
