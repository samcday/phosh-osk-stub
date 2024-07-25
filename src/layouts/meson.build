json_glib_utils = find_program('json-glib-validate', required: false)

layouts = [
  'ara.json',
  'be.json',
  'bg.json',
  'by.json',
  'ca.json',
  'cz.json',
  'de.json',
  'dk.json',
  'ee.json',
  'es.json',
  'fi.json',
  'fr.json',
  'ge.json',
  'gr.json',
  'hr.json',
  'il.json',
  'in+bolnagri.json',
  'in+mal.json',
  'it.json',
  'lt.json',
  'lv.json',
  'my.json',
  'nl.json',
  'no.json',
  'ph.json',
  'pl.json',
  'pt.json',
  'ro.json',
  'rs.json',
  'ru.json',
  'se.json',
  'si.json',
  'sk.json',
  'tr.json',
  'ua.json',
  'uk.json',
  'us.json',
  'vn.json',
  'za.json',
  # custom lauouts
  'terminal.json',
]

if json_glib_utils.found()
  foreach layout : layouts
    test('Validate layout @0@'.format(layout),
	 json_glib_utils, args: meson.current_source_dir() / layout)
  endforeach		 
endif


