#include "ast.hpp"
#include "fmt/core.h"
#include "san.hpp"
#include <memory>
#include <cassert>

using namespace ast;

namespace san {

void MissingReturn::visit(Program* node) {
}

void MissingReturn::visit(RoutineDecl* node) {
}

void MissingReturn::visit(AliasedType* node) {
}

void MissingReturn::visit(IntegerType*) {
}

void MissingReturn::visit(RealType*) {
}

void MissingReturn::visit(BooleanType*) {
}

void MissingReturn::visit(ArrayType* node) {
}

void MissingReturn::visit(RecordType* node) {
}

void MissingReturn::visit(VariableDecl* node) {
}

void MissingReturn::visit(TypeDecl* node) {
}

void MissingReturn::visit(Body* node) {
}

void MissingReturn::visit(ReturnStatement* node) {
}

void MissingReturn::visit(Assignment* node) {
}

void MissingReturn::visit(WhileLoop* node) {
}

void MissingReturn::visit(ForLoop* node) {
}

void MissingReturn::visit(IfStatement* node) {
}

void MissingReturn::visit(UnaryExpression* node) {
}

void MissingReturn::visit(BinaryExpression* node) {
}

void MissingReturn::visit(IntegerLiteral* node) {
}

void MissingReturn::visit(RealLiteral* node) {
}

void MissingReturn::visit(BooleanLiteral* node) {
}

void MissingReturn::visit(Identifier* node) {
}

void MissingReturn::visit(RoutineCall* node) {
}

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
