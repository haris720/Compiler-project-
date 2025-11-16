#include "type_checker.h"
#include <iostream>
#include <sstream>

// Helper function to convert error enum to string
std::string typeErrorToString(TypeChkError error) {
    switch (error) {
        case TypeChkError::ErroneousVarDecl: return "ErroneousVarDecl";
        case TypeChkError::FnCallParamCount: return "FnCallParamCount";
        case TypeChkError::FnCallParamType: return "FnCallParamType";
        case TypeChkError::ErroneousReturnType: return "ErroneousReturnType";
        case TypeChkError::ExpressionTypeMismatch: return "ExpressionTypeMismatch";
        case TypeChkError::ExpectedBooleanExpression: return "ExpectedBooleanExpression";
        case TypeChkError::ErroneousBreak: return "ErroneousBreak";
        case TypeChkError::NonBooleanCondStmt: return "NonBooleanCondStmt";
        case TypeChkError::EmptyExpression: return "EmptyExpression";
        case TypeChkError::AttemptedBoolOpOnNonBools: return "AttemptedBoolOpOnNonBools";
        case TypeChkError::AttemptedBitOpOnNonNumeric: return "AttemptedBitOpOnNonNumeric";
        case TypeChkError::AttemptedShiftOnNonInt: return "AttemptedShiftOnNonInt";
        case TypeChkError::AttemptedAddOpOnNonNumeric: return "AttemptedAddOpOnNonNumeric";
        case TypeChkError::AttemptedExponentiationOfNonNumeric: return "AttemptedExponentiationOfNonNumeric";
        case TypeChkError::ReturnStmtNotFound: return "ReturnStmtNotFound";
        default: return "UnknownError";
    }
}

TypeChecker::TypeChecker(ScopeAnalyzer* analyzer) 
    : scopeAnalyzer(analyzer), currentFunctionReturnType("void"), 
      hasReturnStatement(false), loopDepth(0) {}

void TypeChecker::addError(TypeChkError errorType, const std::string& message) {
    errors.emplace_back(errorType, message, 0, 0);
}

void TypeChecker::addError(TypeChkError errorType, const std::string& message, int line, int col) {
    errors.emplace_back(errorType, message, line, col);
}

// Type helper methods
bool TypeChecker::isNumericType(const std::string& type) {
    return type == "int" || type == "float" || type == "double" || 
           type == "long" || type == "short" || type == "char";
}

bool TypeChecker::isIntegerType(const std::string& type) {
    return type == "int" || type == "long" || type == "short" || type == "char";
}

bool TypeChecker::isBooleanType(const std::string& type) {
    return type == "bool";
}

bool TypeChecker::areTypesCompatible(const std::string& type1, const std::string& type2) {
    if (type1 == type2) return true;
    
    // Error type is compatible with nothing
    if (type1 == "error" || type2 == "error") {
        return false;
    }
    
    // Numeric type promotions (int, float, etc.)
    if (isNumericType(type1) && isNumericType(type2)) {
        return true; // Allow implicit conversions between numeric types
    }
    
    // Boolean type is NOT compatible with numeric types
    if (isBooleanType(type1) || isBooleanType(type2)) {
        return false; // Strict: no bool-to-numeric or numeric-to-bool conversions
    }
    
    // Void is only compatible with void
    if (type1 == "void" || type2 == "void") {
        return type1 == type2;
    }
    
    return false;
}

std::string TypeChecker::promoteTypes(const std::string& type1, const std::string& type2) {
    if (type1 == type2) return type1;
    
    // Promotion hierarchy: char < short < int < long < float < double
    if (type1 == "double" || type2 == "double") return "double";
    if (type1 == "float" || type2 == "float") return "float";
    if (type1 == "long" || type2 == "long") return "long";
    if (type1 == "int" || type2 == "int") return "int";
    if (type1 == "short" || type2 == "short") return "short";
    
    return "int"; // Default promotion
}

