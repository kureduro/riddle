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
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Program]> null\n", "", m_depth);
        return;
    }
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
    if (node == nullptr) {
        fmt::print("{:|>{}}- [RoutineDecl]> null\n", "", m_depth);
        return;
    }
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
void AstPrinter::visit(Type* node) {
    if (ast::PrimitiveType* specific = dynamic_cast<ast::PrimitiveType*>(node);
        specific != nullptr) {
        specific->accept(*this);
    } else if (ast::ArrayType* specific = dynamic_cast<ast::ArrayType*>(node);
               specific != nullptr) {
        specific->accept(*this);
    } else if (ast::RecordType* specific = dynamic_cast<ast::RecordType*>(node);
               specific != nullptr) {
        specific->accept(*this);
    } else if (ast::AliasedType* specific =
                   dynamic_cast<ast::AliasedType*>(node);
               specific != nullptr) {
        fmt::print("{:|>{}}- [Type Identifier]> {}\n", "", m_depth,
                   specific->name);
    } else {
        fmt::print("{:|>{}}- [Type]> (unknown type)\n", "", m_depth);
    }
}
void AstPrinter::visit(AliasedType* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Type Identifier]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Type Identifier]> {}\n", "", m_depth, node->name);
    m_depth++;
    if (node->actualType != nullptr) {
        node->actualType->accept(*this);
    }
    m_depth--;
}
void AstPrinter::visit(PrimitiveType* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [PrimitiveType]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [PrimitiveType]> unknown\n", "", m_depth);
}
void AstPrinter::visit(IntegerType* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [IntegerType]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [IntegerType]\n", "", m_depth);
}
void AstPrinter::visit(RealType* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [RealType]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [RealType]\n", "", m_depth);
}
void AstPrinter::visit(BooleanType* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [BooleanType]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [BooleanType]\n", "", m_depth);
}
void AstPrinter::visit(ArrayType* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [ArrayType]> null\n", "", m_depth);
        return;
    }
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
    if (node == nullptr) {
        fmt::print("{:|>{}}- [RecordType]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [RecordType]>\n", "", m_depth);
    m_depth++;
    for (auto field : node->fields) {
        field->accept(*this);
    }
    m_depth--;
}
void AstPrinter::visit(VariableDecl* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [VariableDecl]> null\n", "", m_depth);
    }
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
    if (node == nullptr) {
        fmt::print("{:|>{}}- [TypeDecl]> null\n", "", m_depth);
    }
    fmt::print("{:|>{}}- [TypeDecl]> {}\n", "", m_depth, node->name);
    m_depth++;
    if (node->type != nullptr) {
        node->type->accept(*this);
    }
    m_depth--;
}
void AstPrinter::visit(Body* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Body]> null\n", "", m_depth);
        return;
    }
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
void AstPrinter::visit(Statement* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Statement]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Statement]> Unknown\n", "", m_depth);
}
void AstPrinter::visit(ReturnStatement* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Return]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Return]>\n", "", m_depth);
    m_depth++;
    if (node->expression != nullptr) {
        node->expression->accept(*this);
    }
    m_depth--;
}
void AstPrinter::visit(Assignment* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Assignment]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Assignment]>\n", "", m_depth);
    m_depth++;
    node->lhs->accept(*this);
    node->rhs->accept(*this);
    m_depth--;
}
void AstPrinter::visit(WhileLoop* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [WhileLoop]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [WhileLoop]>\n", "", m_depth);
    m_depth++;
    node->condition->accept(*this);
    node->body->accept(*this);
    m_depth--;
}
void AstPrinter::visit(ForLoop* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [ForLoop]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [ForLoop]>\n", "", m_depth);
    m_depth++;
    // node->loopVar->accept(*this);
    node->rangeFrom->accept(*this);
    node->rangeTo->accept(*this);
    node->body->accept(*this);
    m_depth--;
}
void AstPrinter::visit(IfStatement* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [IfStatement]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [IfStatement]>\n", "", m_depth);
    m_depth++;
    node->condition->accept(*this);
    node->ifBody->accept(*this);
    if (node->elseBody) {
        node->elseBody->accept(*this);
    }
    m_depth--;
}
void AstPrinter::visit(Expression* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Expression]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Expression]>\n", "", m_depth);
}
void AstPrinter::visit(UnaryExpression* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [UnaryExpression]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [UnaryExpression]> {}\n", "", m_depth,
               g_opToString[node->operation]);
    m_depth++;
    node->operand->accept(*this);
    m_depth--;
}
void AstPrinter::visit(BinaryExpression* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [BinaryExpression]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [BinaryExpression]> {}\n", "", m_depth,
               g_opToString[node->operation]);
    m_depth++;
    node->operand1->accept(*this);
    node->operand2->accept(*this);
    m_depth--;
}
void AstPrinter::visit(Primary* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Primary]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Primary]> unknown\n", "", m_depth);
}
void AstPrinter::visit(IntegerLiteral* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [IntegerLiteral]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [IntegerLiteral]> {}\n", "", m_depth, node->value);
}
void AstPrinter::visit(RealLiteral* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [RealLiteral]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [RealLiteral]> {}\n", "", m_depth, node->value);
}
void AstPrinter::visit(BooleanLiteral* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [BooleanLiteral]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [BooleanLiteral]> {}\n", "", m_depth, node->value);
}
void AstPrinter::visit(Identifier* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Identifier]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Identifier]> {}\n", "", m_depth, node->name);
}
void AstPrinter::visit(RoutineCall* node) {
    if (node == nullptr) {
        fmt::print("{:|>{}}- [RoutineCall]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [RoutineCall]> {}\n", "", m_depth, node->routineName);
    m_depth++;
    for (size_t i = 0; i < node->args.size(); i++) {
        node->args[i]->accept(*this);
    }
    m_depth--;
}

} // namespace visitors
