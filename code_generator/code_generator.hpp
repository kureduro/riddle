#include "ast.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#pragma GCC diagnostic pop
#include <fstream>

namespace cg {

class CodeGenerator : public ast::Visitor {
public:
    CodeGenerator(std::string name = "anonymous");
    void visit(ast::Program* node) override;
    void visit(ast::RoutineDecl* node) override;
    void visit(ast::AliasedType* node) override;
    void visit(ast::IntegerType* node) override;
    void visit(ast::RealType* node) override;
    void visit(ast::BooleanType* node) override;
    void visit(ast::ArrayType* node) override;
    void visit(ast::RecordType* node) override;
    void visit(ast::VariableDecl* node) override;
    void visit(ast::TypeDecl* node) override;
    void visit(ast::Body* node) override;
    void visit(ast::ReturnStatement* node) override;
    void visit(ast::Assignment* node) override;
    void visit(ast::WhileLoop* node) override;
    void visit(ast::ForLoop* node) override;
    void visit(ast::IfStatement* node) override;
    void visit(ast::UnaryExpression* node) override;
    void visit(ast::BinaryExpression* node) override;
    void visit(ast::IntegerLiteral* node) override;
    void visit(ast::RealLiteral* node) override;
    void visit(ast::BooleanLiteral* node) override;
    void visit(ast::Identifier* node) override;
    void visit(ast::RoutineCall* node) override;

    void print(llvm::raw_ostream& stream = llvm::errs()) {
        m_module->print(stream, nullptr);
    }
    void emitCode(std::string filename = "output.o");
    llvm::Module::FunctionListType& getFunctions();

private:
    llvm::LLVMContext m_context;
    llvm::IRBuilder<> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string, llvm::Value*> m_namedValues;
    llvm::TargetMachine* m_targetMachine;

    // used for holding values that should be returned by functions
    llvm::Value* tempVal = nullptr;
    llvm::Value* extractTempVal() {
        llvm::Value* t = tempVal;
        tempVal = nullptr;
        return t;
    }
    llvm::Type* tempType = nullptr;
    llvm::Type* extractTempType() {
        llvm::Type* t = tempType;
        tempType = nullptr;
        return t;
    }

    bool anyIsFloat(std::initializer_list<llvm::Value*> values) {
        for (auto& val : values) {
            if (val->getType()->isFloatingPointTy()) {
                return true;
            }
        }
        return false;
    }
};

void generateIntermediateCpp(std::string, CodeGenerator&);

} // namespace cg
