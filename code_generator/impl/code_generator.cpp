#include "code_generator.hpp"

using namespace llvm;

namespace cg {

CodeGenerator::CodeGenerator(std::string name) : m_builder(m_context) {
    m_module = std::make_unique<Module>(name, m_context);
    auto TargetTriple = sys::getDefaultTargetTriple();
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        errs() << Error;
        return;
    }
    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    m_targetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    m_module->setDataLayout(m_targetMachine->createDataLayout());
    m_module->setTargetTriple(TargetTriple);
}

void CodeGenerator::visit(ast::Program* node) {
    for (auto& type : node->types) {
        type->accept(*this);
    }
    for (auto& var : node->variables) {
        var->accept(*this);
    }
    for (auto& routine : node->routines) {
        routine->accept(*this);
    }
}

void CodeGenerator::visit(ast::RoutineDecl* node) {
    // Types of the parameters
    std::vector<Type*> paramTypes;
    for (auto& param : node->parameters) {
        param->type->accept(*this);
        paramTypes.push_back(extractTempType());
    }
    // The routine's return type
    Type* returnType = Type::getVoidTy(m_context);
    if (node->returnType != nullptr) {
        node->returnType->accept(*this);
        returnType = extractTempType();
    }

    // The function's type: (return type, parameter types, varargs?)
    FunctionType* FT = FunctionType::get(returnType, paramTypes, false);

    // The actual function from the type, local to this module, with the
    //  given name
    Function* F = Function::Create(FT, Function::ExternalLinkage, node->name,
                                   m_module.get());

    // Give the parameters their names
    unsigned idx = 0;
    for (auto& arg : F->args()) {
        arg.setName(node->parameters[idx++]->name);
    }

    // Create a new basic block to start inserting into
    BasicBlock* BB = BasicBlock::Create(m_context, "entry", F);
    // Tell the builder that upcoming instructions should go into this block
    m_builder.SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map
    m_namedValues.clear();
    for (auto& Arg : F->args()) {
        m_namedValues[Arg.getName()] = &Arg;
    }

    node->body->accept(*this);

    verifyFunction(*F);

    tempVal = F;
}

void CodeGenerator::visit(ast::AliasedType*) {
    // Nothing to do here.
    // All aliased types should have been already replaced
}

void CodeGenerator::visit(ast::IntegerType* node) {
    tempType = Type::getInt64Ty(m_context);
}

void CodeGenerator::visit(ast::RealType* node) {
    tempType = Type::getDoubleTy(m_context);
}

void CodeGenerator::visit(ast::BooleanType* node) {
    tempType = Type::getInt1Ty(m_context);
}

void CodeGenerator::visit(ast::ArrayType* node) {
    // TODO
}

void CodeGenerator::visit(ast::RecordType* node) {
    // TODO
}

void CodeGenerator::visit(ast::VariableDecl* node) {
    node->type->accept(*this);
    auto type = extractTempType();
    auto v = m_builder.CreateAlloca(type, 0, node->name);

    if (node->type->getTypeKind() == ast::TypeKind::Array ||
        node->type->getTypeKind() == ast::TypeKind::Record) {
        node->type->accept(*this);
        m_builder.CreateStore(extractTempVal(), v);
    } else {
        if (node->initialValue != nullptr) {
            node->initialValue->accept(*this);
            m_builder.CreateStore(extractTempVal(), v);
        }
    }
    m_namedValues[node->name] = v;

    tempVal = v;
}

void CodeGenerator::visit(ast::TypeDecl* node) {
    // Nothing to do here. all type aliases should have already been replaced
}

void CodeGenerator::visit(ast::Body* node) {
    for (auto& type : node->types) {
        type->accept(*this);
    }
    for (auto& var : node->variables) {
        var->accept(*this);
    }
    for (auto& statement : node->statements) {
        statement->accept(*this);
    }
}

void CodeGenerator::visit(ast::ReturnStatement* node) {
    Value* returnValue = nullptr;
    if (node->expression != nullptr) {
        node->expression->accept(*this);
        returnValue = extractTempVal();
    }
    m_builder.CreateRet(returnValue);
}

void CodeGenerator::visit(ast::Assignment* node) {
    node->lhs->accept(*this);
    auto lhs = extractTempVal();
    node->rhs->accept(*this);
    auto rhs = extractTempVal();
    m_builder.CreateStore(rhs, lhs);
}

