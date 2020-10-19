#include "catch2/catch.hpp"
#include "catch_helpers.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "san.hpp"
#include <iterator>

/**
 * Since only the parser should be tested here, a dummy (stub) lexer
 *  is used to not be affected by bugs in the actual parser
 * TODO: This class is broken since the Lexer refactoring. It should be fixed
 *  such that it now overrides `scanNext` instead of `Next`
 */
class DummyLexer : public lexer::Lexer {

public:
    DummyLexer(std::initializer_list<lexer::Token> tokens) : lexer::Lexer("") {
        for (auto it = tokens.begin(); it != tokens.end(); it++) {
            m_tokens.push_back(*it);
        }
        it = m_tokens.begin();
    }

    lexer::Token Next() {
        if (it == m_tokens.end()) {
            return {.type = lexer::TokenType::Eof};
        }
        lexer::Token t = *it;
        it++;
        return t;
    }

    lexer::Token operator[](size_t idx) { return m_tokens.at(idx); }

private:
    std::vector<lexer::Token> m_tokens;
    std::vector<lexer::Token>::iterator it;
};

SCENARIO("Parser builds a tree from tokens") {
    GIVEN("A lexer that gets tokens") {
        WHEN("Tokens represent the expression '-2+6/3'") {
            lexer::Lexer lx{"-2+6/3"};

            /* --------Tree "-2+6/3"
            - [BinaryExpression]> +
            |- [UnaryExpression]> -
            ||- [IntegerLiteral]> 2
            |- [BinaryExpression]> /
            ||- [IntegerLiteral]> 6
            ||- [IntegerLiteral]> 3
            */

            THEN("Order of operations is preserved") {
                parser::Parser parser(lx);
                auto tree = parser.parseExpression();

                // visitors::PrintVisitor v;
                // tree->accept(v);

                std::shared_ptr<ast::BinaryExpression> rootNode =
                    std::dynamic_pointer_cast<ast::BinaryExpression>(tree);
                REQUIRE(rootNode != nullptr);
                REQUIRE(rootNode->operation == lexer::TokenType::Add);

                // Left
                std::shared_ptr<ast::UnaryExpression> leftChild =
                    std::dynamic_pointer_cast<ast::UnaryExpression>(
                        rootNode->operand1);
                REQUIRE(leftChild != nullptr);
                REQUIRE(leftChild->operation == lexer::TokenType::Sub);
                std::shared_ptr<ast::IntegerLiteral> leftLeftChild =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(
                        leftChild->operand);
                REQUIRE(leftLeftChild != nullptr);
                REQUIRE(leftLeftChild->value == 2);

                // Right
                std::shared_ptr<ast::BinaryExpression> rightChild =
                    std::dynamic_pointer_cast<ast::BinaryExpression>(
                        rootNode->operand2);
                REQUIRE(rightChild != nullptr);
                REQUIRE(rightChild->operation == lexer::TokenType::Div);
                std::shared_ptr<ast::IntegerLiteral> rightLeftChild =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(
                        rightChild->operand1);
                REQUIRE(rightLeftChild != nullptr);
                REQUIRE(rightLeftChild->value == 6);
                std::shared_ptr<ast::IntegerLiteral> rightRightChild =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(
                        rightChild->operand2);
                REQUIRE(rightRightChild != nullptr);
                REQUIRE(rightRightChild->value == 3);
            }
        }

        WHEN("Tokens represent a routine") {
            lexer::Lexer lx{"routine main(num : integer) : integer is\nvar a "
                            "is 1;\nreturn a;\nend"};
            /*
            routine main(num : integer) : integer is
                var a is 1;
                return a;
            end

            -----------Tree

            - [RoutineDecl]> main
            |- [VariableDecl]> num
            ||- [IntegerType]
            |- [Body]>
            ||- [VariableDecl]> a
            |||- [IntegerLiteral]> 1
            ||- [Return]>
            |||- [Identifier]> a
            |- [IntegerType]
            */

            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseRoutineDecl();

                std::shared_ptr<ast::RoutineDecl> routine =
                    std::dynamic_pointer_cast<ast::RoutineDecl>(tree);
                REQUIRE(routine != nullptr);
                REQUIRE(routine->name == "main");

                REQUIRE(routine->parameters.size() == 1);
                std::shared_ptr<ast::VariableDecl> parameters =
                    std::dynamic_pointer_cast<ast::VariableDecl>(
                        routine->parameters[0]);
                REQUIRE(parameters != nullptr);

                std::shared_ptr<ast::Body> body =
                    std::dynamic_pointer_cast<ast::Body>(routine->body);
                REQUIRE(body != nullptr);

                std::shared_ptr<ast::IntegerType> returnType =
                    std::dynamic_pointer_cast<ast::IntegerType>(
                        routine->returnType);
                REQUIRE(returnType != nullptr);
            }
        }

        WHEN("The routine is not named") {
            lexer::Lexer lx{"routine (num : integer) : integer is\nvar a is "
                            "1;\nreturn a;\nend"};
            /*
            routine (num : integer) : integer is
                var a is 1;
                return a;
            end
            */
            THEN("An error is reported") {
                parser::Parser parser(lx);
                auto tree = parser.parseRoutineDecl();
                auto errors = parser.getErrors();

                REQUIRE(!errors.empty());
                REQUIRE(errors[0].message == "expected identifier, got '('");
            }
        }

        WHEN("Tokens represent a parameter \"x: array integer\"") {
            lexer::Lexer lx{"x: array integer"};

            /* ------Tree

            - [VariableDecl]> x
            |- [ArrayType]>
            ||- [IntegerType]

            */

            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseParameter();

                std::shared_ptr<ast::VariableDecl> x =
                    std::dynamic_pointer_cast<ast::VariableDecl>(tree);
                REQUIRE(x != nullptr);
                REQUIRE(x->name == "x");

                std::shared_ptr<ast::ArrayType> arrayType =
                    std::dynamic_pointer_cast<ast::ArrayType>(x->type);
                REQUIRE(arrayType != nullptr);
                std::shared_ptr<ast::IntegerType> type =
                    std::dynamic_pointer_cast<ast::IntegerType>(
                        arrayType->elementType);
                REQUIRE(arrayType != nullptr);
            }
        }

        WHEN("Tokens represent an array type \"array [ 5+3 ] real\"") {
            lexer::Lexer lx{"array [ 5+3 ] real"};

            /*-----------Tree
            - [ArrayType]>
            |- [BinaryExpression]> +
            ||- [IntegerLiteral]> 5
            ||- [IntegerLiteral]> 3
            |- [RealType]
            */

            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseType();

                // visitors::PrintVisitor v;
                // tree->accept(v);

                std::shared_ptr<ast::ArrayType> array =
                    std::dynamic_pointer_cast<ast::ArrayType>(tree);
                REQUIRE(array != nullptr);
                std::shared_ptr<ast::BinaryExpression> length =
                    std::dynamic_pointer_cast<ast::BinaryExpression>(
                        array->length);
                REQUIRE(length != nullptr);
                REQUIRE(length->operation == lexer::TokenType::Add);
                std::shared_ptr<ast::IntegerLiteral> leftChild =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(
                        length->operand1);
                REQUIRE(leftChild != nullptr);
                REQUIRE(leftChild->value == 5);
                std::shared_ptr<ast::IntegerLiteral> rightChild =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(
                        length->operand2);
                REQUIRE(rightChild != nullptr);
                REQUIRE(rightChild->value == 3);

                std::shared_ptr<ast::RealType> el_type =
                    std::dynamic_pointer_cast<ast::RealType>(
                        array->elementType);
                REQUIRE(el_type != nullptr);
            }
        }

        WHEN("Tokens represent a for loop with reverse") {
            lexer::Lexer lx{
                "for i in reverse 1..4 loop \n sum := sum + i; \n end"};
            /*
            for i in reverse 1..4 loop
                sum := i + sum;
            end

            ----------------Tree

            - [ForLoop]>
            |- [IntegerLiteral]> 1
            |- [IntegerLiteral]> 4
            |- [Body]>
            ||- [Assignment]>
            |||- [Identifier]> sum
            |||- [BinaryExpression]> +
            ||||- [Identifier]> sum
            ||||- [Identifier]> i
            */

            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseForLoop();

                std::shared_ptr<ast::ForLoop> forloop =
                    std::dynamic_pointer_cast<ast::ForLoop>(tree);
                REQUIRE(forloop != nullptr);
                REQUIRE(forloop->reverse == true);

                std::shared_ptr<ast::IntegerLiteral> rangefrom =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(
                        forloop->rangeFrom);
                REQUIRE(rangefrom != nullptr);
                REQUIRE(rangefrom->value == 1);

                std::shared_ptr<ast::IntegerLiteral> rangeto =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(
                        forloop->rangeTo);
                REQUIRE(rangeto != nullptr);
                REQUIRE(rangeto->value == 4);

                std::shared_ptr<ast::Body> body =
                    std::dynamic_pointer_cast<ast::Body>(forloop->body);
                REQUIRE(body != nullptr);
            }
        }

        WHEN("Tokens represent a while loop") {
            lexer::Lexer lx{"while (sum < 5) loop \nsum := sum + 1;\nend"};
            /*
            while (sum < 5) loop
                sum := sum + 1;
            end

            ----------------Tree

            - [WhileLoop]>
            |- [BinaryExpression]> <
            ||- [Identifier]> sum
            ||- [IntegerLiteral]> 5
            |- [Body]>
            ||- [Assignment]>
            |||- [Identifier]> sum
            |||- [BinaryExpression]> +
            ||||- [Identifier]> sum
            ||||- [IntegerLiteral]> 1
            */
            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseWhileLoop();

                std::shared_ptr<ast::WhileLoop> whileloop =
                    std::dynamic_pointer_cast<ast::WhileLoop>(tree);
                REQUIRE(whileloop != nullptr);

                std::shared_ptr<ast::BinaryExpression> condition =
                    std::dynamic_pointer_cast<ast::BinaryExpression>(
                        whileloop->condition);
                REQUIRE(condition != nullptr);
                REQUIRE(condition->operation == lexer::TokenType::Less);

                std::shared_ptr<ast::Body> body =
                    std::dynamic_pointer_cast<ast::Body>(whileloop->body);
                REQUIRE(body != nullptr);
            }
        }
        WHEN("Tokens represent an if-else statement") {
            lexer::Lexer lx{
                "if num /= 1 then\nreturn num;\nelse\nreturn 1;\nend"};
            /*
            if num /= 1 then
                return num;
            else
                return 1;
            end

            ----------------Tree

            - [IfStatement]>
            |- [BinaryExpression]> /=
            ||- [Identifier]> num
            ||- [IntegerLiteral]> 1
            |- [Body]>
            ||- [Return]>
            |||- [Identifier]> num
            |- [Body]>
            ||- [Return]>
            |||- [IntegerLiteral]> 1
            */
            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseIfStatement();

                std::shared_ptr<ast::IfStatement> ifst =
                    std::dynamic_pointer_cast<ast::IfStatement>(tree);
                REQUIRE(ifst != nullptr);

                std::shared_ptr<ast::BinaryExpression> condition =
                    std::dynamic_pointer_cast<ast::BinaryExpression>(
                        ifst->condition);
                REQUIRE(condition != nullptr);
                REQUIRE(condition->operation == lexer::TokenType::Neq);

                std::shared_ptr<ast::Body> thenb =
                    std::dynamic_pointer_cast<ast::Body>(ifst->ifBody);
                REQUIRE(thenb != nullptr);

                std::shared_ptr<ast::Body> elseb =
                    std::dynamic_pointer_cast<ast::Body>(ifst->elseBody);
                REQUIRE(elseb != nullptr);
            }
        }
        WHEN("Tokens represent an if seatement without else") {
            lexer::Lexer lx{"if num /= 1 then\nreturn num;\nend"};
            /*
            if num /= 1 then
                return num;
            end
            -----------------Tree
            - [IfStatement]>
            |- [BinaryExpression]> /=
            ||- [Identifier]> num
            ||- [IntegerLiteral]> 1
            |- [Body]>
            ||- [Return]>
            |||- [Identifier]> num
            */
            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseIfStatement();

                std::shared_ptr<ast::IfStatement> ifst =
                    std::dynamic_pointer_cast<ast::IfStatement>(tree);
                REQUIRE(ifst != nullptr);

                std::shared_ptr<ast::BinaryExpression> condition =
                    std::dynamic_pointer_cast<ast::BinaryExpression>(
                        ifst->condition);
                REQUIRE(condition != nullptr);
                REQUIRE(condition->operation == lexer::TokenType::Neq);

                std::shared_ptr<ast::Body> thenb =
                    std::dynamic_pointer_cast<ast::Body>(ifst->ifBody);
                REQUIRE(thenb != nullptr);

                std::shared_ptr<ast::Body> elseb =
                    std::dynamic_pointer_cast<ast::Body>(ifst->elseBody);
                REQUIRE(elseb == nullptr);
            }
        }
        WHEN("Tokens represent a body") {
            lexer::Lexer lx{"type int is integer\nvar a : int\na := "
                            "5;\nrout(a);\nb := a;\nend"};
            /*
            type int is integer
            var a : int
            a := 5;
            rout(a);
            b := a;
            end
            -----------------Tree
            - [Body]>
            |- [TypeDecl]> int
            ||- [IntegerType]
            |- [VariableDecl]> a
            ||- [Type Identifier]> int
            |- [Assignment]>
            ||- [Identifier]> a
            ||- [IntegerLiteral]> 5
            |- [RoutineCall]> rout
            ||- [Identifier]> a
            |- [Assignment]>
            ||- [Identifier]> b
            ||- [Identifier]> a
            */
            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseBody();

                // visitors::PrintVisitor v;
                // tree->accept(v);

                std::shared_ptr<ast::Body> body =
                    std::dynamic_pointer_cast<ast::Body>(tree);
                REQUIRE(body != nullptr);
                REQUIRE(body->statements.size() == 3);
                REQUIRE(body->variables.size() == 1);
                REQUIRE(body->types.size() == 1);

                std::shared_ptr<ast::Assignment> as1 =
                    std::dynamic_pointer_cast<ast::Assignment>(
                        body->statements[0]);
                REQUIRE(as1 != nullptr);
                std::shared_ptr<ast::Identifier> a1 =
                    std::dynamic_pointer_cast<ast::Identifier>(as1->lhs);
                REQUIRE(a1 != nullptr);
                REQUIRE(a1->name == "a");
                std::shared_ptr<ast::IntegerLiteral> five =
                    std::dynamic_pointer_cast<ast::IntegerLiteral>(as1->rhs);
                REQUIRE(five != nullptr);
                REQUIRE(five->value == 5);

                std::shared_ptr<ast::Assignment> as2 =
                    std::dynamic_pointer_cast<ast::Assignment>(
                        body->statements[2]);
                REQUIRE(as2 != nullptr);
                std::shared_ptr<ast::Identifier> b1 =
                    std::dynamic_pointer_cast<ast::Identifier>(as2->lhs);
                REQUIRE(b1 != nullptr);
                REQUIRE(b1->name == "b");
                std::shared_ptr<ast::Identifier> a2 =
                    std::dynamic_pointer_cast<ast::Identifier>(as2->rhs);
                REQUIRE(a2 != nullptr);
                REQUIRE(a2->name == "a");

                std::shared_ptr<ast::RoutineCall> rc =
                    std::dynamic_pointer_cast<ast::RoutineCall>(
                        body->statements[1]);
                REQUIRE(rc != nullptr);
                REQUIRE(rc->routineName == "rout");
                REQUIRE(rc->args.size() == 1);
                std::shared_ptr<ast::Identifier> identifier =
                    std::dynamic_pointer_cast<ast::Identifier>(rc->args[0]);
                REQUIRE(identifier != nullptr);
                REQUIRE(identifier->name == "a");
            }
        }
        WHEN("Tokens represent a type declaration") {
            lexer::Lexer lx{"type int is integer;"};
            /*
            type int is integer;

            -----------------Tree

            - [TypeDecl]> int
            |- [IntegerType]
            */
            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseTypeDecl();

                std::shared_ptr<ast::TypeDecl> type =
                    std::dynamic_pointer_cast<ast::TypeDecl>(tree);
                REQUIRE(type != nullptr);
                REQUIRE(type->name == "int");
                std::shared_ptr<ast::IntegerType> t =
                    std::dynamic_pointer_cast<ast::IntegerType>(type->type);
                REQUIRE(t != nullptr);
            }
        }
        WHEN("Tokens represent a program") {
            lexer::Lexer lx{
                "type vector is array [5] integer\nvar a : vector\nvar b : "
                "float\nroutine main(m : integer) is\nb := 5.2\nvar i is "
                "1\nwhile i <= 5 loop\na[i] := b\nb := b + 0.5\nend\nend"};
            /*
            type vector is array [5] integer
            var a : vector
            var b : float

            routine main(m : integer) is
                b := 5.2
                var i is 1
                while i <= 5 loop
                    a[i] := b
                    b := b + 0.5
                end
            end
            -----------------Tree

            - [Program]>
            |- [TypeDecl]> vector
            ||- [ArrayType]>
            |||- [IntegerLiteral]> 5
            |||- [IntegerType]
            |- [VariableDecl]> a
            ||- [Type Identifier]> vector
            |- [VariableDecl]> b
            ||- [Type Identifier]> float
            |- [RoutineDecl]> main
            ||- [VariableDecl]> m
            |||- [IntegerType]
            ||- [Body]>
            |||- [VariableDecl]> i
            ||||- [IntegerLiteral]> 1
            |||- [Assignment]>
            ||||- [Identifier]> b
            ||||- [RealLiteral]> 5.2
            |||- [WhileLoop]>
            ||||- [BinaryExpression]> <=
            |||||- [Identifier]> i
            |||||- [IntegerLiteral]> 5
            ||||- [Body]>
            |||||- [Assignment]>
            ||||||- [BinaryExpression]> []
            |||||||- [Identifier]> a
            |||||||- [Identifier]> i
            ||||||- [Identifier]> b
            |||||- [Assignment]>
            ||||||- [Identifier]> b
            ||||||- [BinaryExpression]> +
            |||||||- [Identifier]> b
            |||||||- [RealLiteral]> 0.5
            */
            THEN("It is parsed correctly") {
                parser::Parser parser(lx);
                auto tree = parser.parseProgram();
            }
        }
    }
}
