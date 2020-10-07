#include "san.hpp"
#include <cassert>

using namespace ast;

namespace san {

void ExpressionTypeFiller::visit(Program* node) {
    for (auto const& typeDecl : node->types) {
        typeDecl->accept(*this);
    }
    for (auto const& varDecl : node->variables) {
        varDecl->accept(*this);
    }
    for (auto const& routine : node->routines) {
        routine->accept(*this);
    }
}

void ExpressionTypeFiller::visit(RoutineDecl* node) {}

void ExpressionTypeFiller::visit(AliasedType* node) {}

void ExpressionTypeFiller::visit(IntegerType* node) {}

void ExpressionTypeFiller::visit(RealType* node) {}

void ExpressionTypeFiller::visit(BooleanType* node) {}

void ExpressionTypeFiller::visit(ArrayType* node) {
    if (node->length != nullptr) {
        node->length->accept(*this);
    }
    node->elementType->accept(*this);
}

void ExpressionTypeFiller::visit(RecordType* node) {
    for (auto const field : node->fields) {
        field->accept(*this);
    }
}

void ExpressionTypeFiller::visit(VariableDecl* node) {
    if (node->type == nullptr) {
        if (node->initialValue == nullptr) {
            error(node->begin,
                  "either a type or an initial value must be specified");
            return;
        }
        // TODO: infer type from initial value
    } else if (node->initialValue != nullptr) {
        // TODO: confirm conformance
    }
    // else: specifies type but not initial value. job of code generator
}

void ExpressionTypeFiller::visit(TypeDecl* node) {}

void ExpressionTypeFiller::visit(Body* node) {}

void ExpressionTypeFiller::visit(ReturnStatement* node) {
    // Keep context with current routine. verify types match
}

void ExpressionTypeFiller::visit(Assignment* node) {
    node->lhs->accept(*this);
    node->rhs->accept(*this);

    // TODO: verify conformance
}

void ExpressionTypeFiller::visit(WhileLoop* node) {}

void ExpressionTypeFiller::visit(ForLoop* node) {}

void ExpressionTypeFiller::visit(IfStatement* node) {}

void ExpressionTypeFiller::visit(UnaryExpression* node) {}

void ExpressionTypeFiller::visit(BinaryExpression* node) {
    node->operand1->accept(*this);
    if (node->operation == lexer::TokenType::Dot) {
        if (!isRecord(node->operand1.get())) {
            error(node->operand1->end, "can only access members of records");
            return;
        }

        auto operand1Type =
            std::dynamic_pointer_cast<ast::RecordType>(node->operand1->type);
        if (!isIdentifier(node->operand2.get())) {
            error(node->operand2->begin, "expected an identifier");
            return;
        }

        auto operand2 =
            std::dynamic_pointer_cast<ast::Identifier>(node->operand1);
        for (auto const& field : operand1Type->fields) {
            if (field->name == operand2->name) {
                operand2->variable = field;
                break;
            }
        }
        if (operand2->variable == nullptr) {
            error(operand2->begin, "'{}' is not a member of the record",
                  operand2->name);
            return;
        }
    } else if (node->operation == lexer::TokenType::OpenBrack) {
        // Array access
    } else {
        // TODO: verify conformance
        // TODO: set node->type according to casting rules
    }
    node->operand2->accept(*this);
}

void ExpressionTypeFiller::visit(IntegerLiteral* node) {}

void ExpressionTypeFiller::visit(RealLiteral* node) {}

void ExpressionTypeFiller::visit(BooleanLiteral* node) {}

void ExpressionTypeFiller::visit(Identifier* node) {
    // an identifier has not been associated with a variable declaration. This
    //  must mean it is a member of some record
    if (node->variable == nullptr) {
        // TODO: check m_recordDecl, find appropriate varDecl, and link it
    }
}

void ExpressionTypeFiller::visit(RoutineCall* node) {
    for (auto const& arg : node->args) {
        arg->accept(*this);
    }
}

void ExpressionTypeFiller::visit(Expression* node) { assert(false); }

void ExpressionTypeFiller::visit(Primary* node) { assert(false); }

void ExpressionTypeFiller::visit(Statement* node) { assert(false); }

void ExpressionTypeFiller::visit(Type* node) { assert(false); }

void ExpressionTypeFiller::visit(PrimitiveType* node) { assert(false); }
} // namespace san
