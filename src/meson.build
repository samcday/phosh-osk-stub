subdir('dbus')
subdir('layouts')

libpos_enum_headers = files(['pos-enums.h'])
libpos_enum_sources = gnome.mkenums_simple(
  'pos-enum-types',
  sources : libpos_enum_headers)

libpos_deps = [
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

pos_resources = gnome.compile_resources(
  'pos-resources',
  'phosh-osk-stub.gresources.xml',
  extra_args: '--manual-register',
  c_name: 'pos',
)

libpos_sources = files(
  'pos-char-popup.h',
  'pos-char-popup.c',
  'pos-enums.h',
  'pos-main.c',
  'pos-main.h',
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
)

libpos_generated_sources = [
  generated_dbus_sources,
  libpos_enum_sources,
  pos_resources,
  wl_proto_headers,
  wl_proto_sources,
]

# copied verbatim from phoh
phosh_contrib_sources = files(
  'contrib/layersurface.h',
  'contrib/layersurface.c',
  'contrib/util.h',
  'contrib/util.c',
)

pos_inc = ['.', 'contrib/' ]
pos_includes = [pos_inc, root_inc, dbus_inc, proto_inc]

pos_lib = static_library(
  'pos',
  libpos_sources,
  libpos_generated_sources,
  phosh_contrib_sources,
  dependencies: libpos_deps,
  include_directories: pos_includes,
  install: false)

libpos_dep = declare_dependency(
  include_directories: pos_includes,
  link_with: pos_lib,
  dependencies: libpos_deps)

executable('phosh-osk-stub',
	   'phosh-osk-stub.c',
	   'pos-input-surface.h',
	   'pos-input-surface.c',
           include_directories: pos_includes,
           install: true,
           dependencies: libpos_dep)

if get_option('gtk_doc')
  pos_gir_extra_args = [
    '--quiet',
   ]

  pos_gir = gnome.generate_gir(pos_lib,
                 sources: libpos_sources,
               nsversion: '0',
               namespace: 'Pos',
         export_packages: 'pos-0',
           symbol_prefix: 'pos',
       identifier_prefix: 'Pos',
                includes: ['Gio-2.0', 'Gtk-3.0', 'Handy-1'],
                 install: false,
              extra_args: pos_gir_extra_args,
            dependencies: libpos_dep,
   )
endif
