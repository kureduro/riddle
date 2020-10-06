incdir = include_directories('.')
libparser = static_library('parser', 
                           sources : ['impl/parser.cpp'],
                           cpp_args : riddle_cpp_args,
                           c_args : riddle_c_args,
                           link_args : riddle_link_args,
                           dependencies : [ fmt_dep, lexer_dep, common_dep, ast_dep ],
                           install : true)


parser_dep = declare_dependency(include_directories : incdir,
                                link_with : libparser)
