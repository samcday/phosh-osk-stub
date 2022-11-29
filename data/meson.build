desktop_files = {
  'sm.puri.OSK0.desktop': true,
}

desktopconf = configuration_data()
desktopconf.set('bindir', bindir)

foreach desktop_file, install : desktop_files
  i18n.merge_file(
    input: configure_file(
      input: desktop_file + '.in.in',
      output: desktop_file + '.in',
      configuration: desktopconf
    ),
    output: desktop_file,
    po_dir: '../po',
    install: true,
    install_dir: desktopdir,
    type: 'desktop'
  )
endforeach

#generate XML enum definitions for GSettings schema
schema_enum_headers = files(
  '../src/phosh-osk-enums.h',
)
generate_enums_schema = gnome.mkenums('sm.puri.phosh.osk.enums.xml',
  sources: schema_enum_headers,
  comments: '<!-- @comment@ -->',
  fhead: '<schemalist>',
  vhead: '  <@type@ id="sm.puri.phosh.osk.@EnumName@">',
  vprod: '    <value nick="@valuenick@" value="@valuenum@"/>',
  vtail: '  </@type@>',
  ftail: '</schemalist>',
  install_header: true,
  install_dir: schemasdir
)

#workaround due to https://github.com/mesonbuild/meson/issues/1687
copy_schema = custom_target('copy-gschema-to-builddir',
  input: 'sm.puri.phosh.osk.gschema.xml',
  output: 'sm.puri.phosh.osk.gschema.xml',
  command: ['cp', '@INPUT@', '@OUTPUT@']
)
schemas = ['sm.puri.phosh.osk.gschema.xml']
compile_schemas = custom_target('glib-compile-schemas',
  build_by_default: true,
  output: 'gschemas.compiled',
  install: false,
  command: [find_program('glib-compile-schemas'),
    meson.current_build_dir()
  ],
  depends: [generate_enums_schema, copy_schema]
)
install_data(
  schemas,
  install_dir: schemasdir
)

