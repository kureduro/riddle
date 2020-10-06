#include "fmt/core.h"
#include "visitors.hpp"
#include <memory>

using namespace ast;

extern std::map<lexer::TokenType, std::string> g_opToString;

namespace san {

void PrettyPrinter::visit(Program* node) {
    if (node == nullptr) {
        return;
    }

    for (auto type : node->types) {
        type->accept(*this);
        newline();
        newline();
    }

    for (auto variableDecl : node->variables) {
        variableDecl->accept(*this);
        newline();
    }

    newline();

    for (auto routine : node->routines) {
        routine->accept(*this);
        newline();
        newline();
    }
}

void PrettyPrinter::visit(RoutineDecl* node) {
    if (node == nullptr) {
        return;
    }

    fmt::print("routine {} (", node->name);

    m_oneLine++;

    for (size_t i = 0; i < node->parameters.size(); i++) {
        node->parameters[i]->accept(*this);
        if (i < node->parameters.size() - 1) {
            fmt::print(", ");
        }
    }

    fmt::print(")");

    if (node->returnType != nullptr) {
        fmt::print(" : ");

        node->returnType->accept(*this);
    }

    m_oneLine--;

    fmt::print(" is");

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
        fmt::print(" [");
        node->length->accept(*this);
        fmt::print("]");
    }

    if (node->elementType != nullptr) {
        fmt::print(" ");
        node->elementType->accept(*this);
    }
}

void PrettyPrinter::visit(RecordType* node) {
    fmt::print("record");

    if (!m_oneLine) {
        m_depth++;
    } else {
        fmt::print(" ");
    }

    for (auto field : node->fields) {
        if (!m_oneLine) {
            newline();
        }

        field->accept(*this);

        if (m_oneLine) {
            fmt::print("; ");
        }
    }

    if (!m_oneLine) {
        m_depth--;
        newline();
        fmt::print("end");
    } else {
        fmt::print("end;");
    }
}

void PrettyPrinter::visit(VariableDecl* node) {
    fmt::print("var {}", node->name);

    m_oneLine++;

    if (node->type != nullptr) {
        fmt::print(" : ");
        node->type->accept(*this);
    }

    m_oneLine--;

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
    m_depth++;

    for (auto type : node->types) {
        newline();
        type->accept(*this);
        newline();
    }

    for (auto variableDecl : node->variables) {
        newline();
        variableDecl->accept(*this);
    }

    if (!node->variables.empty() && !node->statements.empty()) {
        newline();
    }

    for (auto statement : node->statements) {
        newline();
        statement->accept(*this);
    }

    m_depth--;
    newline();

    // Yes, we'll have
    // if x = 5 then
    //     ...
    // end else
    //     ...
    // end
    // doesn't matter that much... I think
    fmt::print("end");
}

void PrettyPrinter::visit(Statement*) {
    fmt::print("<stray statement>");
}

void PrettyPrinter::visit(ReturnStatement* node) {
    fmt::print("return");

    if (node->expression != nullptr) {
        fmt::print(" ");
        node->expression->accept(*this);
    }
}

void PrettyPrinter::visit(Assignment* node) {
    node->lhs->accept(*this);

    fmt::print(" := ");

    node->rhs->accept(*this);
}
void PrettyPrinter::visit(WhileLoop* node) {
    fmt::print("while ");

    node->condition->accept(*this);

    fmt::print(" loop");

    node->body->accept(*this);
}

void PrettyPrinter::visit(ForLoop* node) {
    fmt::print("for {} in ", node->loopVar->name);

    if (node->reverse) {
        fmt::print("reverse ");
    }

    node->rangeFrom->accept(*this);

    fmt::print("..");

    node->rangeTo->accept(*this);

    fmt::print(" loop");

    node->body->accept(*this);
}

void PrettyPrinter::visit(IfStatement* node) {
    fmt::print("if ");

    node->condition->accept(*this);

    fmt::print(" then");

    node->ifBody->accept(*this);

    if (node->elseBody) {
        fmt::print(" else ");
        node->elseBody->accept(*this);
    }
}

void PrettyPrinter::visit(Expression*) {
    fmt::print("<stray expression>");
}

void PrettyPrinter::visit(UnaryExpression* node) {
    fmt::print("{}", g_opToString[node->operation]);
    node->operand->accept(*this);
}

void PrettyPrinter::visit(BinaryExpression* node) {
    fmt::print("(");

    node->operand1->accept(*this);

    fmt::print(" {} ", g_opToString[node->operation]);

    node->operand2->accept(*this);

    fmt::print(")");
}

void PrettyPrinter::visit(Primary*) {
    fmt::print("<stray primary>");
}

void PrettyPrinter::visit(IntegerLiteral* node) {
    fmt::print("{}", node->value);
}

void PrettyPrinter::visit(RealLiteral* node) {
    fmt::print("{}", node->value);
}

void PrettyPrinter::visit(BooleanLiteral* node) {
    fmt::print("{}", node->value);
}

void PrettyPrinter::visit(Identifier* node) {
    fmt::print("{}", node->name);
}

void PrettyPrinter::visit(RoutineCall* node) {
    fmt::print("{}(", node->routineName);

    for (size_t i = 0; i < node->args.size(); i++) {
        node->args[i]->accept(*this);

        if (i + 1 != node->args.size()) {
            fmt::print(", ");
        }
    }

    fmt::print(")");
}

void PrettyPrinter::newline() {
    fmt::print("\n{:\t>{}}", "", m_depth);
}

} // namespace san
