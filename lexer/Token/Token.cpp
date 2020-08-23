#include <string>

enum class TokenType{
    VarDec, // var
    TypeDec, // type
    RoutineDec, // routine
    Is, // is
    IntegerType, // integer
    RealType, // real
    BooleanType, // boolean
    RecordType, // record
    ArrayType, // array
    True, // true
    False, // false
    WhileLoop, // while
    ForLoop, // for
    LoopBegin, // loop
    End, // end
    ReverseRange, // reverse
    InRange, // in
    If, // if
    Else, // else
    AndLogic, // and
    OrLogic, // or
    XorLogic, // xor
    SmallerComp, // <
    SeqComp, // <=
    BiggerComp, // >
    BeqComp, // >=
    EqComp, // =
    NeqComp, // /=
    MultOp, // *
    DevOp, // /
    RemainderOp, // %
    PlusOp, // +
    MinusOp, // -
    IntegerLiteral, // int const
    RealLiteral, // real const
    Identifier, // name
    Dot, // .
    TwoDots, // ..
    Comma, // ,
    BracketOpen, // (
    BracketClose, // )
    SquareBracketOpen, // [
    SquareBracketClose // ]
};

struct Token{
    TokenType code;
    int srcPos;
    std::string image;
};