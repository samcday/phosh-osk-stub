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


``phosh-osk-stub`` has two modes of operation. If the application
supports and uses ``text-input-unstable-v3`` the Wayland compositor
will use the ``input-method-unstable-v2`` protocol to interact with
the OSK. This allows it to

- automatically unfold the keyboard
- handle text prediction / correction via preedit and surrounding text
- automatically switch to special layouts like to terminal by input
  hints sent from the application

This is the preferred mode of operation. For legacy applications like
e.g. Electron applications the OSK falls back to a virtual keyboard mode
(basically emulating key presses). This means that it e.g. can't unfold
automatically or support completion. It will hence hide the completion bar
in that mode of operation.


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

For the keyboard to fold and unfold automatically make sure
``org.gnome.desktop.interface`` ``gtk-im-module`` is set to the empty string
(`''`).  This is the default in most distributions. If unsure check via:

::

  gsettings get org.gnome.desktop.interface gtk-im-module


WORD COMPLETION
^^^^^^^^^^^^^^^

``phosh-osk-stub`` has support for word completion via various
completers (see below). It has several modes of operation represented
by flags that can be combined:

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

Note that completion is always disabled when

- No usable completers are found on startup
- Terminal or emoji layout is in use
- The application doesn't support text-input so ``phosh-osk-stub`` is
  falling back virtual-keyboard mode.


AVAILABLE COMPLETERS
####################

The available completers depend on how ``phosh-osk-stub`` was
built. Available are currently at most

  - ``hunspell``: word correction based on the hunspell library
  - ``presage``: (experimental) word prediction based on the presage libarary
  - ``pipe``: completer using a pipe
  - ``fzf``: completer based on fzf command line tool. Useful for experiments)
  - ``varnam``: completer using govarnam for Indic languages

The default completer is selected via the
``sm.puri.phosh.osk.Completers`` ``default`` GSetting.

::

  gsettings set sm.puri.phosh.osk.Completers default hunspell

You need to restart ``phosh-osk-stub`` for the new default completer
to become active.


TEXT CORRECTION USING HUNSPELL
******************************

The hunspell completer needs dictionaries and affix files in
``/usr/share/hunspell`. Most importantly ``/usr/share/hunspell/en_US.dic``
and ``/usr/share/hunspell/en_US.aff`` are required as fallback when no
matching dictionary for the current layout is found.


TEXT COMPLETION USING PRESAGE
*****************************

The presage based completer is considered experimental as there are
some known issues when interacting with GTK4 applications.

For the presage based completer to work you need a model file in
`/usr/share/phosh/osk/presage/`. Likely your distribution already
ships one with the presage library. You can simply symlink it
there.  Models for more languages can be found in
https://gitlab.gnome.org/guidog/phosh-osk-data


TEXT COMPLETION USING PIPE
**************************

This completer feeds the current input word (preedit) to an executable
file and expects the executable to output possible completions on
stdout. The executable to invoke is configured via the
``sm.puri.phosh.osk.Completers.Pipe`` ``command`` GSetting. It defaults
to ``cat``. This can be used to experiment with different completion
patterns without having to modify ``phosh-osk-stub`` itself.

::

  gsettings set sm.puri.phosh.osk.Completers.Pipe command 'wc -c'

You need to restart ``phosh-osk-stub`` for the new command to become
active. A commonly used executable is swipeGuess: https://git.sr.ht/~earboxer/swipeGuess

TEXT COMPLETION USING VARNAM
****************************

This completer feeds the current input word (preedit) to govarnam.
Note that the completer is experimental and has Malayalam hardcoded.

For the completer to work it needs govarnam and the Malaylam schema files
installed. Please refer to the govarnam documentation.

TERMINAL SHORTCUTS
^^^^^^^^^^^^^^^^^^
``phosh-osk-stub`` can provide a row of keyboard shortcuts on the
terminal layout. These are configured via the ``shortcuts`` GSetting

::

  gsettings set sm.puri.phosh.osk.Terminal shortcuts "['<ctrl>a', '<ctrl>e', '<ctrl>r']"

For valid values see documentation of `gtk_accelerator_parse()`: https://docs.gtk.org/gtk3/func.accelerator_parse.html

IGNORING ACTIVATION
^^^^^^^^^^^^^^^^^^^
For some applications you might not want to unfold the OSK when the
application requests it. This can e.g. be useful when you usually read what
the application displays (and hence want to use as much as screen
space as possible) but the application focuses a text entry. By adding the
application's app-id to the ``ignore-activation`` list you can prevent the automatic
unfold. The OSK can still be unfolded by other means (e.g. via the DBus API or the OSK
button in Phosh). To determine an applications app-id you can use the
``foreign-toplevel`` command.

::

  gsettings set sm.puri.phosh.osk ignore-activation "['org.gnome.Calculator']"


ENVIRONMENT VARIABLES
---------------------

``phosh-osk-stub`` honors the following environment variables for debugging purposes:

- ``POS_DEBUG``: A comma separated list of flags:

  - ``force-show``: Ignore the `screen-keyboard-enabled` GSetting and always enable the OSK. This
    GSetting is usually managed by the user and Phosh.
  - ``force-completion``: Force text completion to ignoring the `completion-mode` GSetting.
- ``POS_TEST_LAYOUT``: Load the given layout instead of the ones configured via GSetting.
- ``POS_TEST_COMPLETER``: Use the given completer instead of the configured ones.
  The available values depend on how phosh-osk-stub was built (see above).
- ``G_MESSAGES_DEBUG``, ``G_DEBUG`` and other environment variables supported
  by glib. https://docs.gtk.org/glib/running.html
- ``GTK_DEBUG`` and other environment variables supported by GTK, see
  https://docs.gtk.org/gtk3/running.html

See also
--------

``phosh(1)`` ``squeekboard(1)`` ``text2ngram(1)`` ``gsettings(1)`` ``hunspell(5)``
