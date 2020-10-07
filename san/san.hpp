#include "ast.hpp"
#include <algorithm>
#include <memory>

namespace san {

template <typename T> using sPtr = std::shared_ptr<T>;

class AstPrinter : public ast::Visitor {
public:
    AstPrinter(size_t depth = 0) : m_depth(depth) {}
    void visit(ast::Program* node) override;
    void visit(ast::RoutineDecl* node) override;
    void visit(ast::Type* node) override;
    void visit(ast::AliasedType* node) override;
    void visit(ast::PrimitiveType* node) override;
    void visit(ast::IntegerType* node) override;
    void visit(ast::RealType* node) override;
    void visit(ast::BooleanType* node) override;
    void visit(ast::ArrayType* node) override;
    void visit(ast::RecordType* node) override;
    void visit(ast::VariableDecl* node) override;
    void visit(ast::TypeDecl* node) override;
    void visit(ast::Body* node) override;
    void visit(ast::Statement* node) override;
    void visit(ast::ReturnStatement* node) override;
    void visit(ast::Assignment* node) override;
    void visit(ast::WhileLoop* node) override;
    void visit(ast::ForLoop* node) override;
    void visit(ast::IfStatement* node) override;
    void visit(ast::Expression* node) override;
    void visit(ast::UnaryExpression* node) override;
    void visit(ast::BinaryExpression* node) override;
    void visit(ast::Primary* node) override;
    void visit(ast::IntegerLiteral* node) override;
    void visit(ast::RealLiteral* node) override;
    void visit(ast::BooleanLiteral* node) override;
    void visit(ast::Identifier* node) override;
    void visit(ast::RoutineCall* node) override;

private:
    size_t m_depth;
};

class PrettyPrinter : public ast::Visitor {
public:
    PrettyPrinter(size_t depth = 0) : m_depth(depth) {}
    void visit(ast::Program* node) override;
    void visit(ast::RoutineDecl* node) override;
    void visit(ast::Type* node) override;
    void visit(ast::AliasedType* node) override;
    void visit(ast::PrimitiveType* node) override;
    void visit(ast::IntegerType* node) override;
    void visit(ast::RealType* node) override;
    void visit(ast::BooleanType* node) override;
    void visit(ast::ArrayType* node) override;
    void visit(ast::RecordType* node) override;
    void visit(ast::VariableDecl* node) override;
    void visit(ast::TypeDecl* node) override;
    void visit(ast::Body* node) override;
    void visit(ast::Statement* node) override;
    void visit(ast::ReturnStatement* node) override;
    void visit(ast::Assignment* node) override;
    void visit(ast::WhileLoop* node) override;
    void visit(ast::ForLoop* node) override;
    void visit(ast::IfStatement* node) override;
    void visit(ast::Expression* node) override;
    void visit(ast::UnaryExpression* node) override;
    void visit(ast::BinaryExpression* node) override;
    void visit(ast::Primary* node) override;
    void visit(ast::IntegerLiteral* node) override;
    void visit(ast::RealLiteral* node) override;
    void visit(ast::BooleanLiteral* node) override;
    void visit(ast::Identifier* node) override;
    void visit(ast::RoutineCall* node) override;

private:
    size_t m_depth = 0;

    // If not zero, prefer to output code in one line.
    // The type is integer to allow nested (recursive) "enabling" and
    //  "disabling" of this option.
    int m_oneLine = 0;

    // Option should be changed to non-zero value to skip 'var' keyword when
    //  printing variable declarations.
    // Useful for correctly printing record fields and routine parameters.
    int m_skipVarKeyword = 0;

