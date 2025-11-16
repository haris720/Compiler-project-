#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "parser.h"
#include "scope_analyzer.h"

// Type checking error enumeration
enum class TypeChkError {
    ErroneousVarDecl,
    FnCallParamCount,
    FnCallParamType,
    ErroneousReturnType,
    ExpressionTypeMismatch,
    ExpectedBooleanExpression,
    ErroneousBreak,
    NonBooleanCondStmt,
    EmptyExpression,
    AttemptedBoolOpOnNonBools,
    AttemptedBitOpOnNonNumeric,
    AttemptedShiftOnNonInt,
    AttemptedAddOpOnNonNumeric,
    AttemptedExponentiationOfNonNumeric,
    ReturnStmtNotFound
};

// Type error information
struct TypeErrorInfo {
    TypeChkError errorType;
    std::string message;
    int line;
    int column;
    
    TypeErrorInfo(TypeChkError type, const std::string& msg, int ln = 0, int col = 0)
        : errorType(type), message(msg), line(ln), column(col) {}
};

// Function signature information
struct FunctionSignature {
    std::string name;
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> parameters; // (type, name) pairs
};

// Type Checker class
class TypeChecker {
private:
    std::vector<TypeErrorInfo> errors;
    ScopeAnalyzer* scopeAnalyzer;
    std::unordered_map<std::string, FunctionSignature> functionSignatures;
    std::string currentFunctionReturnType;
    std::string currentFunctionName; // Track current function for scoped variable lookup
    bool hasReturnStatement;
    int loopDepth; // Track if we're inside a loop (for break/continue)
    
    // Helper methods for type checking
    std::string inferExpressionType(const std::shared_ptr<ExpressionNode>& expr);
    std::string inferLiteralType(const std::shared_ptr<LiteralNode>& literal);
    std::string inferBinaryOpType(const std::shared_ptr<BinaryOpNode>& binOp);
    std::string inferUnaryOpType(const std::shared_ptr<UnaryOpNode>& unaryOp);
    std::string inferIdentifierType(const std::shared_ptr<IdentifierNode>& identifier);
    std::string inferFunctionCallType(const std::shared_ptr<FunctionCallNode>& funcCall);
    std::string inferTernaryType(const std::shared_ptr<TernaryNode>& ternary);
    std::string inferAssignmentType(const std::shared_ptr<AssignmentNode>& assignment);
    
    bool isNumericType(const std::string& type);
    bool isIntegerType(const std::string& type);
    bool isBooleanType(const std::string& type);
    bool areTypesCompatible(const std::string& type1, const std::string& type2);
    std::string promoteTypes(const std::string& type1, const std::string& type2);
    
    // Statement type checking
    void checkStatement(const std::shared_ptr<StatementNode>& stmt);
    void checkVariableDeclaration(const std::shared_ptr<VariableDeclarationNode>& varDecl);
    void checkAssignmentStatement(const std::shared_ptr<AssignmentNode>& assignment);
    void checkIfStatement(const std::shared_ptr<IfStatementNode>& ifStmt);
    void checkWhileStatement(const std::shared_ptr<WhileStatementNode>& whileStmt);
    void checkForStatement(const std::shared_ptr<ForStatementNode>& forStmt);
    void checkDoWhileStatement(const std::shared_ptr<DoWhileStatementNode>& doWhileStmt);
    void checkReturnStatement(const std::shared_ptr<ReturnStatementNode>& returnStmt);
    void checkBreakStatement(const std::shared_ptr<BreakStatementNode>& breakStmt);
    void checkBlock(const std::shared_ptr<BlockNode>& block);
    void checkFunctionCall(const std::shared_ptr<FunctionCallNode>& funcCall);
    
    // Function type checking
    void checkFunction(const std::shared_ptr<FunctionNode>& func);
    void buildFunctionSignatures(const std::shared_ptr<ProgramNode>& program);
    
    void addError(TypeChkError errorType, const std::string& message);
    void addError(TypeChkError errorType, const std::string& message, int line, int col);

public:
    TypeChecker(ScopeAnalyzer* analyzer);
    
    bool check(const std::shared_ptr<ProgramNode>& program);
    void printErrors() const;
    bool hasErrors() const { return !errors.empty(); }
    const std::vector<TypeErrorInfo>& getErrors() const { return errors; }
};

// Helper function to convert TypeChkError to string
std::string typeErrorToString(TypeChkError error);

#endif // TYPE_CHECKER_H
