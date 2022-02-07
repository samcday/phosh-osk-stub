# DBus client interfaces
generated_dbus_sources = []

phosh_osk_dbus_sources = files('sm.puri.OSK0.xml')
generated_dbus_sources = gnome.gdbus_codegen('pos-osk0-dbus',
					     phosh_osk_dbus_sources,
					     interface_prefix: 'sm.puri.',
					     namespace: 'PosDbus')