    // Print a new line and indent correctly.
    void newline();
};

/**
 * This visitor performs resolution of variables/routine calls to their
 *  declarations.
 * First, it verifies that there are no conflicts in names in each scope.
 * Then, each instance of Identifier is either filled with a reference to its
 *  declaration or replaced with a routine call referring to its routine
 *  declaration.
 */
class IdentifierResolver : public ast::Visitor {
public:
    void visit(ast::Program* node) override;
    void visit(ast::RoutineDecl* node) override;
    void visit(ast::Type* node) override;
    void visit(ast::AliasedType* node) override;
    void visit(ast::PrimitiveType* node) override;
    void visit(ast::IntegerType* node) override;
    void visit(ast::RealType* node) override;
    void visit(ast::BooleanType* node) override;
    void visit(ast::ArrayType* node) override;
    void visit(ast::RecordType* node) override;
    void visit(ast::VariableDecl* node) override;
    void visit(ast::TypeDecl* node) override;
    void visit(ast::Body* node) override;
    void visit(ast::Statement* node) override;
    void visit(ast::ReturnStatement* node) override;
    void visit(ast::Assignment* node) override;
    void visit(ast::WhileLoop* node) override;
    void visit(ast::ForLoop* node) override;
    void visit(ast::IfStatement* node) override;
    void visit(ast::Expression* node) override;
    void visit(ast::UnaryExpression* node) override;
    void visit(ast::BinaryExpression* node) override;
    void visit(ast::Primary* node) override;
    void visit(ast::IntegerLiteral* node) override;
    void visit(ast::RealLiteral* node) override;
    void visit(ast::BooleanLiteral* node) override;
    void visit(ast::Identifier* node) override;
    void visit(ast::RoutineCall* node) override;

private:
    // A map since we cannot have 2 routines with the same name.
    std::map<std::string, sPtr<ast::RoutineDecl>> m_routines;
    // Stack that holds available variables in current scope.
    std::vector<sPtr<ast::VariableDecl>> m_variables;
    // Just like above but for types.
    std::vector<sPtr<ast::TypeDecl>> m_types;

    sPtr<ast::RoutineCall> m_toReplaceVar = nullptr;
    sPtr<ast::Type> m_toReplaceType = nullptr;

    sPtr<ast::VariableDecl> findVarDecl(std::string);

    void checkReplacementVar(sPtr<ast::Expression>&);
    void checkReplacementType(sPtr<ast::Type>&);

    // Requires: `Container` to be an std container with of type
    //  `sPtr<DeclPtr>`.
    //           `DeclPtr` should be a pointer to an ast node.
    // Effects: returns true if there exists a declaration in the Container with
    //          position prior to the provided declaration and with the same
    //          name.
    template <typename Container, typename DeclPtr>
    bool hasRedecl(const Container& vec, DeclPtr decl,
                   const std::string& typeStr) {
        auto priorDecl =
            std::find_if(vec.begin(), vec.end(), [&](const auto& declPtr) {
                return declPtr->name == decl->name &&
                       declPtr->begin < decl->begin;
            });

        if (priorDecl != vec.end()) {
            error(decl->begin, "redeclaration of {} '{}' declared at line {}",
                  typeStr, (*priorDecl)->name, (*priorDecl)->begin.line);
            return true;
        }

        return false;
    }

