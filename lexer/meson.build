incdir = include_directories('.')
liblexer = static_library('lexer', 
                         sources : ['impl/lexer.cpp', 'impl/token.cpp'], 
                         cpp_args : riddle_cpp_args,
                         c_args : riddle_c_args,
                         link_args : riddle_link_args,
                         dependencies : [ fmt_dep, common_dep ],
                         install : true)


lexer_dep = declare_dependency(include_directories : '.',
                               link_with : liblexer)
