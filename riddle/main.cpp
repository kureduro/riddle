#include "code_generator.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "cxxopts.hpp"
#pragma GCC diagnostic pop
#include "fmt/color.h"
#include "fmt/core.h"
#include "lexer.hpp"
#include "parser.hpp"
#include "san.hpp"
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
    cxxopts::Options options("riddle", "A compiler for I-lang");
    options.add_options() // Define the options
        ("f,file", "File name", cxxopts::value<std::string>()) // source code
        ("o,out", "Output file name",
         cxxopts::value<std::string>()->default_value("a.out"),
         "<file name>") // output file
        // Verbosity of 0 prints only errors, 1 prints status of the various
        //  stages, 2 prints the stage output (if any), and 3 prints the source
        //  code after the different modification steps.
        ("v,verbosity", "Level of output verbosity (0-3)",
         cxxopts::value<int>()->default_value("0")->implicit_value("1")) //
        ("keep-temp", "Keep temporary files created during compilation",
         cxxopts::value<bool>()->default_value("false")) //
        ("h,help", "Print usage")                        // help
        ;
    options.parse_positional("file");
    options.positional_help("<path to file>");

    auto result = options.parse(argc, argv);

    if (result.count("help") > 0) {
        fmt::print("{}\n", options.help());
        return 0;
    }

    if (result.count("file") != 1) {
        fmt::print(fg(fmt::color::indian_red),
                   "Error: expected 1 input file, got {}\n",
                   result.count("file"));
        return 1;
    }

    auto path = result["file"].as<std::string>();
    auto verbosity = result["verbosity"].as<int>();
    auto outFile = result["out"].as<std::string>();
    auto keepTemp = result["keep-temp"].as<bool>();

    std::ifstream f(path);
    std::string code((std::istreambuf_iterator<char>(f)),
                     (std::istreambuf_iterator<char>()));

    if (verbosity > 2) {
        fmt::print("Input:\n");
        fmt::print(fg(fmt::color::aqua), "{}\n\n", code);
    }

    // ----- Parse program -----
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
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Parsing: ");
        fmt::print(fg(fmt::color::green), "success!\n");
    }
    if (verbosity > 1) {
        san::AstPrinter astPrinter;
        ast->accept(astPrinter);
        fmt::print("\n");
    }
    if (verbosity > 2) {
        san::PrettyPrinter prettyPrinter;
        ast->accept(prettyPrinter);
        fmt::print("\n");
    }

    // ----- Resolve identifiers to their declarations -----
    san::IdentifierResolver idResolver;
    ast->accept(idResolver);
    errors = idResolver.getErrors();
    if (!errors.empty()) {
        fmt::print(fg(fmt::color::indian_red) | fmt::emphasis::bold,
                   "Identifier resolution errors:\n");
        printErrors(code, errors);
        return 1;
    }
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Identifier resolution: ");
        fmt::print(fg(fmt::color::green), "success!\n");
    }
    if (verbosity > 2) {
        san::PrettyPrinter prettyPrinter;
        ast->accept(prettyPrinter);
        fmt::print("\n");
    }

    // -- Check that all array types have the length defined if not in params --
    san::ArrayLengthEnforcer arrLenEnforcer;
    ast->accept(arrLenEnforcer);
    errors = arrLenEnforcer.getErrors();
    if (!errors.empty()) {
        fmt::print(fg(fmt::color::indian_red) | fmt::emphasis::bold,
                   "Errors:\n");
        printErrors(code, errors);
        return 1;
    }
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Array length defined: ");
        fmt::print(fmt::fg(fmt::color::green), "pass\n");
    }

    // ----- Check that a function with return type always returns -----
    san::MissingReturn missingReturn;
    ast->accept(missingReturn);
    errors = missingReturn.getErrors();
    if (!errors.empty()) {
        fmt::print(fmt::fg(fmt::color::indian_red), "Missing return errors:\n");
        printErrors(code, errors);
        return 1;
    }
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Missing return: ");
        fmt::print(fmt::fg(fmt::color::green), "pass\n");
    }

    // --- Check if amount of params is equal to routine's amount of params ---
    san::ParamsValidator paramsValidator;
    ast->accept(paramsValidator);
    errors = paramsValidator.getErrors();
    if (!errors.empty()) {
        fmt::print(fg(fmt::color::indian_red) | fmt::emphasis::bold,
                   "Errors:\n");
        printErrors(code, errors);
        return 1;
    }
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Routine call arguments: ");
        fmt::print(fmt::fg(fmt::color::green), "pass\n");
    }

    // ----- Check for types conformance and fill expression types -----
    san::TypeDeriver deriveType;
    ast->accept(deriveType);
    errors = deriveType.getErrors();
    if (!errors.empty()) {
        fmt::print(fmt::fg(fmt::color::indian_red), "Type conformance: fail\n");
        printErrors(code, errors);
        return 1;
    }
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Type conformance/inference: ");
        fmt::print(fmt::fg(fmt::color::green), "success\n");
    }
    if (verbosity > 2) {
        san::PrettyPrinter prettyPrinter;
        ast->accept(prettyPrinter);
        fmt::print("\n");
    }

    // ----- Generate LLVM IR and executable -----
    cg::CodeGenerator codeGen(path);
    ast->accept(codeGen);
    errors = codeGen.getErrors();
    if (!errors.empty()) {
        fmt::print("Code generation errors:\n");
        printErrors(code, errors);
    }
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Code generation: ");
        fmt::print(fmt::fg(fmt::color::green), "success\n");
    }
    if (verbosity > 1) {
        fmt::print("Generated code:\n");
        std::string buffer;
        llvm::raw_string_ostream ir_ss(buffer);
        codeGen.print(ir_ss);
        fmt::print(fg(fmt::color::aqua), "{}\n", ir_ss.str());
    }

    std::string tempCppFileName = "_temp_.cpp";
    std::string tempObjFileName = "_temp_output_.o";
    codeGen.emitCode(tempObjFileName);
    cg::generateIntermediateCpp(tempCppFileName, codeGen);
    system(fmt::format("clang++ {} {} -o {}", tempCppFileName, tempObjFileName,
                       outFile)
               .c_str());
    if (!keepTemp) {
        system(
            fmt::format("rm {} {}", tempCppFileName, tempObjFileName).c_str());
    }
    if (verbosity > 0) {
        fmt::print(fmt::emphasis::bold, "Executable: creation: ");
        fmt::print(fmt::fg(fmt::color::green), "success\n");
    }

    return 0;
}
