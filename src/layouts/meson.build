json_glib_utils = find_program('json-glib-validate', required: false)

layouts = files(
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
  'ru.json',
  'se.json',
  'ua.json',
  'us.json',
  'vn.json',
  # custom lauouts
  'terminal.json',
)

if json_glib_utils.found()
  foreach layout : layouts
    test('Validate layout @0@'.format(layout),
	 json_glib_utils, args: layout)
  endforeach		 
endif