// Expression type inference
std::string TypeChecker::inferLiteralType(const std::shared_ptr<LiteralNode>& literal) {
    if (!literal) return "error";
    
    // Check the value to determine type
    if (literal->type == "int" || literal->type == "long" || 
        literal->type == "short" || literal->type == "char") {
        return literal->type;
    }
    if (literal->type == "float" || literal->type == "double") {
        return literal->type;
    }
    if (literal->type == "bool") {
        return "bool";
    }
    if (literal->type == "string") {
        return "string";
    }
    
    return literal->type;
}

std::string TypeChecker::inferIdentifierType(const std::shared_ptr<IdentifierNode>& identifier) {
    if (!identifier) return "error";
    
    // Look up variable type in symbol table (through scope analyzer)
    if (scopeAnalyzer) {
        std::string varType = scopeAnalyzer->getVariableType(identifier->name, currentFunctionName);
        if (varType != "error") {
            return varType;
        }
    }
    
    // If not found, might be a function parameter or error
    // For now, default to int as fallback (should ideally track all scopes)
    return "int";
}

std::string TypeChecker::inferBinaryOpType(const std::shared_ptr<BinaryOpNode>& binOp) {
    if (!binOp) return "error";
    
    std::string leftType = inferExpressionType(binOp->left);
    std::string rightType = inferExpressionType(binOp->right);
    
    if (leftType == "error" || rightType == "error") return "error";
    
    std::string op = binOp->op;
    
    // Arithmetic operators: +, -, *, /, %
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            addError(TypeChkError::AttemptedAddOpOnNonNumeric,
                    "Arithmetic operator '" + op + "' requires numeric operands, got " + 
                    leftType + " and " + rightType);
            return "error";
        }
        return promoteTypes(leftType, rightType);
    }
    
    if (op == "%") {
        if (!isIntegerType(leftType) || !isIntegerType(rightType)) {
            addError(TypeChkError::AttemptedAddOpOnNonNumeric,
                    "Modulo operator requires integer operands, got " + 
                    leftType + " and " + rightType);
            return "error";
        }
        return promoteTypes(leftType, rightType);
    }
    
    // Power operator
    if (op == "**" || op == "^") {
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            addError(TypeChkError::AttemptedExponentiationOfNonNumeric,
                    "Exponentiation operator requires numeric operands, got " + 
                    leftType + " and " + rightType);
            return "error";
        }
        return promoteTypes(leftType, rightType);
    }
    
    // Bitwise operators: &, |, ^, <<, >>
    if (op == "&" || op == "|" || op == "^") {
        if (!isIntegerType(leftType) || !isIntegerType(rightType)) {
            addError(TypeChkError::AttemptedBitOpOnNonNumeric,
                    "Bitwise operator '" + op + "' requires integer operands, got " + 
                    leftType + " and " + rightType);
            return "error";
        }
        return promoteTypes(leftType, rightType);
    }
    
    if (op == "<<" || op == ">>") {
        if (!isIntegerType(leftType) || !isIntegerType(rightType)) {
            addError(TypeChkError::AttemptedShiftOnNonInt,
                    "Shift operator '" + op + "' requires integer operands, got " + 
                    leftType + " and " + rightType);
            return "error";
        }
        return leftType; // Shift result has type of left operand
    }
    
    // Logical operators: &&, ||
    if (op == "&&" || op == "||") {
        if (!isBooleanType(leftType) || !isBooleanType(rightType)) {
            // In C, numeric types can be used as boolean (0 = false, non-zero = true)
            // But we'll be strict here
            if (!isNumericType(leftType) && !isBooleanType(leftType)) {
                addError(TypeChkError::AttemptedBoolOpOnNonBools,
                        "Logical operator '" + op + "' requires boolean or numeric operands, got " + leftType);
                return "error";
            }
            if (!isNumericType(rightType) && !isBooleanType(rightType)) {
                addError(TypeChkError::AttemptedBoolOpOnNonBools,
                        "Logical operator '" + op + "' requires boolean or numeric operands, got " + rightType);
                return "error";
            }
        }
        return "bool";
    }
    
    // Comparison operators: ==, !=, <, >, <=, >=
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (!areTypesCompatible(leftType, rightType)) {
            addError(TypeChkError::ExpressionTypeMismatch,
                    "Comparison operator '" + op + "' used on incompatible types: " + 
                    leftType + " and " + rightType);
            return "error";
        }
        return "bool";
    }
    
    // Assignment operators
    if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || 
        op == "%=" || op == "&=" || op == "|=" || op == "^=" || op == "<<=" || op == ">>=") {
        if (!areTypesCompatible(leftType, rightType)) {
            addError(TypeChkError::ExpressionTypeMismatch,
                    "Assignment operator '" + op + "' type mismatch: cannot assign " + 
                    rightType + " to " + leftType);
            return "error";
        }
        return leftType;
    }
    
    return "error";
}

