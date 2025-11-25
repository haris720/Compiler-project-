#include "ir_generator.h"
#include <iostream>

// Helper: Generate new temporary variable
std::string IRGenerator::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

// Helper: Generate new label
std::string IRGenerator::newLabel() {
    return "L" + std::to_string(labelCounter++);
}

// Main generation entry point
void IRGenerator::generate(const std::shared_ptr<ProgramNode>& program) {
    if (!program) return;
    
    std::cout << "\n=== PHASE 5: IR GENERATION ===\n" << std::endl;
    
    // Generate IR for each function
    for (const auto& function : program->functions) {
        generateFunction(function);
    }
    
    std::cout << "IR Generation completed successfully.\n" << std::endl;
}

// Generate IR for function
void IRGenerator::generateFunction(const std::shared_ptr<FunctionNode>& function) {
    if (!function) return;
    
    currentFunction = function->name;
    
    // Emit function label
    emit(TACInstruction(TACOpcode::LABEL, "func_" + function->name));
    
    // Generate IR for function body statements
    if (function->body) {
        for (const auto& stmt : function->body->statements) {
            generateStatement(stmt);
        }
    }
    
    // Emit return void if no explicit return
    if (function->returnType == "void") {
        emit(TACInstruction(TACOpcode::RETURN_VOID));
    }
    
    emit(TACInstruction(TACOpcode::NOP)); // Separator
}

// Generate IR for statements
void IRGenerator::generateStatement(const std::shared_ptr<StatementNode>& stmt) {
    if (!stmt) return;
    
    if (auto varDecl = std::dynamic_pointer_cast<VariableDeclarationNode>(stmt)) {
        generateVariableDeclaration(varDecl);
    }
    else if (auto assignment = std::dynamic_pointer_cast<AssignmentNode>(stmt)) {
        generateAssignment(assignment);
    }
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(stmt)) {
        generateIfStatement(ifStmt);
    }
    else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(stmt)) {
        generateWhileStatement(whileStmt);
    }
    else if (auto forStmt = std::dynamic_pointer_cast<ForStatementNode>(stmt)) {
        generateForStatement(forStmt);
    }
    else if (auto switchStmt = std::dynamic_pointer_cast<SwitchStatementNode>(stmt)) {
        generateSwitchStatement(switchStmt);
    }
    else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStatementNode>(stmt)) {
        generateSwitchStatement(switchStmt);
    }
    else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStatementNode>(stmt)) {
        generateReturnStatement(returnStmt);
    }
    else if (auto printStmt = std::dynamic_pointer_cast<PrintStatementNode>(stmt)) {
        generatePrintStatement(printStmt);
    }
    else if (auto breakStmt = std::dynamic_pointer_cast<BreakStatementNode>(stmt)) {
        generateBreakStatement(breakStmt);
    }
    else if (auto continueStmt = std::dynamic_pointer_cast<ContinueStatementNode>(stmt)) {
        generateContinueStatement(continueStmt);
    }
    else if (auto block = std::dynamic_pointer_cast<BlockNode>(stmt)) {
        // Process block statements
        for (const auto& blockStmt : block->statements) {
            generateStatement(blockStmt);
        }
    }
}

// Generate IR for variable declaration
void IRGenerator::generateVariableDeclaration(const std::shared_ptr<VariableDeclarationNode>& varDecl) {
    if (!varDecl) return;
    
    if (varDecl->initializer) {
        std::string initValue = generateExpression(varDecl->initializer);
        emit(TACInstruction(TACOpcode::ASSIGN, varDecl->name, initValue));
    }
}

// Generate IR for assignment
void IRGenerator::generateAssignment(const std::shared_ptr<AssignmentNode>& assignment) {
    if (!assignment) return;
    
    std::string value = generateExpression(assignment->value);
    emit(TACInstruction(TACOpcode::ASSIGN, assignment->variable, value));
}

