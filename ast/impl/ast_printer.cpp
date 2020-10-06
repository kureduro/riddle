#include "visitors.hpp"

using namespace ast;

std::map<lexer::TokenType, std::string> g_opToString{
    {lexer::TokenType::Add, "+"},     {lexer::TokenType::Sub, "-"},
    {lexer::TokenType::Mul, "*"},     {lexer::TokenType::Div, "/"},
    {lexer::TokenType::Mod, "%"},     {lexer::TokenType::Or, "or"},
    {lexer::TokenType::Xor, "xor"},   {lexer::TokenType::And, "and"},
    {lexer::TokenType::Eq, "="},      {lexer::TokenType::Neq, "/="},
    {lexer::TokenType::Less, "<"},    {lexer::TokenType::Leq, "<="},
    {lexer::TokenType::Greater, ">"}, {lexer::TokenType::Geq, ">="},
    {lexer::TokenType::Not, "not"},   {lexer::TokenType::OpenParen, "()"},
    {lexer::TokenType::Dot, "."},     {lexer::TokenType::OpenBrack, "[]"},
};

namespace san {

void AstPrinter::visit(Program* node) {
    fmt::print("{:|>{}}- [Program]>\n", "", m_depth);

    m_depth++;

    for (auto type : node->types) {
        type->accept(*this);
    }

    for (auto variableDecl : node->variables) {
        variableDecl->accept(*this);
    }

    for (auto routine : node->routines) {
        routine->accept(*this);
    }

    m_depth--;
}
void AstPrinter::visit(RoutineDecl* node) {
    fmt::print("{:|>{}}- [RoutineDecl]> {}\n", "", m_depth, node->name);

    m_depth++;

    for (auto parameter : node->parameters) {
        parameter->accept(*this);
    }

    node->body->accept(*this);

    if (node->returnType != nullptr) {
        node->returnType->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(Type*) {
    fmt::print("{:|>{}}- [Type]> (stray type)\n", "", m_depth);
}

void AstPrinter::visit(AliasedType* node) {
    fmt::print("{:|>{}}- [Type Identifier]> {}\n", "", m_depth, node->name);

    m_depth++;

    if (node->actualType != nullptr) {
        node->actualType->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(PrimitiveType*) {
    fmt::print("{:|>{}}- [PrimitiveType]> unknown\n", "", m_depth);
}

void AstPrinter::visit(IntegerType*) {
    fmt::print("{:|>{}}- [IntegerType]\n", "", m_depth);
}

void AstPrinter::visit(RealType*) {
    fmt::print("{:|>{}}- [RealType]\n", "", m_depth);
}

void AstPrinter::visit(BooleanType*) {
    fmt::print("{:|>{}}- [BooleanType]\n", "", m_depth);
}

void AstPrinter::visit(ArrayType* node) {
    fmt::print("{:|>{}}- [ArrayType]>\n", "", m_depth);

    m_depth++;

    if (node->length != nullptr) {
        node->length->accept(*this);
    }

    if (node->elementType != nullptr) {
        node->elementType->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(RecordType* node) {
    fmt::print("{:|>{}}- [RecordType]>\n", "", m_depth);

    m_depth++;

    for (auto field : node->fields) {
        field->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(VariableDecl* node) {
    fmt::print("{:|>{}}- [VariableDecl]> {}\n", "", m_depth, node->name);

    m_depth++;

    if (node->type != nullptr) {
        node->type->accept(*this);
    }

    if (node->initialValue != nullptr) {
        node->initialValue->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(TypeDecl* node) {
    fmt::print("{:|>{}}- [TypeDecl]> {}\n", "", m_depth, node->name);

    m_depth++;

    if (node->type != nullptr) {
        node->type->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(Body* node) {
    fmt::print("{:|>{}}- [Body]>\n", "", m_depth);

    m_depth++;

    for (auto type : node->types) {
        type->accept(*this);
    }

    for (auto variableDecl : node->variables) {
        variableDecl->accept(*this);
    }

    for (auto statement : node->statements) {
        statement->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(Statement*) {
    fmt::print("{:|>{}}- [Statement]> Stray\n", "", m_depth);
}

void AstPrinter::visit(ReturnStatement* node) {
    fmt::print("{:|>{}}- [Return]>\n", "", m_depth);
    
    m_depth++;

    if (node->expression != nullptr) {
        node->expression->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(Assignment* node) {
    fmt::print("{:|>{}}- [Assignment]>\n", "", m_depth);

    m_depth++;

    node->lhs->accept(*this);
    node->rhs->accept(*this);

    m_depth--;
}

void AstPrinter::visit(WhileLoop* node) {
    fmt::print("{:|>{}}- [WhileLoop]>\n", "", m_depth);

    m_depth++;

    node->condition->accept(*this);
    node->body->accept(*this);

    m_depth--;
}

void AstPrinter::visit(ForLoop* node) {
    fmt::print("{:|>{}}- [ForLoop]>\n", "", m_depth);

    m_depth++;

    // node->loopVar->accept(*this);
    node->rangeFrom->accept(*this);
    node->rangeTo->accept(*this);
    node->body->accept(*this);

    m_depth--;
}

void AstPrinter::visit(IfStatement* node) {
    fmt::print("{:|>{}}- [IfStatement]>\n", "", m_depth);
    
    m_depth++;

    node->condition->accept(*this);
    node->ifBody->accept(*this);
    if (node->elseBody) {
        node->elseBody->accept(*this);
    }

    m_depth--;
}

void AstPrinter::visit(Expression*) {
    fmt::print("{:|>{}}- [Expression]> Stray\n", "", m_depth);
}

void AstPrinter::visit(UnaryExpression* node) {
    fmt::print("{:|>{}}- [UnaryExpression]> {}\n", "", m_depth,
               g_opToString[node->operation]);

    m_depth++;

    node->operand->accept(*this);

    m_depth--;
}

void AstPrinter::visit(BinaryExpression* node) {
    fmt::print("{:|>{}}- [BinaryExpression]> {}\n", "", m_depth,
               g_opToString[node->operation]);

    m_depth++;

    node->operand1->accept(*this);
    node->operand2->accept(*this);

    m_depth--;
}

void AstPrinter::visit(Primary*) {
    fmt::print("{:|>{}}- [Primary]> stray\n", "", m_depth);
}

void AstPrinter::visit(IntegerLiteral* node) {
    fmt::print("{:|>{}}- [IntegerLiteral]> {}\n", "", m_depth, node->value);
}

void AstPrinter::visit(RealLiteral* node) {
    fmt::print("{:|>{}}- [RealLiteral]> {}\n", "", m_depth, node->value);
}

void AstPrinter::visit(BooleanLiteral* node) {
    fmt::print("{:|>{}}- [BooleanLiteral]> {}\n", "", m_depth, node->value);
}

void AstPrinter::visit(Identifier* node) {
    fmt::print("{:|>{}}- [Identifier]> {}\n", "", m_depth, node->name);
}

void AstPrinter::visit(RoutineCall* node) {
    fmt::print("{:|>{}}- [RoutineCall]> {}\n", "", m_depth, node->routineName);

    m_depth++;

    for (size_t i = 0; i < node->args.size(); i++) {
        node->args[i]->accept(*this);
    }

    m_depth--;
}

} // namespace visitors
