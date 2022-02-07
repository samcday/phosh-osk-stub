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

