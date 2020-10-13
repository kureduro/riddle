#include "code_generator.hpp"
#include "fmt/core.h"

std::string typeToStr(const llvm::Type* type) {
    if (type->isIntegerTy(1)) {
        return "bool";
    }
    if (type->isIntegerTy()) {
        return "uint64_t";
    }
    if (type->isFloatingPointTy()) {
        return "double";
    }
    return "void*";
}

std::string getParser(const llvm::Type* type) {
    if (type->isIntegerTy(1)) {
        return "toBool";
    }
    if (type->isIntegerTy()) {
        return "std::stoll";
    }
    if (type->isFloatingPointTy()) {
        return "std::stod";
    }
    return "";
}

namespace cg {

void generateIntermediateCpp(std::string filename, CodeGenerator& codegen) {
    std::FILE* file = std::fopen(filename.c_str(), "w");
    // Headers
    fmt::print(file, "#include <iostream>\n");
    fmt::print(file, "#include <sstream>\n");
    fmt::print(file, "#include <cstring>\n");
    fmt::print(file, "\n");

    // Custom functions
    fmt::print(file, "bool toBool(std::string s) {{\n");
    fmt::print(
        file,
        "\tbool b; std::istringstream(s) >> std::boolalpha >> b; return b;\n");
    fmt::print(file, "}}\n");
    fmt::print(file, "\n");

    // Extern C function declarations
    fmt::print(file, "extern \"C\" {{\n");
    for (auto& func : codegen.getFunctions()) {
        if (func.getName().startswith("llvm")) {
            continue;
        }
        // return type
        fmt::print(file, "{} ", typeToStr(func.getReturnType()));
        // function name
        fmt::print(file, "{}(", func.getName().str());
        // parameters
        auto args = func.args();
        for (auto it = args.begin(); it != args.end();) {
            fmt::print(file, "{}", typeToStr(it->getType()));
            it++;
            if (it != args.end()) {
                fmt::print(file, ", ");
            }
        }
        fmt::print(file, ");\n");
    }
    fmt::print(file, "}}\n\n");

    // main function
    fmt::print(file, "int main(int argc, char* argv[]) {{\n");
    fmt::print(file, "\tif (argc < 2) {{ std::cerr << \"Expected at least one"
                     " argument\\n\"; return 1; }}\n\n");

    for (auto& func : codegen.getFunctions()) {
        if (func.getName().startswith("llvm")) {
            continue;
        }
        fmt::print(file, "\tif (strcmp(argv[1], \"{}\") == 0) {{\n",
                   func.getName().str());
        fmt::print(file,
                   "\t\tif (argc != {}) {{ std::cerr << \"Unexpected number"
                   " of arguments\\n\"; return 2; }}\n",
                   func.arg_size() + 2);
        fmt::print(file, "\t\tstd::cout << {}(", func.getName().str());

        for (unsigned int i = 0; i < func.arg_size(); i++) {
            fmt::print(file, "{}(argv[{}])",
                       getParser(func.getArg(i)->getType()), i + 2);
            if (i + 1 < func.arg_size()) {
                fmt::print(file, ", ");
            }
        }
        fmt::print(file, ") << std::endl;\n", func.getName().str());
        fmt::print(file, "\t}}\n");
    }
    fmt::print(file, "\treturn 0;\n");
    fmt::print(file, "}}\n");

    std::fclose(file);
}

} // namespace cg
