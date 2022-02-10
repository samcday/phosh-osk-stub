json_glib_utils = find_program('json-glib-validate', required: false)

layouts = files(
  'ara.json',
  'bg.json',
  'cz.json',
  'de.json',
  'in+bolnagri.json',
  'in+mal.json',
  'pl.json',
  'ru.json',
  'us.json',
)

if json_glib_utils.found()
  foreach layout : layouts
    test('Validate layout @0@'.format(layout),
	 json_glib_utils, args: layout)
  endforeach		 
endif

