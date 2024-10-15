foreach completer: completers
  i18n.merge_file(
    input: completer + '.completer.in',
    output: completer + '.completer',
    po_dir: '../po',
    install: true,
    install_dir: pkgdatadir / 'completers',
    type: 'desktop',
  )
endforeach