std::string TypeChecker::inferUnaryOpType(const std::shared_ptr<UnaryOpNode>& unaryOp) {
    if (!unaryOp) return "error";
    
    std::string operandType = inferExpressionType(unaryOp->operand);
    if (operandType == "error") return "error";
    
    std::string op = unaryOp->op;
    
    // Arithmetic unary: +, -, ++, --
    if (op == "+" || op == "-" || op == "++" || op == "--") {
        if (!isNumericType(operandType)) {
            addError(TypeChkError::AttemptedAddOpOnNonNumeric,
                    "Unary operator '" + op + "' requires numeric operand, got " + operandType);
            return "error";
        }
        return operandType;
    }
    
    // Logical NOT: !
    if (op == "!") {
        if (!isBooleanType(operandType) && !isNumericType(operandType)) {
            addError(TypeChkError::AttemptedBoolOpOnNonBools,
                    "Logical NOT requires boolean or numeric operand, got " + operandType);
            return "error";
        }
        return "bool";
    }
    
    // Bitwise NOT: ~
    if (op == "~") {
        if (!isIntegerType(operandType)) {
            addError(TypeChkError::AttemptedBitOpOnNonNumeric,
                    "Bitwise NOT requires integer operand, got " + operandType);
            return "error";
        }
        return operandType;
    }
    
    return "error";
}

std::string TypeChecker::inferFunctionCallType(const std::shared_ptr<FunctionCallNode>& funcCall) {
    if (!funcCall) return "error";
    
    // Look up function signature
    auto it = functionSignatures.find(funcCall->functionName);
    if (it == functionSignatures.end()) {
        // Function not found (should have been caught by scope analysis)
        return "error";
    }
    
    const FunctionSignature& sig = it->second;
    
    // Check parameter count
    if (funcCall->arguments.size() != sig.parameters.size()) {
        std::ostringstream oss;
        oss << "Function '" << funcCall->functionName << "' expects " 
            << sig.parameters.size() << " parameters, but " 
            << funcCall->arguments.size() << " were provided";
        addError(TypeChkError::FnCallParamCount, oss.str());
        return sig.returnType; // Return the expected return type anyway
    }
    
    // Check parameter types
    for (size_t i = 0; i < funcCall->arguments.size(); i++) {
        std::string argType = inferExpressionType(funcCall->arguments[i]);
        std::string expectedType = sig.parameters[i].first;
        
        if (!areTypesCompatible(expectedType, argType)) {
            std::ostringstream oss;
            oss << "Function '" << funcCall->functionName << "' parameter " << (i + 1)
                << " ('" << sig.parameters[i].second << "'): expected " << expectedType 
                << ", got " << argType;
            addError(TypeChkError::FnCallParamType, oss.str());
        }
    }
    
    return sig.returnType;
}

