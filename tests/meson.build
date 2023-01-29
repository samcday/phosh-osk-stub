if get_option('tests')

test_env = environment()
test_env.set('G_DEBUG', 'gc-friendly,fatal-warnings')
test_env.set('MALLOC_CHECK_','2')
test_env.set('NO_AT_BRIDGE','1')

grab_word_test_sources = [
  'test-grab-word.c',
]

grab_word_test = executable('test-grab-word',
			    grab_word_test_sources,
			    pie: true,
			    dependencies : libpos_dep
)
test ('grab-word', grab_word_test, env: test_env)

endif