// Generate IR for if statement
void IRGenerator::generateIfStatement(const std::shared_ptr<IfStatementNode>& ifStmt) {
    if (!ifStmt) return;
    
    std::string condTemp = generateExpression(ifStmt->condition);
    std::string elseLabel = newLabel();
    std::string endLabel = newLabel();
    
    // if !condition goto elseLabel
    emit(TACInstruction(TACOpcode::IF_FALSE, condTemp, elseLabel));
    
    // Then branch
    if (ifStmt->thenBlock) {
        for (const auto& stmt : ifStmt->thenBlock->statements) {
            generateStatement(stmt);
        }
    }
    
    if (ifStmt->elseBlock) {
        // goto endLabel
        emit(TACInstruction(TACOpcode::GOTO, endLabel));
        
        // elseLabel:
        emit(TACInstruction(TACOpcode::LABEL, elseLabel));
        
        // Else branch
        for (const auto& stmt : ifStmt->elseBlock->statements) {
            generateStatement(stmt);
        }
        
        // endLabel:
        emit(TACInstruction(TACOpcode::LABEL, endLabel));
    } else {
        // elseLabel:
        emit(TACInstruction(TACOpcode::LABEL, elseLabel));
    }
}

// Generate IR for while statement
void IRGenerator::generateWhileStatement(const std::shared_ptr<WhileStatementNode>& whileStmt) {
    if (!whileStmt) return;
    
    std::string startLabel = newLabel();
    std::string endLabel = newLabel();
    
    // Push loop labels for break/continue
    pushLoopLabels(endLabel, startLabel);
    
    // startLabel:
    emit(TACInstruction(TACOpcode::LABEL, startLabel));
    
    // Evaluate condition
    std::string condTemp = generateExpression(whileStmt->condition);
    
    // if !condition goto endLabel
    emit(TACInstruction(TACOpcode::IF_FALSE, condTemp, endLabel));
    
    // Loop body
    if (whileStmt->body) {
        for (const auto& stmt : whileStmt->body->statements) {
            generateStatement(stmt);
        }
    }
    
    // goto startLabel
    emit(TACInstruction(TACOpcode::GOTO, startLabel));
    
    // endLabel:
    emit(TACInstruction(TACOpcode::LABEL, endLabel));
    
    popLoopLabels();
}

// Generate IR for for statement
void IRGenerator::generateForStatement(const std::shared_ptr<ForStatementNode>& forStmt) {
    if (!forStmt) return;
    
    // Initialization
    if (forStmt->initialization) {
        generateStatement(forStmt->initialization);
    }
    
    std::string startLabel = newLabel();
    std::string updateLabel = newLabel();
    std::string endLabel = newLabel();
    
    // Push loop labels for break/continue
    pushLoopLabels(endLabel, updateLabel);
    
    // startLabel:
    emit(TACInstruction(TACOpcode::LABEL, startLabel));
    
    // Condition check
    if (forStmt->condition) {
        std::string condTemp = generateExpression(forStmt->condition);
        emit(TACInstruction(TACOpcode::IF_FALSE, condTemp, endLabel));
    }
    
    // Loop body
    if (forStmt->body) {
        for (const auto& stmt : forStmt->body->statements) {
            generateStatement(stmt);
        }
    }
    
    // updateLabel:
    emit(TACInstruction(TACOpcode::LABEL, updateLabel));
    
    // Update expression
    if (forStmt->update) {
        generateStatement(forStmt->update);
    }
    
    // goto startLabel
    emit(TACInstruction(TACOpcode::GOTO, startLabel));
    
    // endLabel:
    emit(TACInstruction(TACOpcode::LABEL, endLabel));
    
    popLoopLabels();
}

// Generate IR for return statement
void IRGenerator::generateReturnStatement(const std::shared_ptr<ReturnStatementNode>& returnStmt) {
    if (!returnStmt) return;
    
    if (returnStmt->expression) {
        std::string retValue = generateExpression(returnStmt->expression);
        emit(TACInstruction(TACOpcode::RETURN, retValue));
    } else {
        emit(TACInstruction(TACOpcode::RETURN_VOID));
    }
}

// Generate IR for print statement
void IRGenerator::generatePrintStatement(const std::shared_ptr<PrintStatementNode>& printStmt) {
    if (!printStmt) return;
    
    std::string value = generateExpression(printStmt->expression);
    emit(TACInstruction(TACOpcode::PRINT, value));
}

// Generate IR for break statement
void IRGenerator::generateBreakStatement(const std::shared_ptr<BreakStatementNode>& breakStmt) {
    std::string breakLabel = getCurrentBreakLabel();
    if (!breakLabel.empty()) {
        emit(TACInstruction(TACOpcode::GOTO, breakLabel));
    }
}