std::string TypeChecker::inferTernaryType(const std::shared_ptr<TernaryNode>& ternary) {
    if (!ternary) return "error";
    
    std::string condType = inferExpressionType(ternary->condition);
    std::string trueType = inferExpressionType(ternary->trueValue);
    std::string falseType = inferExpressionType(ternary->falseValue);
    
    // Condition must be boolean or numeric
    if (!isBooleanType(condType) && !isNumericType(condType)) {
        addError(TypeChkError::ExpectedBooleanExpression,
                "Ternary operator condition must be boolean or numeric, got " + condType);
    }
    
    // True and false branches should have compatible types
    // Only check if neither is already error type
    if (trueType != "error" && falseType != "error" && !areTypesCompatible(trueType, falseType)) {
        addError(TypeChkError::ExpressionTypeMismatch,
                "Ternary operator branches have incompatible types: " + 
                trueType + " and " + falseType);
        return "error";
    }
    
    // If one branch has error, return error
    if (trueType == "error" || falseType == "error") {
        return "error";
    }
    
    return promoteTypes(trueType, falseType);
}

std::string TypeChecker::inferAssignmentType(const std::shared_ptr<AssignmentNode>& assignment) {
    if (!assignment) return "error";
    
    if (assignment->variable.empty()) {
        addError(TypeChkError::ExpressionTypeMismatch, "Assignment has no target");
        return "error";
    }
    
    // For now, assume variable type is "int" - should look up in symbol table
    std::string targetType = "int";
    std::string valueType = inferExpressionType(assignment->value);
    
    if (!areTypesCompatible(targetType, valueType)) {
        addError(TypeChkError::ExpressionTypeMismatch,
                "Cannot assign " + valueType + " to " + targetType);
        return "error";
    }
    
    return targetType;
}

std::string TypeChecker::inferExpressionType(const std::shared_ptr<ExpressionNode>& expr) {
    if (!expr) {
        addError(TypeChkError::EmptyExpression, "Empty expression encountered");
        return "error";
    }
    
    // Try to cast to different expression types
    if (auto literal = std::dynamic_pointer_cast<LiteralNode>(expr)) {
        return inferLiteralType(literal);
    }
    if (auto identifier = std::dynamic_pointer_cast<IdentifierNode>(expr)) {
        return inferIdentifierType(identifier);
    }
    if (auto binOp = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
        return inferBinaryOpType(binOp);
    }
    if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpNode>(expr)) {
        return inferUnaryOpType(unaryOp);
    }
    if (auto funcCall = std::dynamic_pointer_cast<FunctionCallNode>(expr)) {
        return inferFunctionCallType(funcCall);
    }
    if (auto ternary = std::dynamic_pointer_cast<TernaryNode>(expr)) {
        return inferTernaryType(ternary);
    }
    
    return "error";
}

// Statement type checking
void TypeChecker::checkVariableDeclaration(const std::shared_ptr<VariableDeclarationNode>& varDecl) {
    if (!varDecl) return;
    
    // If there's an initializer, check type compatibility
    if (varDecl->initializer) {
        std::string initType = inferExpressionType(varDecl->initializer);
        
        if (!areTypesCompatible(varDecl->type, initType)) {
            std::ostringstream oss;
            oss << "Variable '" << varDecl->name << "' declared as " << varDecl->type
                << " but initialized with " << initType;
            addError(TypeChkError::ErroneousVarDecl, oss.str());
        }
    }
}

void TypeChecker::checkAssignmentStatement(const std::shared_ptr<AssignmentNode>& assignment) {
    if (!assignment) return;
    inferAssignmentType(assignment); // This will add errors if types don't match
}

void TypeChecker::checkIfStatement(const std::shared_ptr<IfStatementNode>& ifStmt) {
    if (!ifStmt) return;
    
    // Check condition is boolean or numeric
    if (ifStmt->condition) {
        std::string condType = inferExpressionType(ifStmt->condition);
        if (!isBooleanType(condType) && !isNumericType(condType)) {
            addError(TypeChkError::NonBooleanCondStmt,
                    "If statement condition must be boolean or numeric, got " + condType);
        }
    }
    
    // Check then block
    if (ifStmt->thenBlock) {
        checkBlock(ifStmt->thenBlock);
    }
    
    // Check else block
    if (ifStmt->elseBlock) {
        checkBlock(ifStmt->elseBlock);
    }
}

