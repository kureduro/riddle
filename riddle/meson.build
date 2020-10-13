cxxopts_dep = dependency('cxxopts', fallback : ['cxxopts', 'cxxopts_dep'])

executable('riddle', 
          sources : [ './main.cpp' ],
          cpp_args : riddle_cpp_args,
          c_args : riddle_c_args,
          link_args : riddle_link_args,
          dependencies : [ cxxopts_dep, cg_dep, llvm_dep, san_dep, parser_dep, ast_dep, fmt_dep, lexer_dep, common_dep ])