// Generate IR for continue statement
void IRGenerator::generateContinueStatement(const std::shared_ptr<ContinueStatementNode>& continueStmt) {
    std::string continueLabel = getCurrentContinueLabel();
    if (!continueLabel.empty()) {
        emit(TACInstruction(TACOpcode::GOTO, continueLabel));
    }
}

// Generate IR for expressions
std::string IRGenerator::generateExpression(const std::shared_ptr<ExpressionNode>& expr) {
    if (!expr) return "";
    
    if (auto binOp = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
        return generateBinaryOp(binOp);
    }
    else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpNode>(expr)) {
        return generateUnaryOp(unaryOp);
    }
    else if (auto ternary = std::dynamic_pointer_cast<TernaryNode>(expr)) {
        return generateTernary(ternary);
    }
    else if (auto funcCall = std::dynamic_pointer_cast<FunctionCallNode>(expr)) {
        return generateFunctionCall(funcCall);
    }
    else if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(expr)) {
        return generateIdentifier(ident);
    }
    else if (auto literal = std::dynamic_pointer_cast<LiteralNode>(expr)) {
        return generateLiteral(literal);
    }
    
    return "";
}

// Generate IR for binary operations
std::string IRGenerator::generateBinaryOp(const std::shared_ptr<BinaryOpNode>& binOp) {
    if (!binOp) return "";
    
    std::string left = generateExpression(binOp->left);
    std::string right = generateExpression(binOp->right);
    std::string result = newTemp();
    
    TACOpcode opcode;
    
    // Map operator to TAC opcode
    if (binOp->op == "+") opcode = TACOpcode::ADD;
    else if (binOp->op == "-") opcode = TACOpcode::SUB;
    else if (binOp->op == "*") opcode = TACOpcode::MUL;
    else if (binOp->op == "/") opcode = TACOpcode::DIV;
    else if (binOp->op == "%") opcode = TACOpcode::MOD;
    else if (binOp->op == "&&") opcode = TACOpcode::AND;
    else if (binOp->op == "||") opcode = TACOpcode::OR;
    else if (binOp->op == "&") opcode = TACOpcode::BIT_AND;
    else if (binOp->op == "|") opcode = TACOpcode::BIT_OR;
    else if (binOp->op == "^") opcode = TACOpcode::BIT_XOR;
    else if (binOp->op == "<<") opcode = TACOpcode::LEFT_SHIFT;
    else if (binOp->op == ">>") opcode = TACOpcode::RIGHT_SHIFT;
    else if (binOp->op == "==") opcode = TACOpcode::EQ;
    else if (binOp->op == "!=") opcode = TACOpcode::NEQ;
    else if (binOp->op == "<") opcode = TACOpcode::LT;
    else if (binOp->op == "<=") opcode = TACOpcode::LE;
    else if (binOp->op == ">") opcode = TACOpcode::GT;
    else if (binOp->op == ">=") opcode = TACOpcode::GE;
    else {
        // Unknown operator, default to ADD
        opcode = TACOpcode::ADD;
    }
    
    emit(TACInstruction(opcode, result, left, right));
    return result;
}

// Generate IR for unary operations
std::string IRGenerator::generateUnaryOp(const std::shared_ptr<UnaryOpNode>& unaryOp) {
    if (!unaryOp) return "";
    
    std::string operand = generateExpression(unaryOp->operand);
    std::string result = newTemp();
    
    TACOpcode opcode;
    
    if (unaryOp->op == "-") opcode = TACOpcode::NEG;
    else if (unaryOp->op == "!") opcode = TACOpcode::NOT;
    else if (unaryOp->op == "~") opcode = TACOpcode::BIT_NOT;
    else {
        // Unknown operator
        opcode = TACOpcode::NOT;
    }
    
    emit(TACInstruction(opcode, result, operand));
    return result;
}

