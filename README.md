# Ayatana System Indicator &mdash; Session

The -session Ayatana System Indicator is the session menu indicator for
Unity7, MATE and Lomiri (optionally for others, e.g. XFCE, LXDE). Its
behavior and features are listed at
https://wiki.ayatana-indicators.org/AyatanaIndicatorSession

For instructions on building and running built-in tests, see the INSTALL.md file.

## Notes for Client Renderers

Ayatana Indicator Session has two custom menuitems: the Guest and User
switchers. As per the
https://wiki.ayatana-indicators.org/AyatanaIndicatorSession
specification, both need four visual components: (1) an Active Session
Mark, the user's (2) icon, (3) name, and (4) a Logged-In Mark.

### User menuitems have "x-ayatana-type" set to "indicator.user-menu-item"

   Their four visual components are determined by:

   1. You can test for the Action Session Mark by checking the action's state.
      The state is a dicionary whose "active-user" key yields the current
      session's owner's username. If it matches the username in this menuitem's
      "target" attribute, show the Active Session Mark.

   2. The icon is stored in the menuitem's "icon" attribute. If none is set,
      the client should use a fallback icon such as "avatar-default."

   3. The name is stored in the menuitem's "label" attribute.

   4. You can test for the Logged In Mark by checking the action's state.
      The state is a dictionary whose "logged-in-users" key will give
      an array of usernames. If the array contains the username in this
      menuitem's "target" attribute, show the Logged In Mark.

### The Guest switcher has "x-ayatana-type" set to "indicator.guest-menu-item" action

   Its four visual components are determined by:

   1. You can test for the Active Session Mark by checking the action's state.
      The state is a dictionary whose "is-active" key yields a boolean.
      If the boolean is true, show the Active Session Mark.

   2. The guest user should use a fallback icon such as "avatar-default."

   3. The name ("Guest") is stored in the menuitem's "label" attribute.

   4. You can test for the Logged In Mark by checking the action's state.
      The state is a dictionary whose "is-logged-in" key yields a boolean.
      If the boolean is true, show the Logged In Mark.
