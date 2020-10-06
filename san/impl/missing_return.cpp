#include "ast.hpp"
#include "fmt/core.h"
#include "san.hpp"
#include <memory>
#include <cassert>

using namespace ast;

namespace san {

void MissingReturn::visit(Program* node) {
    for (auto routine : node->routines) {
        // Routines that don't return anything, can return nothing.
        if (routine->returnType == nullptr) {
            continue;
        }

        m_hasReturn = false;

        routine->accept(*this);

        if (!m_hasReturn) {
            error(routine->begin, "missing a return statement on some execution paths");
        }
    }
}

void MissingReturn::visit(RoutineDecl* node) {
    node->body->accept(*this);
}

void MissingReturn::visit(AliasedType*) {}

void MissingReturn::visit(IntegerType*) {}

void MissingReturn::visit(RealType*) {}

void MissingReturn::visit(BooleanType*) {}

void MissingReturn::visit(ArrayType*) {}

void MissingReturn::visit(RecordType*) {}

void MissingReturn::visit(VariableDecl*) {}

void MissingReturn::visit(TypeDecl*) {}

void MissingReturn::visit(Body* node) {
    m_hasReturn = false;

    for (auto statement : node->statements) {
        statement->accept(*this);

        if (m_hasReturn) {
            return;
        }
    }
}

void MissingReturn::visit(ReturnStatement*) {
    m_hasReturn = true;
}

void MissingReturn::visit(Assignment*) {}

void MissingReturn::visit(WhileLoop*) {
    // We will ignore loops and pretend that we can not
    // determine if it always returns unconditionally.
    // That is, this will be treated as possibly not returning:
    // whil true loop
    //     return
    // end
}

void MissingReturn::visit(ForLoop*) {
    // As well as this:
    // for i in 1..9 loop
    //     return
    // end
}

void MissingReturn::visit(IfStatement* node) {
    // If statements can be determined to be always returning.
    // It is if all two branches return.

    m_hasReturn = false;

    if (node->elseBody == nullptr) {
        return;
    }
    
    node->ifBody->accept(*this);

    // Don't waste our time on else branch, if we already know that if may
    // not return.
    if (!m_hasReturn) {
        return;
    }

    node->elseBody->accept(*this);
}

void MissingReturn::visit(UnaryExpression*) {}

void MissingReturn::visit(BinaryExpression*) {}

void MissingReturn::visit(IntegerLiteral*) {}

void MissingReturn::visit(RealLiteral*) {}

void MissingReturn::visit(BooleanLiteral*) {}

void MissingReturn::visit(Identifier*) {}

void MissingReturn::visit(RoutineCall*) {}

void MissingReturn::visit(Type*) {
    assert(false);
}

void MissingReturn::visit(PrimitiveType*) {
    assert(false);
}

void MissingReturn::visit(Statement*) {
    assert(false);
}

void MissingReturn::visit(Expression*) {
    assert(false);
}

void MissingReturn::visit(Primary*) {
    assert(false);
}

} // namespace san
