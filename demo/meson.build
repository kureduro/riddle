executable('lexer_demo', 'lexer_demo.cpp',
          cpp_args : riddle_cpp_args,
          c_args : riddle_c_args,
          link_args : riddle_link_args,
          dependencies : [ lexer_dep, common_dep, fmt_dep ])

executable('parser_demo', 'parser_demo.cpp',
          cpp_args : riddle_cpp_args,
          c_args : riddle_c_args,
          link_args : riddle_link_args,
          dependencies : [ san_dep, parser_dep, ast_dep, lexer_dep, common_dep, fmt_dep ])
