libpos_completer_includes = include_directories('.')

# Presage based completer
libpos_completer_presage_sources = files(
  'pos-completer-presage.h',
  'pos-completer-presage.c',
)

libpos_completer_presage_deps = [
  gio_dep,
  glib_dep,
  gtk_dep,
  presage_dep,
]

libpos_completer_presage_lib = static_library(
  'pos-completer-presage',
  libpos_completer_presage_sources,
  include_directories: pos_includes,
  install: false,
  dependencies: libpos_completer_presage_deps,
  override_options : ['cpp_std=c++11'])

libpos_completer_presage_dep = declare_dependency(
  include_directories: libpos_completer_includes,
  link_with: libpos_completer_presage_lib,
)

if fzf.found()
  #  fzf based completer
  libpos_completer_fzf_sources = files(
    'pos-completer-fzf.h',
    'pos-completer-fzf.c',
  )

  libpos_completer_fzf_deps = [
    gio_dep,
    glib_dep,
    gtk_dep,
  ]

  libpos_completer_fzf_lib = static_library(
    'pos-completer-fzf',
    libpos_completer_fzf_sources,
    include_directories: pos_includes,
    install: false,
    dependencies: libpos_completer_fzf_deps)

  libpos_completer_fzf_dep = declare_dependency(
    include_directories: libpos_completer_includes,
    link_with: libpos_completer_fzf_lib,
  )
else
  libpos_completer_fzf_sources = []
  libpos_completer_fzf_lib = []
  libpos_completer_fzf_dep = declare_dependency()
endif

libpos_completers_sources = [
  libpos_completer_fzf_sources,
  libpos_completer_presage_sources,
]

libpos_completer_libs = [
  libpos_completer_fzf_lib,
  libpos_completer_presage_lib,
]

libpos_completers_dep = declare_dependency(
  dependencies: [
    libpos_completer_fzf_dep,
    libpos_completer_presage_dep,
  ]
)
