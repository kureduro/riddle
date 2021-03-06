#include "ast.hpp"
#include <memory>

namespace san {

template <typename T> using sPtr = std::shared_ptr<T>;

class AstPrinter : public ast::Visitor {
public:
    AstPrinter(size_t depth = 0) : m_depth(depth) {}
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

private:
    size_t m_depth;
};

/**
 * Pretty printer will produce a code using the AST, that would resemble
 * real code as much as possible.
 *
 * Does not alter the tree.
 */
class PrettyPrinter : public ast::Visitor {
public:
    PrettyPrinter(size_t depth = 0) : m_depth(depth) {}
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

private:
    size_t m_depth = 0;

    // If not zero, prefer to output code in one line.
    // The type is integer to allow nested (recursive) "enabling" and
    // "disabling" of this option.
    int m_oneLine = 0;

    // Option should be changed to non-zero value to skip 'var' keyword when
    // printing variable declarations. Useful for correctly printing record
    // fields and routine parameters.
    int m_skipVarKeyword = 0;

    // Print a new line and indent correctly.
    void newline();
};

/**
 * Analyze bodies of the routines and check that routine will always return.
 *
 * It is possible to satisfy checker by always putting a return statement at
 * the end of routines. More complex constructions may not be allowed.
 *
 * For example, this, although always returning, would be rejected:
 *
 * for i in 1..9 loop
 *     for j in 1..9 loop
 *         if i = 9 and j = 9 then
 *             return
 *         end
 *     end
 * end
 */
class MissingReturn : public ast::Visitor {
public:
    MissingReturn() {}
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

private:
    bool m_hasReturn = true;
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

private:
    bool m_insideParameters = false;
};
/**
 * This visitor is responsible for validation of parameters
 * of the routines:
 * - check that number of parameters and their types in RoutineCalls match the
 * respective RoutineDecl
 * + assignment of the RoutineDecl's return type to the RoutineCall's type
 */
class ParamsValidator : public ast::Visitor {
public:
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
};

/**
 * This visitor is responsible for
 * - detection of possibility of type conversion
 * - determining the resulting type of the expression
 * - checking that array index and range thresholds are integers
 * - checking that dot is called only on records
 */

class TypeDeriver : public ast::Visitor {
public:
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

private:
    // function, which returns the type of expression, if its lhs is of type
    // `initialType` and rhs is of type `targetType`
    sPtr<ast::Type> getGreaterType(sPtr<ast::Type> initialType,
                                   sPtr<ast::Type> targetType);

    // checks if the given type is an Integer, Boolean or Real
    bool typeIsPrimitive(sPtr<ast::Type> type);

    // checks if TypeKind is Integer or Boolean
    bool typeIsBooleanconvertible(sPtr<ast::Type> type);

    // if this variable is set to true, variable `m_arrayInnerType` will be
    // set to the type of the array during array visiting
    bool m_searchArray = false;
    sPtr<ast::Type> m_arrayInnerType = nullptr;

    // if this variable is set to true, variable `m_recordField` will be set to
    // the name of the last visited identifier
    bool m_searchField = false;
    std::string m_recordField = "";

    // if this variable is set to true, variable `m_recordInnerType` will be set
    // to the type of field `m_recordField`
    bool m_searchRecord = false;
    sPtr<ast::Type> m_recordInnerType = nullptr;

    // checks if the visitor is inside the list of routine paramenters (used for
    // array length check)
    bool m_inRoutineParams = false;

    // bool DeriveType::checkTypesAreEqual(sPtr<ast::Type> type1,
    //                                     sPtr<ast::Type> type2);
    // sPtr<std::vector<ast::TypeKind>> getFullType(sPtr<ast::Type> type1);
};

} // namespace san
