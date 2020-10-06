project('riddle',
        'cpp',
        default_options : ['cpp_std=gnu++17'])

riddle_cpp_args = [
  '-Wall',
  '-Wextra',
  '-Wconversion',
  '-Wcast-align',
  '-Wunused',
  '-Wno-missing-braces',
  '-Wno-missing-field-initializers',
  '-Wno-sign-conversion',
]

riddle_c_args = []
riddle_link_args = []


if get_option('always-sanitize-address') or get_option('buildtype') == 'debug'
  warning('Compiling with address sanitizer. Performance may be affected.')
  flags = [ '-fsanitize=address' ]
  riddle_cpp_args += flags
  riddle_c_args += flags
  riddle_link_args += flags
endif


fmt_dep = dependency('fmt', fallback : ['fmt', 'fmt_dep'])

subdir('common')
subdir('lexer')
subdir('ast')
subdir('parser')
subdir('san')

if get_option('build-tests')
  subdir('tests')
endif

if get_option('build-demos')
  subdir('demo')
endif

if get_option('build-compiler')
  subdir('riddle')
endif