void TypeChecker::checkWhileStatement(const std::shared_ptr<WhileStatementNode>& whileStmt) {
    if (!whileStmt) return;
    
    // Check condition
    if (whileStmt->condition) {
        std::string condType = inferExpressionType(whileStmt->condition);
        if (!isBooleanType(condType) && !isNumericType(condType)) {
            addError(TypeChkError::NonBooleanCondStmt,
                    "While statement condition must be boolean or numeric, got " + condType);
        }
    }
    
    // Check body
    loopDepth++;
    if (whileStmt->body) {
        checkBlock(whileStmt->body);
    }
    loopDepth--;
}

void TypeChecker::checkForStatement(const std::shared_ptr<ForStatementNode>& forStmt) {
    if (!forStmt) return;
    
    // Check initialization
    if (forStmt->initialization) {
        checkStatement(forStmt->initialization);
    }
    
    // Check condition
    if (forStmt->condition) {
        std::string condType = inferExpressionType(forStmt->condition);
        if (!isBooleanType(condType) && !isNumericType(condType)) {
            addError(TypeChkError::NonBooleanCondStmt,
                    "For statement condition must be boolean or numeric, got " + condType);
        }
    }
    
    // Check update
    if (forStmt->update) {
        checkStatement(forStmt->update);
    }
    
    // Check body
    loopDepth++;
    if (forStmt->body) {
        checkBlock(forStmt->body);
    }
    loopDepth--;
}

void TypeChecker::checkDoWhileStatement(const std::shared_ptr<DoWhileStatementNode>& doWhileStmt) {
    if (!doWhileStmt) return;
    
    // Check body first
    loopDepth++;
    if (doWhileStmt->body) {
        checkBlock(doWhileStmt->body);
    }
    loopDepth--;
    
    // Check condition
    if (doWhileStmt->condition) {
        std::string condType = inferExpressionType(doWhileStmt->condition);
        if (!isBooleanType(condType) && !isNumericType(condType)) {
            addError(TypeChkError::NonBooleanCondStmt,
                    "Do-while statement condition must be boolean or numeric, got " + condType);
        }
    }
}

void TypeChecker::checkReturnStatement(const std::shared_ptr<ReturnStatementNode>& returnStmt) {
    if (!returnStmt) return;
    
    hasReturnStatement = true;
    
    std::string returnType = "void";
    if (returnStmt->expression) {
        returnType = inferExpressionType(returnStmt->expression);
    }
    
    // Check if return type matches function return type
    // Don't report error if expression already has error type (avoid cascading)
    if (returnType != "error" && !areTypesCompatible(currentFunctionReturnType, returnType)) {
        std::ostringstream oss;
        oss << "Function returns " << currentFunctionReturnType 
            << " but return statement has type " << returnType;
        addError(TypeChkError::ErroneousReturnType, oss.str());
    }
}

void TypeChecker::checkBreakStatement(const std::shared_ptr<BreakStatementNode>& breakStmt) {
    if (!breakStmt) return;
    
    if (loopDepth == 0) {
        addError(TypeChkError::ErroneousBreak,
                "Break statement outside of loop");
    }
}

void TypeChecker::checkBlock(const std::shared_ptr<BlockNode>& block) {
    if (!block) return;
    
    for (const auto& stmt : block->statements) {
        if (stmt) {
            checkStatement(stmt);
        }
    }
}

void TypeChecker::checkFunctionCall(const std::shared_ptr<FunctionCallNode>& funcCall) {
    if (!funcCall) return;
    inferFunctionCallType(funcCall); // This will add errors if needed
}

