#include "visitors.hpp"
#include <memory>
#include <set>

using namespace ast;

namespace san {

void IdentifierResolver::visit(Program* node) {
    // Check if anything is declared twice in global scope and visit
    // all routines.

    bool ok = true;

    for (auto& typeDecl : node->types) {
        if (isRedeclared(node, typeDecl)) {
            ok = false;
        }

        typeDecl->accept(*this);
        checkReplacementType(typeDecl->type);

        m_types.push_back(typeDecl);
    }

    for (auto& var : node->variables) {
        if (isRedeclared(node, var)) {
            ok = false;
        }

        if (var->initialValue != nullptr) {
            var->initialValue->accept(*this);
            checkReplacementVar(var->initialValue);
        }

        if (var->type != nullptr) {
            var->type->accept(*this);
            checkReplacementType(var->type);
        }

        m_variables.push_back(var);
    }

    for (auto routine : node->routines) {
        if (isRedeclared(node, routine)) {
            ok = false;
        }

        m_routines.insert({routine->name, routine});
    }

    if (!ok) {
        return;
    }

    for (auto routine : node->routines) {
        routine->accept(*this);
    }

    m_variables.clear();
    m_routines.clear();
    m_types.clear();
}

void IdentifierResolver::visit(RoutineDecl* node) {
    // To keep track of the scope, each function that introduces a new scope
    //  will append its declarations to the `m_variables` vector and then remove
    //  them again before returning.
    // When searching for the closest variable declaration later on, the same
    //  vector will be iterated on from end to start

    auto oldSize = m_variables.size();

    for (auto& parameter : node->parameters) {
        parameter->type->accept(*this);
        checkReplacementType(parameter->type);

        m_variables.push_back(parameter);
    }

    if (node->returnType != nullptr) {
        node->returnType->accept(*this);
        checkReplacementType(node->returnType);
    }

    node->body->accept(*this);

    m_variables.resize(oldSize);
}

void IdentifierResolver::visit(Type*) {}

void IdentifierResolver::visit(AliasedType* node) {
    for (auto it = m_types.rbegin(); it != m_types.rend(); it++) {
        auto typeDecl = *it;
        if (typeDecl->name == node->name) {
            m_toReplaceType = typeDecl->type;
            return;
        }
    }
    error(node->begin, "{} does not name a type", node->name);
}

void IdentifierResolver::visit(PrimitiveType*) {}

void IdentifierResolver::visit(IntegerType*) {}

void IdentifierResolver::visit(RealType*) {}

void IdentifierResolver::visit(BooleanType*) {}

void IdentifierResolver::visit(ArrayType* node) {
    if (node->length != nullptr) {
        node->length->accept(*this);
        checkReplacementVar(node->length);
    }

    node->elementType->accept(*this);
    checkReplacementType(node->elementType);
}

void IdentifierResolver::visit(RecordType* node) {
    // ensure that no two fields have the same name
    std::set<std::string> fieldNames;
    for (auto field : node->fields) {
        if (fieldNames.find(field->name) != fieldNames.end()) {
            error(field->begin, "field name is already in use");
            return;
        }

        fieldNames.insert(field->name);
    }

    for (auto field : node->fields) {
        field->accept(*this);
    }
}

void IdentifierResolver::visit(VariableDecl* node) {
    if (node->type != nullptr) {
        node->type->accept(*this);
        checkReplacementType(node->type);
    }

    if (node->initialValue != nullptr) {
        node->initialValue->accept(*this);
        checkReplacementVar(node->initialValue);
    }
}

void IdentifierResolver::visit(TypeDecl* node) {
    node->type->accept(*this);
    checkReplacementType(node->type);
}

void IdentifierResolver::visit(Body* node) {
    // Check for duplicates.

    bool ok = true;

    for (auto var : node->variables) {
        if (hasRedecl(node->variables, var, "variable")) {
            ok = false;
        }
    }

    for (auto type : node->types) {
        if (hasRedecl(node->types, type, "type")) {
            ok = false;
        }
    }

    if (!ok) {
        return;
    }

    // Traverse

    auto oldVarsSize = m_variables.size();
    auto oldTypesSize = m_types.size();

    for (auto& type : node->types) {
        type->accept(*this);
        checkReplacementType(type->type);

        m_types.push_back(type);
    }

    for (auto& variable : node->variables) {
        m_variables.push_back(variable);

        if (variable->initialValue != nullptr) {
            variable->initialValue->accept(*this);
            checkReplacementVar(variable->initialValue);
        }

        if (variable->type != nullptr) {
            variable->type->accept(*this);
            checkReplacementType(variable->type);
        }
    }

    for (auto statement : node->statements) {
        statement->accept(*this);
    }

    m_variables.resize(oldVarsSize);
    m_types.resize(oldTypesSize);
}

void IdentifierResolver::visit(Statement*) {}

void IdentifierResolver::visit(ReturnStatement* node) {
    node->expression->accept(*this);
    checkReplacementVar(node->expression);
}

void IdentifierResolver::visit(Assignment* node) {
    node->lhs->accept(*this);
    checkReplacementVar(node->lhs);

    node->rhs->accept(*this);
    checkReplacementVar(node->rhs);
}

void IdentifierResolver::visit(WhileLoop* node) {
    node->condition->accept(*this);
    checkReplacementVar(node->condition);

    node->body->accept(*this);
}

void IdentifierResolver::visit(ForLoop* node) {
    auto oldSize = m_variables.size();

    m_variables.push_back(node->loopVar);

    node->body->accept(*this);

    m_variables.resize(oldSize);
}

void IdentifierResolver::visit(IfStatement* node) {
    node->condition->accept(*this);
    checkReplacementVar(node->condition);

    node->ifBody->accept(*this);

    if (node->elseBody != nullptr) {
        node->elseBody->accept(*this);
    }
}

void IdentifierResolver::visit(Expression*) {}

void IdentifierResolver::visit(UnaryExpression* node) {
    node->operand->accept(*this);
    checkReplacementVar(node->operand);
}

void IdentifierResolver::visit(BinaryExpression* node) {
    node->operand1->accept(*this);
    checkReplacementVar(node->operand1);

    if (node->operation == lexer::TokenType::Dot) {
        // The left operand can be an expression (ex: `a.b` in `a.b.c`), but the
        //  right one always has to be an Identifier (cannot have `x.3`).
        // No further checks can be implemented here since we need to first
        //  populate expression types to make sure that the identifier is
        //  actually a record member.
        // The code is left here commented out for later use in the upcoming
        //  checkers

        // auto operand1 =
        // std::dynamic_pointer_cast<Identifier>(node->operand1); if (operand1
        // == nullptr) {
        //     error(node->operand1->begin,
        //           "expected variable identifier before '.'");
        //     return;
        // }

        auto operand2 = std::dynamic_pointer_cast<Identifier>(node->operand2);
        if (operand2 == nullptr) {
            error(node->operand2->begin, "expected identifier after '.'");
            return;
        }

        // auto recordDecl =
        // std::dynamic_pointer_cast<RecordType>(operand1->type); if (recordDecl
        // == nullptr) {
        //     error(node->operand2->begin, "only records can be member
        //     accessed"); return;
        // }
        // for (auto field : recordDecl->fields) {
        //     if (field->name == operand2->name) {
        //         operand2->variable = field;
        //         operand2->type = field->type;
        //         break;
        //     }
        // }
        // if (operand2->variable == nullptr) {
        //     error(operand2->begin, "record '{}' has no member '{}'",
        //           operand1->name, operand2->name);
        //     return;
        // }
    } else if (node->operation == lexer::TokenType::OpenBrack) {
        // Similar to the problem with record access, we cannot check the type
        //  of the left operand until expression types are populated.
        // Code is left for reference.

        // auto arrayDecl =
        //     std::dynamic_pointer_cast<ArrayType>(node->operand1->type);
        // if (arrayDecl == nullptr) {
        //     error(node->operand1->end, "non-array types cannot be indexed");
        //     return;
        // }
        node->operand2->accept(*this);
        checkReplacementVar(node->operand2);
    } else {
        node->operand2->accept(*this);
        checkReplacementVar(node->operand2);
    }
}

void IdentifierResolver::visit(Primary*) {}

void IdentifierResolver::visit(IntegerLiteral*) {}

void IdentifierResolver::visit(RealLiteral*) {}

void IdentifierResolver::visit(BooleanLiteral*) {}

void IdentifierResolver::visit(Identifier* node) {
    // Here comes the main work of figuring out which variable/routine this
    // identifier refers to

    auto variable = findVarDecl(node->name);
    if (variable != nullptr) {
        node->variable = variable;

        if (variable->type != nullptr) {
            node->type = variable->type;
        }

        return;
    }

    // If it did not match any variable, it must be a parameterless routine call

    m_toReplaceVar = std::make_shared<RoutineCall>();
    m_toReplaceVar->begin = node->begin;
    m_toReplaceVar->routineName = node->name;
    m_toReplaceVar->end = node->end;
    m_toReplaceVar->accept(*this);
}

void IdentifierResolver::visit(RoutineCall* node) {
    auto routineIt = m_routines.find(node->routineName);
    if (routineIt == m_routines.end()) {
        error(node->begin, "undeclared routine: {}", node->routineName);
        return;
    }

    node->routine = routineIt->second;

    for (auto& arg : node->args) {
        arg->accept(*this);
        checkReplacementVar(arg);
    }
}

/**
 * Checks if `m_toReplaceVar` is set, and if so changes the passed parameter to
 *  point to it instead
 */
void IdentifierResolver::checkReplacementVar(sPtr<Expression>& expr) {
    if (m_toReplaceVar != nullptr) {
        expr = m_toReplaceVar;
        m_toReplaceVar = nullptr;
    }
}

/**
 * Checks if `m_toReplaceType` is set, and if so changes the passed parameter to
 *  point to it instead
 */
void IdentifierResolver::checkReplacementType(sPtr<Type>& type) {
    if (m_toReplaceType != nullptr) {
        type = m_toReplaceType;
        m_toReplaceType = nullptr;
    }
}

sPtr<VariableDecl> IdentifierResolver::findVarDecl(std::string name) {
    for (auto it = m_variables.rbegin(); it != m_variables.rend(); it++) {
        auto variable = *it;
        if (variable->name == name) {
            return variable;
        }
    }

    return nullptr;
}

} // namespace san
