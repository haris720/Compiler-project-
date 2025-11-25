#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "parser.h"
#include "scope_analyzer.h"
#include "type_checker.h"

// TAC Instruction Types
enum class TACOpcode {
    // Arithmetic
    ADD,           // t1 = t2 + t3
    SUB,           // t1 = t2 - t3
    MUL,           // t1 = t2 * t3
    DIV,           // t1 = t2 / t3
    MOD,           // t1 = t2 % t3
    NEG,           // t1 = -t2
    
    // Logical
    AND,           // t1 = t2 && t3
    OR,            // t1 = t2 || t3
    NOT,           // t1 = !t2
    
    // Bitwise
    BIT_AND,       // t1 = t2 & t3
    BIT_OR,        // t1 = t2 | t3
    BIT_XOR,       // t1 = t2 ^ t3
    BIT_NOT,       // t1 = ~t2
    LEFT_SHIFT,    // t1 = t2 << t3
    RIGHT_SHIFT,   // t1 = t2 >> t3
    
    // Comparison
    EQ,            // t1 = t2 == t3
    NEQ,           // t1 = t2 != t3
    LT,            // t1 = t2 < t3
    LE,            // t1 = t2 <= t3
    GT,            // t1 = t2 > t3
    GE,            // t1 = t2 >= t3
    
    // Assignment & Copy
    ASSIGN,        // t1 = t2
    COPY,          // t1 = t2
    
    // Control Flow
    LABEL,         // L1:
    GOTO,          // goto L1
    IF_FALSE,      // if !t1 goto L1
    IF_TRUE,       // if t1 goto L1
    
    // Function Operations
    PARAM,         // param t1
    CALL,          // t1 = call func, n
    RETURN,        // return t1
    RETURN_VOID,   // return
    
    // Array Operations
    ARRAY_READ,    // t1 = arr[t2]
    ARRAY_WRITE,   // arr[t1] = t2
    
    // Special
    PRINT,         // print t1
    NOP            // no operation
};

// Convert opcode to string
inline std::string opcodeToString(TACOpcode op) {
    switch (op) {
        case TACOpcode::ADD: return "ADD";
        case TACOpcode::SUB: return "SUB";
        case TACOpcode::MUL: return "MUL";
        case TACOpcode::DIV: return "DIV";
        case TACOpcode::MOD: return "MOD";
        case TACOpcode::NEG: return "NEG";
        case TACOpcode::AND: return "AND";
        case TACOpcode::OR: return "OR";
        case TACOpcode::NOT: return "NOT";
        case TACOpcode::BIT_AND: return "BIT_AND";
        case TACOpcode::BIT_OR: return "BIT_OR";
        case TACOpcode::BIT_XOR: return "BIT_XOR";
        case TACOpcode::BIT_NOT: return "BIT_NOT";
        case TACOpcode::LEFT_SHIFT: return "LEFT_SHIFT";
        case TACOpcode::RIGHT_SHIFT: return "RIGHT_SHIFT";
        case TACOpcode::EQ: return "EQ";
        case TACOpcode::NEQ: return "NEQ";
        case TACOpcode::LT: return "LT";
        case TACOpcode::LE: return "LE";
        case TACOpcode::GT: return "GT";
        case TACOpcode::GE: return "GE";
        case TACOpcode::ASSIGN: return "ASSIGN";
        case TACOpcode::COPY: return "COPY";
        case TACOpcode::LABEL: return "LABEL";
        case TACOpcode::GOTO: return "GOTO";
        case TACOpcode::IF_FALSE: return "IF_FALSE";
        case TACOpcode::IF_TRUE: return "IF_TRUE";
        case TACOpcode::PARAM: return "PARAM";
        case TACOpcode::CALL: return "CALL";
        case TACOpcode::RETURN: return "RETURN";
        case TACOpcode::RETURN_VOID: return "RETURN_VOID";
        case TACOpcode::ARRAY_READ: return "ARRAY_READ";
        case TACOpcode::ARRAY_WRITE: return "ARRAY_WRITE";
        case TACOpcode::PRINT: return "PRINT";
        case TACOpcode::NOP: return "NOP";
        default: return "UNKNOWN";
    }
}

// TAC Instruction
struct TACInstruction {
    TACOpcode opcode;
    std::string result;      // Result operand (t1, x, etc.)
    std::string arg1;        // First argument
    std::string arg2;        // Second argument
    
    TACInstruction(TACOpcode op, const std::string& res = "", 
                   const std::string& a1 = "", const std::string& a2 = "")
        : opcode(op), result(res), arg1(a1), arg2(a2) {}
    
