incdir = include_directories('.')
libsan = static_library('visitors',
                             sources : [
                                 'impl/ast_printer.cpp',
                                 'impl/identifier_resolver.cpp',
                                 'impl/pretty_printer.cpp',
                                 'impl/array_length_enforcer.cpp',
                                 'impl/expression_type_filler.cpp',
                             ],
                             cpp_args : riddle_cpp_args,
                             c_args : riddle_c_args,
                             link_args : riddle_link_args,
                             dependencies : [ ast_dep, lexer_dep, fmt_dep, common_dep ],
                             install : true)

san_dep = declare_dependency(include_directories : incdir,
                             link_with : libsan)