    template <typename DeclPtr>
    bool isRedeclared(ast::Program const* node, DeclPtr decl) {
        return hasRedecl(node->routines, decl, "routine") ||
               hasRedecl(node->variables, decl, "variable") ||
               hasRedecl(node->types, decl, "type");
    }
};

/**
 * This visitor is responsible for verifying that declarations of array type
 *  include the size except possibly for parameters
 */
class ArrayLengthEnforcer : public ast::Visitor {
public:
    void visit(ast::Program* node) override;
    void visit(ast::RoutineDecl* node) override;
    void visit(ast::Type* node) override;
    void visit(ast::AliasedType* node) override;
    void visit(ast::PrimitiveType* node) override;
    void visit(ast::IntegerType* node) override;
    void visit(ast::RealType* node) override;
    void visit(ast::BooleanType* node) override;
    void visit(ast::ArrayType* node) override;
    void visit(ast::RecordType* node) override;
    void visit(ast::VariableDecl* node) override;
    void visit(ast::TypeDecl* node) override;
    void visit(ast::Body* node) override;
    void visit(ast::Statement* node) override;
    void visit(ast::ReturnStatement* node) override;
    void visit(ast::Assignment* node) override;
    void visit(ast::WhileLoop* node) override;
    void visit(ast::ForLoop* node) override;
    void visit(ast::IfStatement* node) override;
    void visit(ast::Expression* node) override;
    void visit(ast::UnaryExpression* node) override;
    void visit(ast::BinaryExpression* node) override;
    void visit(ast::Primary* node) override;
    void visit(ast::IntegerLiteral* node) override;
    void visit(ast::RealLiteral* node) override;
    void visit(ast::BooleanLiteral* node) override;
    void visit(ast::Identifier* node) override;
    void visit(ast::RoutineCall* node) override;

private:
    bool m_insideParameters = false;
};

/**
 * This visitor is responsible for verifying that expression types are
 *  compatible and filling the types with the appropriate casting rules
 */
class ExpressionTypeFiller : public ast::Visitor {
public:
    void visit(ast::Program* node) override;
    void visit(ast::RoutineDecl* node) override;
    void visit(ast::Type* node) override;
    void visit(ast::AliasedType* node) override;
    void visit(ast::PrimitiveType* node) override;
    void visit(ast::IntegerType* node) override;
    void visit(ast::RealType* node) override;
    void visit(ast::BooleanType* node) override;
    void visit(ast::ArrayType* node) override;
    void visit(ast::RecordType* node) override;
    void visit(ast::VariableDecl* node) override;
    void visit(ast::TypeDecl* node) override;
    void visit(ast::Body* node) override;
    void visit(ast::Statement* node) override;
    void visit(ast::ReturnStatement* node) override;
    void visit(ast::Assignment* node) override;
    void visit(ast::WhileLoop* node) override;
    void visit(ast::ForLoop* node) override;
    void visit(ast::IfStatement* node) override;
    void visit(ast::Expression* node) override;
    void visit(ast::UnaryExpression* node) override;
    void visit(ast::BinaryExpression* node) override;
    void visit(ast::Primary* node) override;
    void visit(ast::IntegerLiteral* node) override;
    void visit(ast::RealLiteral* node) override;
    void visit(ast::BooleanLiteral* node) override;
    void visit(ast::Identifier* node) override;
    void visit(ast::RoutineCall* node) override;

private:
    bool isBoolean(ast::Expression* node) {
        return std::dynamic_pointer_cast<ast::BooleanType>(node->type) !=
               nullptr;
    }
    bool isInteger(ast::Expression* node) {
        return std::dynamic_pointer_cast<ast::IntegerType>(node->type) !=
               nullptr;
    }
    bool isReal(ast::Expression* node) {
        return std::dynamic_pointer_cast<ast::RealType>(node->type) != nullptr;
    }
    bool isRecord(ast::Expression* node) {
        return std::dynamic_pointer_cast<ast::RecordType>(node->type) !=
               nullptr;
    }
    bool isArray(ast::Expression* node) {
        return std::dynamic_pointer_cast<ast::ArrayType>(node->type) != nullptr;
    }
    bool isIdentifier(ast::Expression* node) {
        return dynamic_cast<ast::Identifier*>(node) != nullptr;
    }
    bool isPrimitive(ast::Expression* node) {
        return isBoolean(node) || isInteger(node) || isReal(node);
    }
    bool conforms(ast::Expression* lhs, ast::Expression* rhs) {
        if (isBoolean(lhs)) {
            bool is1or0 = false;
            auto literal = dynamic_cast<ast::IntegerLiteral*>(rhs);
            if (isInteger(rhs) && literal != nullptr) {
                is1or0 = literal->value == 1 || literal->value == 0;
            }
            return isBoolean(rhs) || is1or0;
        }
        if (isInteger(lhs) || isReal(lhs)) {
            return isPrimitive(rhs);
        }
        // if not primitive, must match exactly (by value, not reference)
        // TODO: handle array conformance for length
        //  might be easier to implement on the == operator itself
        return *lhs->type == *rhs->type;
    }
};

} // namespace san
