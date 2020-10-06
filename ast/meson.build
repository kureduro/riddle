incdir = include_directories('.')
libast = static_library('ast',
                        include_directories : incdir,
                        cpp_args : riddle_cpp_args,
                        c_args : riddle_c_args,
                        link_args : riddle_link_args,
                        dependencies: [ lexer_dep ],
                        install : true)

ast_dep = declare_dependency(include_directories : '.',
                             link_with : libast)


libvisitors = static_library('visitors', 
                             sources : [
                                 'impl/ast_printer.cpp',
                                 'impl/identifier_resolver.cpp',
                                 'impl/pretty_printer.cpp',
                             ],
                             cpp_args : riddle_cpp_args,
                             c_args : riddle_c_args,
                             link_args : riddle_link_args,
                             dependencies : [ ast_dep, lexer_dep, fmt_dep, common_dep ],
                             install : true)

visitors_dep = declare_dependency(include_directories : '.',
                               link_with : libvisitors)
