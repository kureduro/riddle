#include "fmt/color.h"
#include "fmt/core.h"
#include "lexer.hpp"
#include "parser.hpp"
#include "visitors.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

void printError(std::string line, ast::Error error) {
    fmt::print("*\t{}\n", line);
    fmt::print("\t{:->{}}^{:-<{}}\n", "", error.pos.column - 1, "",
               line.length() - error.pos.column - 1);
    fmt::print(fg(fmt::color::indian_red), "\t[line: {}, column: {}]: {}\n\n",
               error.pos.line, error.pos.column, error.message);
}

std::vector<std::string> splitLines(std::string code) {
    std::vector<std::string> lines;
    std::string line;
    std::istringstream codeStream(code);
    while (std::getline(codeStream, line, '\n')) {
        lines.push_back(line);
    }
    return lines;
}

void printErrors(std::string source_code, std::vector<ast::Error> errors) {
    auto lines = splitLines(source_code);
    for (auto error : errors) {
        printError(lines[error.pos.line - 1], error);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fmt::print(stderr, "riddle compiler\nUsage:\n"
                           "\triddle path/to/source.file\n\n");
        return 1;
    }

    std::ifstream f(argv[1]);
    std::string code((std::istreambuf_iterator<char>(f)),
                     (std::istreambuf_iterator<char>()));

    // TODO: hide behind option `--print-source`
    fmt::print("Code:\n");
    fmt::print(fg(fmt::color::aqua), "{}\n\n", code);

    lexer::Lexer lx{code};
    parser::Parser parser(lx);

    auto ast = parser.parseProgram();
    auto errors = parser.getErrors();

    if (!errors.empty()) {
        fmt::print(fg(fmt::color::indian_red) | fmt::emphasis::bold,
                   "Parsing Errors:\n");
        printErrors(code, errors);
        return 1;
    }
    fmt::print(fmt::emphasis::bold, "Parsing: ");
    fmt::print(fg(fmt::color::green), "success!\n\n");

    // TODO: hide behind option `--print-ast`
    // Print the AST
    //san::AstPrinter astPrinter;
    //ast->accept(astPrinter);

    san::PrettyPrinter prettyPrinter;
    ast->accept(prettyPrinter);

    // Resolve identifiers to their declarations
    san::IdentifierResolver idResolver;
    ast->accept(idResolver);
    errors = idResolver.getErrors();

    if (!errors.empty()) {
        fmt::print(fg(fmt::color::indian_red) | fmt::emphasis::bold,
                   "Identifier resolution Errors:\n");
        printErrors(code, errors);
        return 1;
    }

    fmt::print(fmt::emphasis::bold, "\nIdentifier resolution: ");
    fmt::print(fg(fmt::color::green), "success!\n\n");

    /*
    // Print again after tree modification
    fmt::print(fg(fmt::color::gold), "\nNew tree:\n\n");
    ast->accept(astPrinter);
    */

    // TODO: Remove
    ast->accept(prettyPrinter);

    return 0;
}