void TypeChecker::checkStatement(const std::shared_ptr<StatementNode>& stmt) {
    if (!stmt) return;
    
    // Try casting to different statement types
    if (auto varDecl = std::dynamic_pointer_cast<VariableDeclarationNode>(stmt)) {
        checkVariableDeclaration(varDecl);
    }
    else if (auto assignment = std::dynamic_pointer_cast<AssignmentNode>(stmt)) {
        checkAssignmentStatement(assignment);
    }
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(stmt)) {
        checkIfStatement(ifStmt);
    }
    else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(stmt)) {
        checkWhileStatement(whileStmt);
    }
    else if (auto forStmt = std::dynamic_pointer_cast<ForStatementNode>(stmt)) {
        checkForStatement(forStmt);
    }
    else if (auto doWhileStmt = std::dynamic_pointer_cast<DoWhileStatementNode>(stmt)) {
        checkDoWhileStatement(doWhileStmt);
    }
    else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStatementNode>(stmt)) {
        checkReturnStatement(returnStmt);
    }
    else if (auto breakStmt = std::dynamic_pointer_cast<BreakStatementNode>(stmt)) {
        checkBreakStatement(breakStmt);
    }
    else if (auto block = std::dynamic_pointer_cast<BlockNode>(stmt)) {
        checkBlock(block);
    }
    else if (auto funcCall = std::dynamic_pointer_cast<FunctionCallNode>(stmt)) {
        checkFunctionCall(funcCall);
    }
}

void TypeChecker::buildFunctionSignatures(const std::shared_ptr<ProgramNode>& program) {
    if (!program) return;
    
    for (const auto& func : program->functions) {
        if (!func) continue;
        
        FunctionSignature sig;
        sig.name = func->name;
        sig.returnType = func->returnType;
        
        for (const auto& param : func->parameters) {
            if (param) {
                sig.parameters.emplace_back(param->type, param->name);
            }
        }
        
        functionSignatures[sig.name] = sig;
    }
}

void TypeChecker::checkFunction(const std::shared_ptr<FunctionNode>& func) {
    if (!func) return;
    
    currentFunctionReturnType = func->returnType;
    currentFunctionName = func->name; // Set current function for scoped lookups
    hasReturnStatement = false;
    loopDepth = 0;
    
    // Check function body
    if (func->body) {
        checkBlock(func->body);
    }
    
    // Check if non-void function has return statement
    if (func->returnType != "void" && !hasReturnStatement) {
        std::ostringstream oss;
        oss << "Function '" << func->name << "' with return type '" 
            << func->returnType << "' must have a return statement";
        addError(TypeChkError::ReturnStmtNotFound, oss.str());
    }
    
    currentFunctionName = ""; // Reset after function
}

bool TypeChecker::check(const std::shared_ptr<ProgramNode>& program) {
    if (!program) {
        addError(TypeChkError::EmptyExpression, "Program is null");
        return false;
    }
    
    errors.clear();
    
    // Build function signatures first
    buildFunctionSignatures(program);
    
    // Check global variable declarations
    for (const auto& varDecl : program->globalVariables) {
        if (varDecl) {
            checkVariableDeclaration(varDecl);
        }
    }
    
    // Check each function
    for (const auto& func : program->functions) {
        if (func) {
            checkFunction(func);
        }
    }
    
    return errors.empty();
}

void TypeChecker::printErrors() const {
    if (errors.empty()) {
        std::cout << "[OK] Type checking passed: No type errors detected." << std::endl;
        return;
    }
    
    std::cout << "[X] Type checking failed with " << errors.size() << " error(s):\n" << std::endl;
    
    for (size_t i = 0; i < errors.size(); i++) {
        const auto& error = errors[i];
        std::cout << "     [" << (i + 1) << "] " << typeErrorToString(error.errorType) << ": "
                  << error.message;
        
        if (error.line > 0) {
            std::cout << " (Line " << error.line;
            if (error.column > 0) {
                std::cout << ", Col " << error.column;
            }
            std::cout << ")";
        }
        std::cout << std::endl;
    }
}
