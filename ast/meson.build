incdir = include_directories('.')
libast = static_library('ast',
                        include_directories : incdir,
                        cpp_args : riddle_cpp_args,
                        c_args : riddle_c_args,
                        link_args : riddle_link_args,
                        dependencies: [ lexer_dep ],
                        install : true)

ast_dep = declare_dependency(include_directories : incdir,
                             link_with : libast)
