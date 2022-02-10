subdir('dbus')
subdir('layouts')

phosh_osk_stub_enum_headers = files(['pos-enums.h'])
phosh_osk_stub_enum_sources = gnome.mkenums_simple(
  'pos-enum-types',
  sources : phosh_osk_stub_enum_headers)

osk_stub_deps = [
  gio_dep,
  gnome_desktop_dep,
  gtk_dep,
  gtk_wayland_dep,
  json_glib_dep,
  libfeedback_dep,
  libhandy_dep,
  libsystemd_dep,
  wayland_client_dep,
  xkbcommon_dep,
  cc.find_library('m', required: false),
  cc.find_library('rt', required: false),
]

phosh_osk_stub_resources = gnome.compile_resources(
  'phosh-osk-stub-resources',
  'phosh-osk-stub.gresources.xml',

  c_name: 'pos',
)

# copied verbatim from phoh
phosh_contrib_sources = files(
  'contrib/layersurface.h',
  'contrib/layersurface.c',
  'contrib/util.h',
  'contrib/util.c',
)

pos_inc = ['.', 'contrib/' ]

executable('phosh-osk-stub', ['phosh-osk-stub.c',
			      'pos-char-popup.h',
			      'pos-char-popup.c',
			      'pos-input-surface.h',
			      'pos-input-surface.c',
			      'pos-osk-dbus.h',
			      'pos-osk-dbus.c',
			      'pos-osk-key.h',
			      'pos-osk-key.c',
			      'pos-osk-widget.h',
			      'pos-osk-widget.c',
			      'pos-vk-driver.h',
			      'pos-vk-driver.c',
			      'pos-virtual-keyboard.h',
			      'pos-virtual-keyboard.c',
			     ] +
			     generated_dbus_sources +
			     phosh_osk_stub_enum_sources +
			     phosh_osk_stub_resources +
			     phosh_contrib_sources +
			     wl_proto_sources,
           include_directories: [root_inc, pos_inc],
           install: true,
           dependencies: osk_stub_deps)