    std::string toString() const {
        std::string str;
        
        switch (opcode) {
            case TACOpcode::LABEL:
                return result + ":";
            
            case TACOpcode::GOTO:
                return "    goto " + result;
            
            case TACOpcode::IF_FALSE:
                return "    if !" + result + " goto " + arg1;
            
            case TACOpcode::IF_TRUE:
                return "    if " + result + " goto " + arg1;
            
            case TACOpcode::PARAM:
                return "    param " + result;
            
            case TACOpcode::CALL:
                if (result.empty())
                    return "    call " + arg1 + ", " + arg2;
                return "    " + result + " = call " + arg1 + ", " + arg2;
            
            case TACOpcode::RETURN:
                return "    return " + result;
            
            case TACOpcode::RETURN_VOID:
                return "    return";
            
            case TACOpcode::PRINT:
                return "    print " + result;
            
            case TACOpcode::NEG:
            case TACOpcode::NOT:
            case TACOpcode::BIT_NOT:
                return "    " + result + " = " + opcodeToString(opcode) + " " + arg1;
            
            case TACOpcode::ASSIGN:
            case TACOpcode::COPY:
                return "    " + result + " = " + arg1;
            
            default:
                // Binary operations
                return "    " + result + " = " + arg1 + " " + opcodeToString(opcode) + " " + arg2;
        }
    }
};

// IR Generator Class
class IRGenerator {
private:
    std::vector<TACInstruction> instructions;
    int tempCounter;
    int labelCounter;
    std::string currentFunction;
    
    // Helper methods
    std::string newTemp();
    std::string newLabel();
    
    // Generate IR for different node types
    std::string generateExpression(const std::shared_ptr<ExpressionNode>& expr);
    std::string generateBinaryOp(const std::shared_ptr<BinaryOpNode>& binOp);
    std::string generateUnaryOp(const std::shared_ptr<UnaryOpNode>& unaryOp);
    std::string generateTernary(const std::shared_ptr<TernaryNode>& ternary);
    std::string generateFunctionCall(const std::shared_ptr<FunctionCallNode>& funcCall);
    std::string generateIdentifier(const std::shared_ptr<IdentifierNode>& ident);
    std::string generateLiteral(const std::shared_ptr<LiteralNode>& literal);
    
    void generateStatement(const std::shared_ptr<StatementNode>& stmt);
    void generateVariableDeclaration(const std::shared_ptr<VariableDeclarationNode>& varDecl);
    void generateAssignment(const std::shared_ptr<AssignmentNode>& assignment);
    void generateIfStatement(const std::shared_ptr<IfStatementNode>& ifStmt);
    void generateWhileStatement(const std::shared_ptr<WhileStatementNode>& whileStmt);
    void generateForStatement(const std::shared_ptr<ForStatementNode>& forStmt);
    void generateSwitchStatement(const std::shared_ptr<SwitchStatementNode>& switchStmt);
    void generateReturnStatement(const std::shared_ptr<ReturnStatementNode>& returnStmt);
    void generatePrintStatement(const std::shared_ptr<PrintStatementNode>& printStmt);
    void generateBreakStatement(const std::shared_ptr<BreakStatementNode>& breakStmt);
    void generateContinueStatement(const std::shared_ptr<ContinueStatementNode>& continueStmt);
    
    void generateFunction(const std::shared_ptr<FunctionNode>& function);
    
    // Break/Continue label stack
    std::vector<std::string> breakLabels;
    std::vector<std::string> continueLabels;
    
    void pushLoopLabels(const std::string& breakLabel, const std::string& continueLabel) {
        breakLabels.push_back(breakLabel);
        continueLabels.push_back(continueLabel);
    }
    
    void popLoopLabels() {
        if (!breakLabels.empty()) breakLabels.pop_back();
        if (!continueLabels.empty()) continueLabels.pop_back();
    }
    
    std::string getCurrentBreakLabel() {
        return breakLabels.empty() ? "" : breakLabels.back();
    }
    
    std::string getCurrentContinueLabel() {
        return continueLabels.empty() ? "" : continueLabels.back();
    }
    
public:
    IRGenerator() : tempCounter(0), labelCounter(0) {}
    
    // Main generation entry point
    void generate(const std::shared_ptr<ProgramNode>& program);
    
    // Get generated instructions
    const std::vector<TACInstruction>& getInstructions() const {
        return instructions;
    }
    
    // Print TAC
    void printTAC() const;
    
    // Add instruction
    void emit(const TACInstruction& instr) {
        instructions.push_back(instr);
    }
};

#endif // IR_GENERATOR_H
