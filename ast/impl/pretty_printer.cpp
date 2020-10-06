#include "visitors.hpp"

using namespace ast;

std::map<lexer::TokenType, std::string> op_to_string{
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

void PrettyPrinter::visit(Program* node) {
    if (node == nullptr) {
        return;
    }

    for (auto type : node->types) {
        type->accept(*this);
        fmt::print("\n\n");
    }

    for (auto variableDecl : node->variables) {
        variableDecl->accept(*this);
        fmt::print("\n");
    }

    for (auto routine : node->routines) {
        routine->accept(*this);
        fmt::print("\n\n");
    }
}

void PrettyPrinter::visit(RoutineDecl* node) {
    if (node == nullptr) {
        return;
    }

    fmt::print("routine {} (", node->name);

    for (size_t i = 0; i < node->parameters.size(); i++) {
        node->parameters[i]->accept(*this);
        if (i < node->parameters.size() - 1) {
            fmt::print(", ");
        }
    }

    fmt::print(")");

    if (node->returnType != nullptr) {
        fmt::print(" : ");

        m_oneLine = true;
        node->returnType->accept(*this);
        m_oneLine = false;
    }

    fmt::print(" is\n");

    node->body->accept(*this);
}

void PrettyPrinter::visit(Type*) {
    fmt::print("<stray type>\n", "", m_depth);
}

void PrettyPrinter::visit(AliasedType* node) {
    fmt::print("{}", node->name);
}

void PrettyPrinter::visit(PrimitiveType*) {
    fmt::print("<unknown primitive type>");
}

void PrettyPrinter::visit(IntegerType*) {
    fmt::print("integer");
}

void PrettyPrinter::visit(RealType*) {
    fmt::print("real");
}

void PrettyPrinter::visit(BooleanType*) {
    fmt::print("boolean", "", m_depth);
}

void PrettyPrinter::visit(ArrayType* node) {
    fmt::print("array");

    if (node->length != nullptr) {
        fmt::print(" ");
        node->length->accept(*this);
    }

    if (node->elementType != nullptr) {
        fmt::print(" ");
        node->elementType->accept(*this);
    }
}

void PrettyPrinter::visit(RecordType* node) {
    fmt::print("record");

    if (!m_oneLine) {
        fmt::print("\n");
        m_depth++;
    } else {
        fmt::print(" ");
    }

    for (auto field : node->fields) {
        if (!m_oneLine) {
            fmt::print("{:\t>{}}", "", m_depth);
        }

        field->accept(*this);

        if (!m_oneLine) {
            fmt::print("\n");
        } else {
            fmt::print("; ");
        }
    }

    if (!m_oneLine) {
        fmt::print("end");
        m_depth--;
    } else {
        fmt::print("end;");
    }
}

void PrettyPrinter::visit(VariableDecl* node) {
    fmt::print("var {}", node->name);

    m_oneLine = true;

    if (node->type != nullptr) {
        fmt::print(" : ");
        node->type->accept(*this);
    }

    m_oneLine = false;

    if (node->initialValue != nullptr) {
        fmt::print(" is ");
        node->initialValue->accept(*this);
    }
}

void PrettyPrinter::visit(TypeDecl* node) {
    fmt::print("type {} is ", node->name);
    
    if (node->type != nullptr) {
        node->type->accept(*this);
    }
}

void PrettyPrinter::visit(Body* node) {
    fmt::print("<skip>\n"); return;
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

void PrettyPrinter::visit(Statement* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Statement]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Statement]> Unknown\n", "", m_depth);
}
void PrettyPrinter::visit(ReturnStatement* node) {
    fmt::print("<skip>\n"); return;
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
void PrettyPrinter::visit(Assignment* node) {
    fmt::print("<skip>\n"); return;
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
void PrettyPrinter::visit(WhileLoop* node) {
    fmt::print("<skip>\n"); return;
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
void PrettyPrinter::visit(ForLoop* node) {
    fmt::print("<skip>\n"); return;
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
void PrettyPrinter::visit(IfStatement* node) {
    fmt::print("<skip>\n"); return;
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
void PrettyPrinter::visit(Expression* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Expression]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Expression]>\n", "", m_depth);
}
void PrettyPrinter::visit(UnaryExpression* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [UnaryExpression]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [UnaryExpression]> {}\n", "", m_depth,
               op_to_string[node->operation]);
    m_depth++;
    node->operand->accept(*this);
    m_depth--;
}
void PrettyPrinter::visit(BinaryExpression* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [BinaryExpression]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [BinaryExpression]> {}\n", "", m_depth,
               op_to_string[node->operation]);
    m_depth++;
    node->operand1->accept(*this);
    node->operand2->accept(*this);
    m_depth--;
}
void PrettyPrinter::visit(Primary* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Primary]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Primary]> unknown\n", "", m_depth);
}
void PrettyPrinter::visit(IntegerLiteral* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [IntegerLiteral]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [IntegerLiteral]> {}\n", "", m_depth, node->value);
}
void PrettyPrinter::visit(RealLiteral* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [RealLiteral]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [RealLiteral]> {}\n", "", m_depth, node->value);
}
void PrettyPrinter::visit(BooleanLiteral* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [BooleanLiteral]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [BooleanLiteral]> {}\n", "", m_depth, node->value);
}
void PrettyPrinter::visit(Identifier* node) {
    fmt::print("<skip>\n"); return;
    if (node == nullptr) {
        fmt::print("{:|>{}}- [Identifier]> null\n", "", m_depth);
        return;
    }
    fmt::print("{:|>{}}- [Identifier]> {}\n", "", m_depth, node->name);
}

void PrettyPrinter::visit(RoutineCall* node) {
    fmt::print("<skip>\n"); return;

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
