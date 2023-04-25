.. _phosh-osk-stub(1):

==============
phosh-osk-stub
==============

----------------------------------------------
An (experimental) on screen keyboard for Phosh
----------------------------------------------

SYNOPSIS
--------
|   **phosh-osk-stub** [OPTIONS...]


DESCRIPTION
-----------

``phosh-osk-stub`` is an on screen keyboard (OSK) for phosh. It is
considered experimental. For a production ready on screen keyboard see
``squeekboard(1)``.

OPTIONS
-------

``-h``, ``--help``
   Print help and exit

``--replace``
   Try to grab the `sm.puri.OSK0` DBus interface, even if it is
   already in use by another keyboard.

``--allow-replacement``
   (Temporarily) Give up the `sm.puri.OSK0` DBus name if another OSK
   requests it. This also unregisters the Wayland input-method-v1 so another
   OSK can act as input method.
   If the name becomes available again it is grabbed again and phosh-osk-stub
   registers itself as input method again.

``-v``, ``--version``
   Show version information


SETUP
-----

In order to be used by Phosh as OSK, phosh-osk-stub needs to be started from
the `/usr/share/applications/sm.puri.OSK0.desktop` desktop file. On Debian
systems this can be achieved via

::

   update-alternatives --config Phosh-OSK

CONFIGURATION
-------------
``phosh-osk-stub`` is configured via ``GSettings``. This includes
configuration of the loaded layouts from
``org.gnome.desktop.input-sources`` via the ``sources`` and
``xkb-options`` keys, whether the OSK is enabled at all via the
``org.gnome.desktop.a11y.applications``'s ``screen-keyboard-enabled`` and
configuration of word completion (see below).

WORD COMPLETION
^^^^^^^^^^^^^^^

``phosh-osk-stub`` has *experimental* support for word completion based on the
`presage` library. It has several modes of operation represented by flags that
can be combined:

- `off`: no completion
- `manual`: enable and disable completion via an option in the language popover
- `hint`: enables and disables completion based on the text input's `completion`
  hint.

Valid settings are `off`, `manual`, `hint` and `manual+hint`. These can be
enabled configured via the `gsettings` command:

::

  # off
  gsettings set sm.puri.phosh.osk completion-mode "[]"
  # manual
  gsettings set sm.puri.phosh.osk completion-mode "['manual']"
  # hint
  gsettings set sm.puri.phosh.osk completion-mode "['hint']"
  # manual+hint
  gsettings set sm.puri.phosh.osk completion-mode "['manual','hint']"
  # Reset to default (off)
  gsettings reset sm.puri.phosh.osk completion-mode

TERMINAL SHORTCUTS
^^^^^^^^^^^^^^^^^^
``phosh-osk-stub`` can provide a row of keyboard shortcuts on the
terminal layout. These are configured via the ``shortcuts`` GSetting

::

  gsettings set sm.puri.phosh.osk.Terminal shortcuts "['<ctrl>a', '<ctrl>e', '<ctrl>r']"

For valid values see documentation of `gtk_accelerator_parse()`: https://docs.gtk.org/gtk3/func.accelerator_parse.html

ENVIRONMENT VARIABLES
---------------------

``phosh-osk-stub`` honors the following environment variables for debugging purposes:

- ``POS_DEBUG``: A comma separated list of flags:

  - ``force-show``: Ignore the `screen-keyboard-enabled` GSetting and always enable the OSK. This
    GSetting is usually managed by the user and Phosh.
  - ``force-completion``: Force text completion to ignoring the `completion-mode` GSetting.
- ``POS_TEST_LAYOUT``: Load the given layout instead of the ones configured via GSetting.
- ``POS_TEST_COMPLETER``: Use the given completer instead of the configured ones.
  The available values depend on how phosh-osk-stub was built. Available are currently at most

  - ``presage``: default completer based on presarge libarary
  - ``fzf``: Completer based on fzf command line tool (only useful for experiments
- ``G_MESSAGES_DEBUG``, ``G_DEBUG`` and other environment variables supported
  by glib. https://docs.gtk.org/glib/running.html
- ``GTK_DEBUG`` and other environment variables supported by GTK, see
  https://docs.gtk.org/gtk3/running.html

See also
--------

``phosh(1)`` ``squeekboard(1)`` ``text2ngram(1)`` ``gsettings(1)``
