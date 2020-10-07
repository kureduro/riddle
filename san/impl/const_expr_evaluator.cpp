#include "san.hpp"
#include <cassert>

namespace san {

void ConstExprEvaluator::visit(ast::Program* node) {
    // records and arrays can have expressions
    for (auto& type : node->types) {
        type->accept(*this);
    }
    for (auto& varDecl : node->variables) {
        varDecl->accept(*this);
    }
    for (auto& routine : node->routines) {
        routine->accept(*this);
    }
}

void ConstExprEvaluator::visit(ast::RoutineDecl* node) {
    for (auto& param : node->parameters) {
        param->accept(*this);
    }
    if (node->returnType != nullptr) {
        node->returnType->accept(*this);
    }
    node->body->accept(*this);
}

void ConstExprEvaluator::visit(ast::AliasedType* node) {
    // There should be no more AliasedType instances at this point as they all
    //  should have been replaced by the IdentifierResolver
}

void ConstExprEvaluator::visit(ast::IntegerType* node) {}

void ConstExprEvaluator::visit(ast::RealType* node) {}

void ConstExprEvaluator::visit(ast::BooleanType* node) {}

void ConstExprEvaluator::visit(ast::ArrayType* node) {
    if (node->length != nullptr) {
        node->length->accept(*this);
        if (!node->length->constant) {
            error(node->length->begin, "array length must be a constant");
        }
    }
    node->elementType->accept(*this);
}

void ConstExprEvaluator::visit(ast::RecordType* node) {
    for (auto& field : node->fields) {
        field->accept(*this);
    }
}

void ConstExprEvaluator::visit(ast::VariableDecl* node) {
    if (node->initialValue != nullptr) {
        node->initialValue->accept(*this);
    }
    if (node->type != nullptr) {
        node->type->accept(*this);
    }
}

void ConstExprEvaluator::visit(ast::TypeDecl* node) {
    node->type->accept(*this);
}

void ConstExprEvaluator::visit(ast::Body* node) {
    for (auto& type : node->types) {
        type->accept(*this);
    }
    for (auto& varDecl : node->variables) {
        varDecl->accept(*this);
    }
    for (auto& statement : node->statements) {
        statement->accept(*this);
    }
}

void ConstExprEvaluator::visit(ast::ReturnStatement* node) {
    if (node->expression) {
        node->expression->accept(*this);
    }
}

void ConstExprEvaluator::visit(ast::Assignment* node) {
    node->lhs->accept(*this);
    node->rhs->accept(*this);
}

void ConstExprEvaluator::visit(ast::WhileLoop* node) {
    node->condition->accept(*this);
    if (node->condition->constant) {
        // For future: remove the while loop, possible with the body as well
    }
    node->body->accept(*this);
}

void ConstExprEvaluator::visit(ast::ForLoop* node) {
    node->rangeFrom->accept(*this);
    node->rangeTo->accept(*this);
    node->body->accept(*this);
}

void ConstExprEvaluator::visit(ast::IfStatement* node) {
    node->condition->accept(*this);
    if (node->condition->constant) {
        // For future: can replace the entire if/else with its body
    }
    node->ifBody->accept(*this);
    if (node->elseBody != nullptr) {
        node->elseBody->accept(*this);
    }
}

void ConstExprEvaluator::visit(ast::UnaryExpression* node) {
    node->operand->accept(*this);
    node->constant = node->operand->constant;
    if (node->constant) {
        // TODO: evaluate it
        // How to evaluate it without having expression types filled?
        switch (node->operation) {
        case lexer::TokenType::Not:
            break;
        case lexer::TokenType::Sub:
            break;
        case lexer::TokenType::Add:
            break;

        default:
            error(node->begin, "unknown unary operator");
        }
    }
}

void ConstExprEvaluator::visit(ast::BinaryExpression* node) {
    node->operand1->accept(*this);
    node->operand2->accept(*this);
    node->constant = (node->operand1->constant && node->operand2->constant);
    if (node->constant) {
        // TODO: evaluate it
        // How to evaluate it without having expression types filled?
        switch (node->operation) {
        case lexer::TokenType::Add:
            break;
        case lexer::TokenType::Sub:
            break;
        case lexer::TokenType::Mul:
            break;
        case lexer::TokenType::Div:
            break;
        case lexer::TokenType::Mod:
            break;
        case lexer::TokenType::Eq:
            break;
        case lexer::TokenType::Neq:
            break;
        case lexer::TokenType::Less:
            break;
        case lexer::TokenType::Leq:
            break;
        case lexer::TokenType::Greater:
            break;
        case lexer::TokenType::Geq:
            break;

        default:
            error(node->begin, "unknown binary operator");
        }
    }
}

void ConstExprEvaluator::visit(ast::IntegerLiteral* node) {
    node->constant = true;
}

void ConstExprEvaluator::visit(ast::RealLiteral* node) {
    node->constant = true;
}

void ConstExprEvaluator::visit(ast::BooleanLiteral* node) {
    node->constant = true;
}

void ConstExprEvaluator::visit(ast::Identifier* node) {
    node->constant = false;
}

void ConstExprEvaluator::visit(ast::RoutineCall* node) {
    // although the routine may possible be returning a constant number from all
    //  its branches, we cannot determine/assume that it does (at least for now)
    node->constant = false;
    for (auto& arg : node->args) {
        arg->accept(*this);
    }
}

} // namespace san