void CodeGenerator::visit(ast::WhileLoop* node) {
    Function* TheFunction = m_builder.GetInsertBlock()->getParent();
    // BasicBlock* PreheaderBB = m_builder.GetInsertBlock();
    BasicBlock* conditionBB =
        BasicBlock::Create(m_context, "condition", TheFunction);
    BasicBlock* loopBB = BasicBlock::Create(m_context, "loop");
    BasicBlock* endBB = BasicBlock::Create(m_context, "loopend");

    // Insert an explicit fall through from the current block to the LoopBB.
    m_builder.CreateBr(conditionBB);
    // Start insertion in conditionBB.
    m_builder.SetInsertPoint(conditionBB);
    node->condition->accept(*this);
    Value* condition = extractTempVal();
    condition = m_builder.CreateICmpNE(
        condition, ConstantInt::get(m_context, APInt(1, 0)), "loopcond");
    m_builder.CreateCondBr(condition, loopBB, endBB);

    TheFunction->getBasicBlockList().push_back(loopBB);

    m_builder.SetInsertPoint(loopBB);
    node->body->accept(*this);

    m_builder.CreateBr(conditionBB);

    TheFunction->getBasicBlockList().push_back(endBB);
    m_builder.SetInsertPoint(endBB);
}

void CodeGenerator::visit(ast::ForLoop* node) {
    // TODO: make this function work!!
    Function* func = m_builder.GetInsertBlock()->getParent();
    BasicBlock* condBB = BasicBlock::Create(m_context, "forloopcond", func);
    BasicBlock* loopBB = BasicBlock::Create(m_context, "forloop");
    BasicBlock* endBB = BasicBlock::Create(m_context, "forloopend");

    node->loopVar->accept(*this);
    auto loopVar = extractTempVal();

    node->rangeFrom->accept(*this);
    auto rangeFrom = extractTempVal();
    m_builder.CreateStore(rangeFrom, loopVar);

    node->rangeTo->accept(*this);
    auto rangeTo = extractTempVal();
    auto endexpr =
        m_builder.CreateAlloca(Type::getInt64Ty(m_context), 0, "endexpr");
    m_builder.CreateStore(rangeTo, endexpr);

    m_builder.CreateBr(condBB);
    m_builder.SetInsertPoint(condBB);

    auto i = m_builder.CreateLoad(loopVar, "loopvar");
    auto pred = m_builder.CreateLoad(endexpr, "pred");
    if (node->reverse) {
        tempVal = m_builder.CreateICmpSGT(i, pred, "comp");
    } else {
        tempVal = m_builder.CreateICmpSLT(i, pred, "comp");
    }
    m_builder.CreateCondBr(tempVal, loopBB, endBB);

    func->getBasicBlockList().push_back(loopBB);
    m_builder.SetInsertPoint(loopBB);

    node->body->accept(*this);

    i = m_builder.CreateLoad(loopVar, "loopvarIncr");
    llvm::Value* incr;
    if (node->reverse) {
        incr = m_builder.CreateSub(
            i, ConstantInt::get(m_context, APInt(64, 1, true)));
    } else {
        incr = m_builder.CreateAdd(
            i, ConstantInt::get(m_context, APInt(64, 1, true)));
    }
    m_builder.CreateStore(incr, loopVar);
    m_builder.CreateBr(condBB);
    func->getBasicBlockList().push_back(endBB);

    m_namedValues.erase(node->loopVar->name);
    m_builder.SetInsertPoint(endBB);
}

void CodeGenerator::visit(ast::IfStatement* node) {
    node->condition->accept(*this);
    Value* condition = extractTempVal();
    if (condition == nullptr) {
        error(node->condition->begin, "Cannot understand the condition");
        return;
    }

    if (condition->getType()->isFloatingPointTy()) {
        error(node->condition->begin, "Cannot cast real to boolean");
        return;
    }
    // Convert condition to a bool by comparing it to 0
    condition = m_builder.CreateICmpNE(
        condition, ConstantInt::get(m_context, APInt(1, 0)), "ifcond");

    // Get the current function
    Function* func = m_builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.
    // Insert the 'then' block at the end of the function.
    bool hasElse = node->elseBody != nullptr;
    BasicBlock* thenBB = BasicBlock::Create(m_context, "then", func);
    BasicBlock* elseBB =
        hasElse ? BasicBlock::Create(m_context, "else") : nullptr;
    BasicBlock* mergeBB = BasicBlock::Create(m_context, "endif");

    m_builder.CreateCondBr(condition, thenBB, hasElse ? elseBB : mergeBB);

    // Start inserting into the "then" block
    m_builder.SetInsertPoint(thenBB);

    node->ifBody->accept(*this);

    if (hasElse) {
        // Jump to after the "else" block
        m_builder.CreateBr(mergeBB);
    }

    // Code of "then" can change current block; update thenBB for the PHI
    thenBB = m_builder.GetInsertBlock();

    if (hasElse) {
        // Emit else block
        func->getBasicBlockList().push_back(elseBB);
        m_builder.SetInsertPoint(elseBB);

        node->elseBody->accept(*this);
        // m_builder.CreateBr(mergeBB); // should not need to explicitly branch

        // Code of 'else' can change the current block; update elseBB for PHI
        // elseBB = m_builder.GetInsertBlock(); // we don't use PHI
    }

    // Emit merge block
    func->getBasicBlockList().push_back(mergeBB);
    m_builder.SetInsertPoint(mergeBB);
}

