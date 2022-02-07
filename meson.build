project('phosh', 'c',
          version: '0.0.1',
          license: 'GPLv3+',
    meson_version: '>= 0.52.0',
  default_options: [ 'warning_level=1', 'buildtype=debugoptimized', 'c_std=gnu11' ],
)

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))
libdir = join_paths(prefix, get_option('libdir'))
libexecdir = join_paths(prefix, get_option('libexecdir'))
desktopdir = join_paths(datadir, 'applications')
sessiondir = join_paths(datadir, 'gnome-session')
pkgdatadir = join_paths(datadir, meson.project_name())
pkglibdir = join_paths(libdir, meson.project_name())
systemddir = join_paths(prefix, 'lib/systemd')
systemduserdir = join_paths(systemddir, 'user')

glib_ver = 'GLIB_VERSION_2_62'

add_project_arguments([
  '-DHAVE_CONFIG_H',
  '-DGLIB_VERSION_MIN_REQUIRED=@0@'.format(glib_ver),
  '-DGLIB_VERSION_MAX_REQUIRED=@0@'.format(glib_ver),
  '-DG_LOG_USE_STRUCTURED',
], language: 'c')

root_inc = include_directories('.')

cc = meson.get_compiler('c')

global_c_args = []
test_c_args = [
  '-Wcast-align',
  '-Wdate-time',
  '-Wdeclaration-after-statement',
  ['-Werror=format-security', '-Werror=format=2'],
  '-Wendif-labels',
  '-Werror=incompatible-pointer-types',
  '-Werror=missing-declarations',
  '-Werror=overflow',
  '-Werror=return-type',
  '-Werror=shift-count-overflow',
  '-Werror=shift-overflow=2',
  '-Werror=implicit-fallthrough=3',
  '-Wfloat-equal',
  '-Wformat-nonliteral',
  '-Wformat-security',
  '-Winit-self',
  '-Wmaybe-uninitialized',
  '-Wmissing-field-initializers',
  '-Wmissing-include-dirs',
  '-Wmissing-noreturn',
  '-Wnested-externs',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-strict-aliasing',
  '-Wno-unused-parameter',
  '-Wold-style-definition',
  '-Wpointer-arith',
  '-Wredundant-decls',
  '-Wshadow',
  '-Wstrict-prototypes',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wtype-limits',
  '-Wundef',
  '-Wunused-function',
]
if get_option('buildtype') != 'plain'
  test_c_args += '-fstack-protector-strong'
endif

foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    global_c_args += arg
  endif
endforeach
add_project_arguments(
  global_c_args,
  language: 'c'
)

gnome = import('gnome')
i18n  = import('i18n')

glib_ver = '>= 2.62'

glib_dep = dependency('glib-2.0', version: glib_ver)
gio_dep = dependency('gio-2.0', version: glib_ver)
gobject_dep = dependency('gobject-2.0', version: glib_ver)
gnome_desktop_dep = dependency('gnome-desktop-3.0', version: '>=3.26')
gsettings_desktop_schemas_dep = dependency('gsettings-desktop-schemas')
gtk_dep = dependency('gtk+-3.0', version: '>=3.22')
gtk_wayland_dep = dependency('gtk+-wayland-3.0', version: '>=3.22')
json_glib_dep =  dependency('json-glib-1.0')
libfeedback_dep = dependency('libfeedback-0.0',
  fallback: ['libfeedback', 'libfeedback_dep'],
  default_options: ['introspection=disabled', 'daemon=false', 'gtk_doc=false']
  )
libhandy_dep = dependency('libhandy-1',
  version: '>=1.1.90',
  fallback: ['libhandy', 'libhandy_dep'],
  default_options: ['introspection=disabled']
			 )
libsystemd_dep = dependency('libsystemd', version: '>= 241')
wayland_client_dep = dependency('wayland-client', version: '>=1.14')
wayland_protos_dep = dependency('wayland-protocols', version: '>=1.12')
xkbcommon_dep = dependency('xkbcommon')

config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', 'phosh')
config_h.set_quoted('LOCALEDIR', localedir)
config_h.set_quoted('PHOSH_OSK_STUB_VERSION', meson.project_version())

configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: config_h,
)

subdir('data')
subdir('po')
subdir('protocol')
subdir('src')

