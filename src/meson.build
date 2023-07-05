subdir('dbus')
subdir('layouts')

libpos_enum_headers = files(['pos-enums.h', 'phosh-osk-enums.h'])
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
  'pos.h',
  'pos-activation-filter.h',
  'pos-activation-filter.c',
  'pos-char-popup.h',
  'pos-char-popup.c',
  'pos-completer.h',
  'pos-completer.c',
  'pos-completer-manager.h',
  'pos-completer-manager.c',
  'pos-completion-bar.h',
  'pos-completion-bar.c',
  'pos-debug-widget.h',
  'pos-debug-widget.c',
  'pos-emoji-picker.h',
  'pos-emoji-picker.c',
  'pos-enums.h',
  'pos-input-method.h',
  'pos-input-method.c',
  'pos-input-surface.h',
  'pos-input-surface.c',
  'pos-main.c',
  'pos-main.h',
  'pos-osk-dbus.h',
  'pos-osk-dbus.c',
  'pos-osk-key.h',
  'pos-osk-key.c',
  'pos-osk-widget.h',
  'pos-osk-widget.c',
  'pos-shortcuts-bar.h',
  'pos-shortcuts-bar.c',
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

pos_inc = include_directories([ '.', 'contrib/' ])
pos_includes = [pos_inc, root_inc, dbus_inc, proto_inc]

subdir('completers')

pos_lib = static_library(
  'pos',
  libpos_sources,
  libpos_generated_sources,
  phosh_contrib_sources,
  dependencies: libpos_deps,
  include_directories: pos_includes,
  install: false)

libpos_dep = declare_dependency(
  sources: libpos_generated_sources,
  include_directories: pos_includes,
  link_with: pos_lib,
  dependencies: [libpos_deps,
		 libpos_completers_dep,
		])

executable('phosh-osk-stub',
	   'phosh-osk-stub.c',
           include_directories: pos_includes,
           install: true,
           dependencies: libpos_dep)

if get_option('gtk_doc')
  pos_gir_extra_args = [
    '--quiet',
  ]

  pos_gir = gnome.generate_gir([pos_lib,
                                libpos_completer_libs,
                               ],
                 sources: [libpos_sources, libpos_completers_sources],
               nsversion: '0',
               namespace: 'Pos',
         export_packages: 'pos-0',
           symbol_prefix: 'pos',
       identifier_prefix: 'Pos',
                includes: ['Gio-2.0', 'Gtk-3.0', 'Handy-1'],
                 install: false,
              extra_args: pos_gir_extra_args,
            dependencies: [libpos_dep, libpos_completers_dep],
   )
endif