void CodeGenerator::visit(ast::UnaryExpression* node) {
    // TODO
}

void CodeGenerator::visit(ast::BinaryExpression* node) {
    Value *L, *R;
    node->operand1->accept(*this);
    L = extractTempVal();

    node->operand2->accept(*this);
    R = extractTempVal();

    if (L == nullptr || R == nullptr) {
        error(node->begin, "a binary expression needs both operands");
        return;
    }

    switch (node->operation) {
    case lexer::TokenType::Add:
        if (anyIsFloat({R, L})) {
            tempVal = m_builder.CreateFAdd(L, R, "addtmp");
        } else {
            tempVal = m_builder.CreateAdd(L, R, "addtmp");
        }
        return;
    case lexer::TokenType::Sub:
        if (anyIsFloat({R, L})) {
            tempVal = m_builder.CreateFSub(L, R, "subtmp");
        } else {
            tempVal = m_builder.CreateSub(L, R, "subtmp");
        }
        return;
    case lexer::TokenType::Mul:
        if (anyIsFloat({R, L})) {
            tempVal = m_builder.CreateFMul(L, R, "multmp");
        } else {
            tempVal = m_builder.CreateMul(L, R, "multmp");
        }
        return;
    // Note: '<' requires both operands to be of the same type
    case lexer::TokenType::Less:
        if (anyIsFloat({R, L})) {
            tempVal = m_builder.CreateFCmpOLT(L, R, "cmptmp");
        } else {
            tempVal = m_builder.CreateICmpSLT(L, R, "cmptmp");
        }
        return;
    // TODO: /, %, =, !=, ., [], <=, >, >=
    default:
        error(node->begin, "invalid binary operator");
        return;
    }
}

void CodeGenerator::visit(ast::IntegerLiteral* node) {
    tempVal = ConstantInt::get(m_context, APInt(64, node->value));
}

void CodeGenerator::visit(ast::RealLiteral* node) {
    tempVal = ConstantFP::get(m_context, APFloat(node->value));
}

void CodeGenerator::visit(ast::BooleanLiteral* node) {
    tempVal = ConstantInt::get(m_context, APInt(1, node->value));
}

void CodeGenerator::visit(ast::Identifier* node) {
    auto V = m_namedValues[node->name];
    if (V == nullptr) {
        error(node->begin, "Unknown variable name");
    }
    tempVal = V;
}

void CodeGenerator::visit(ast::RoutineCall* node) {
    Function* CalleeF = m_module->getFunction(node->routineName);
    if (CalleeF == nullptr) {
        error(node->begin, "unknown function referenced");
        return;
    }

    if (CalleeF->arg_size() != node->args.size()) {
        error(node->begin, "expected {} but got {} parameters",
              CalleeF->arg_size(), node->args.size());
        return;
    }

    std::vector<Value*> args;
    for (auto& arg : node->args) {
        arg->accept(*this);
        Value* argCode = extractTempVal();
        if (argCode == nullptr) {
            error(arg->begin, "what is this?");
            return;
        }
        args.push_back(argCode);
    }

    tempVal = m_builder.CreateCall(CalleeF, args, "calltmp");
}

void CodeGenerator::emitCode(std::string filename) {
    std::error_code EC;
    raw_fd_ostream dest(filename, EC, sys::fs::OF_None);

    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return;
    }

    legacy::PassManager pass;
    auto FileType = CGFT_ObjectFile;

    if (m_targetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        errs() << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*m_module);
    dest.flush();
}

Module::FunctionListType& CodeGenerator::getFunctions() {
    return m_module->getFunctionList();
}

} // namespace cg