// Generate IR for ternary operator
std::string IRGenerator::generateTernary(const std::shared_ptr<TernaryNode>& ternary) {
    if (!ternary) return "";
    
    std::string condition = generateExpression(ternary->condition);
    std::string result = newTemp();
    std::string elseLabel = newLabel();
    std::string endLabel = newLabel();
    
    // if !condition goto elseLabel
    emit(TACInstruction(TACOpcode::IF_FALSE, condition, elseLabel));
    
    // True branch
    std::string trueValue = generateExpression(ternary->trueValue);
    emit(TACInstruction(TACOpcode::ASSIGN, result, trueValue));
    emit(TACInstruction(TACOpcode::GOTO, endLabel));
    
    // elseLabel:
    emit(TACInstruction(TACOpcode::LABEL, elseLabel));
    
    // False branch
    std::string falseValue = generateExpression(ternary->falseValue);
    emit(TACInstruction(TACOpcode::ASSIGN, result, falseValue));
    
    // endLabel:
    emit(TACInstruction(TACOpcode::LABEL, endLabel));
    
    return result;
}

// Generate IR for function call
std::string IRGenerator::generateFunctionCall(const std::shared_ptr<FunctionCallNode>& funcCall) {
    if (!funcCall) return "";
    
    // Evaluate arguments and emit PARAM instructions
    for (const auto& arg : funcCall->arguments) {
        std::string argValue = generateExpression(arg);
        emit(TACInstruction(TACOpcode::PARAM, argValue));
    }
    
    // Call function
    std::string result = newTemp();
    std::string argCount = std::to_string(funcCall->arguments.size());
    emit(TACInstruction(TACOpcode::CALL, result, funcCall->functionName, argCount));
    
    return result;
}

// Generate IR for identifier
std::string IRGenerator::generateIdentifier(const std::shared_ptr<IdentifierNode>& ident) {
    if (!ident) return "";
    return ident->name;
}

// Generate IR for literal
std::string IRGenerator::generateLiteral(const std::shared_ptr<LiteralNode>& literal) {
    if (!literal) return "";
    return literal->value;
}

// Generate IR for switch statement
void IRGenerator::generateSwitchStatement(const std::shared_ptr<SwitchStatementNode>& switchStmt) {
    if (!switchStmt) return;
    
    // Evaluate switch expression
    std::string switchValue = generateExpression(switchStmt->expression);
    
    std::string endLabel = newLabel();
    std::vector<std::string> caseLabels;
    std::string defaultLabel = switchStmt->defaultCase ? newLabel() : endLabel;
    
    // Generate labels for each case
    for (size_t i = 0; i < switchStmt->cases.size(); i++) {
        caseLabels.push_back(newLabel());
    }
    
    // Push break label (switch is valid for break)
    pushLoopLabels(endLabel, "");
    
    // Generate comparison for each case
    for (size_t i = 0; i < switchStmt->cases.size(); i++) {
        const auto& caseStmt = switchStmt->cases[i];
        std::string caseValue = generateExpression(caseStmt->value);
        std::string temp = newTemp();
        
        // Compare: temp = switchValue == caseValue
        emit(TACInstruction(TACOpcode::EQ, temp, switchValue, caseValue));
        
        // If match, jump to case label
        emit(TACInstruction(TACOpcode::IF_TRUE, temp, caseLabels[i]));
    }
    
    // If no case matched, jump to default (or end)
    emit(TACInstruction(TACOpcode::GOTO, defaultLabel));
    
    // Generate code for each case
    for (size_t i = 0; i < switchStmt->cases.size(); i++) {
        const auto& caseStmt = switchStmt->cases[i];
        
        // Case label
        emit(TACInstruction(TACOpcode::LABEL, caseLabels[i]));
        
        // Case statements
        for (const auto& stmt : caseStmt->statements) {
            generateStatement(stmt);
        }
    }
    
    // Generate default case if exists
    if (switchStmt->defaultCase) {
        emit(TACInstruction(TACOpcode::LABEL, defaultLabel));
        
        for (const auto& stmt : switchStmt->defaultCase->statements) {
            generateStatement(stmt);
        }
    }
    
    // End label
    emit(TACInstruction(TACOpcode::LABEL, endLabel));
    
    // Pop break label
    popLoopLabels();
}

// Print TAC instructions
void IRGenerator::printTAC() const {
    std::cout << "\n=== THREE-ADDRESS CODE (TAC) ===\n" << std::endl;
    
    int lineNumber = 1;
    for (const auto& instr : instructions) {
        std::cout << lineNumber++ << ": " << instr.toString() << std::endl;
    }
    
    std::cout << "\nTotal TAC instructions: " << instructions.size() << std::endl;
}
