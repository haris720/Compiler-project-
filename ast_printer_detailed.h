#ifndef AST_PRINTER_DETAILED_H
#define AST_PRINTER_DETAILED_H

#include <iostream>
#include <memory>
#include <string>
#include "parser.h"

class DetailedASTPrinter {
private:
    int indentLevel = 0;
    const int INDENT_SIZE = 4;

    void printIndent() {
        for (int i = 0; i < indentLevel * INDENT_SIZE; i++) {
            std::cout << " ";
        }
    }

    void printExpression(const std::shared_ptr<ExpressionNode>& expr) {
        if (!expr) {
            std::cout << "None";
            return;
        }

        if (auto literal = std::dynamic_pointer_cast<LiteralNode>(expr)) {
            std::cout << literal->value;
        }
        else if (auto var = std::dynamic_pointer_cast<IdentifierNode>(expr)) {
            std::cout << "\"" << var->name << "\"";
        }
        else if (auto binOp = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
            std::cout << std::endl;
            printIndent();
            // Map operators to their proper names
            std::string opName = binOp->op;
            std::string opType = "BinaryOp";
            
            if (binOp->op == "+") {
                opType = "Add";
                opName = "AddOp";
            } else if (binOp->op == "-") {
                opType = "Add";
                opName = "SubOp";
            } else if (binOp->op == "*") {
                opType = "Mul";
                opName = "MulOp";
            } else if (binOp->op == "/") {
                opType = "Mul";
                opName = "DivOp";
            } else if (binOp->op == "%") {
                opType = "Mul";
                opName = "ModOp";
            } else if (binOp->op == "==") {
                opType = "Comp";
                opName = "EqualsOp";
            } else if (binOp->op == "!=") {
                opType = "Comp";
                opName = "NotEqualsOp";
            } else if (binOp->op == "<") {
                opType = "Comp";
                opName = "LessThan";
            } else if (binOp->op == ">") {
                opType = "Comp";
                opName = "GreaterThan";
            } else if (binOp->op == "<=") {
                opType = "Comp";
                opName = "LessThanOrEqual";
            } else if (binOp->op == ">=") {
                opType = "Comp";
                opName = "GreaterThanOrEqual";
            }
            
            std::cout << "::" << opType << "(" << opName << ")" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(binOp->left);
            std::cout << std::endl;
            printIndent();
            printExpression(binOp->right);
            indentLevel--;
        }
        else if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(expr)) {
            std::cout << std::endl;
            printIndent();
            // Map unary operators
            std::string opName = unary->op;
            if (unary->op == "-") {
                opName = "SubOp";
            } else if (unary->op == "!") {
                opName = "NotOp";
            }
            std::cout << "Unary(" << opName << ")" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(unary->operand);
            indentLevel--;
        }
        else if (auto call = std::dynamic_pointer_cast<FunctionCallNode>(expr)) {
            std::cout << std::endl;
            printIndent();
            std::cout << "Call(FnCall {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "ident: \"" << call->functionName << "\"," << std::endl;
            printIndent();
            std::cout << "args: [" << std::endl;
            indentLevel++;
            for (const auto& arg : call->arguments) {
                printIndent();
                std::cout << "Some(" << std::endl;
                indentLevel++;
                printIndent();
                printExpression(arg);
                std::cout << std::endl;
                indentLevel--;
                printIndent();
                std::cout << ")," << std::endl;
            }
            indentLevel--;
            printIndent();
            std::cout << "]," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "})";
        }
        else {
            std::cout << "<complex_expression>";
        }
    }

    void printStatement(const std::shared_ptr<StatementNode>& stmt, const std::string& prefix = "") {
        printIndent();
        
        if (auto varDecl = std::dynamic_pointer_cast<VariableDeclarationNode>(stmt)) {
            std::cout << prefix << "Var(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "VarDecl {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "type_tok: " << varDecl->type << "," << std::endl;
            printIndent();
            std::cout << "ident: \"" << varDecl->name << "\"," << std::endl;
            printIndent();
            std::cout << "expr: Some(" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(varDecl->initializer);
            std::cout << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }
        else if (auto retStmt = std::dynamic_pointer_cast<ReturnStatementNode>(stmt)) {
            std::cout << prefix << "Ret(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "ExprStmt {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "expr: Some(" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(retStmt->expression);
            std::cout << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }
        else if (auto assignStmt = std::dynamic_pointer_cast<AssignmentNode>(stmt)) {
            std::cout << prefix << "Assign(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "AssignStmt {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "ident: \"" << assignStmt->variable << "\"," << std::endl;
            printIndent();
            std::cout << "expr: Some(" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(assignStmt->value);
            std::cout << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }
        else if (auto ifStmt = std::dynamic_pointer_cast<IfStatementNode>(stmt)) {
            std::cout << prefix << "If(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "IfStmt {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "cond: Some(" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(ifStmt->condition);
            std::cout << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
            printIndent();
            std::cout << "if_block: [" << std::endl;
            indentLevel++;
            if (ifStmt->thenBlock) {
                for (const auto& s : ifStmt->thenBlock->statements) {
                    printStatement(s);
                }
            }
            indentLevel--;
            printIndent();
            std::cout << "]," << std::endl;
            printIndent();
            std::cout << "else_block: [" << std::endl;
            indentLevel++;
            if (ifStmt->elseBlock) {
                for (const auto& s : ifStmt->elseBlock->statements) {
                    printStatement(s);
                }
            }
            indentLevel--;
            printIndent();
            std::cout << "]," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }
        else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatementNode>(stmt)) {
            std::cout << prefix << "While(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "WhileStmt {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "cond: Some(" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(whileStmt->condition);
            std::cout << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
            printIndent();
            std::cout << "block: [" << std::endl;
            indentLevel++;
            if (whileStmt->body) {
                for (const auto& s : whileStmt->body->statements) {
                    printStatement(s);
                }
            }
            indentLevel--;
            printIndent();
            std::cout << "]," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }
        else if (auto forStmt = std::dynamic_pointer_cast<ForStatementNode>(stmt)) {
            std::cout << prefix << "For(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "ForStmt {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "init: ";
            if (forStmt->initialization) {
                // Check if it's a variable declaration
                if (auto varDecl = std::dynamic_pointer_cast<VariableDeclarationNode>(forStmt->initialization)) {
                    std::cout << "Some(" << std::endl;
                    indentLevel++;
                    printIndent();
                    std::cout << "VarDecl {" << std::endl;
                    indentLevel++;
                    printIndent();
                    std::cout << "type_tok: " << varDecl->type << "," << std::endl;
                    printIndent();
                    std::cout << "ident: \"" << varDecl->name << "\"," << std::endl;
                    printIndent();
                    std::cout << "expr: Some(" << std::endl;
                    indentLevel++;
                    printIndent();
                    printExpression(varDecl->initializer);
                    std::cout << "," << std::endl;
                    indentLevel--;
                    printIndent();
                    std::cout << ")," << std::endl;
                    indentLevel--;
                    printIndent();
                    std::cout << "}," << std::endl;
                    indentLevel--;
                    printIndent();
                    std::cout << ")," << std::endl;
                } else {
                    std::cout << "Some(" << std::endl;
                    indentLevel++;
                    printStatement(forStmt->initialization, "");
                    indentLevel--;
                    printIndent();
                    std::cout << ")," << std::endl;
                }
            } else {
                std::cout << "None," << std::endl;
            }
            printIndent();
            std::cout << "cond: ExprStmt {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "expr: Some(" << std::endl;
            indentLevel++;
            printIndent();
            printExpression(forStmt->condition);
            std::cout << "," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            printIndent();
            std::cout << "updt: ";
            if (forStmt->update) {
                if (auto assignStmt = std::dynamic_pointer_cast<AssignmentNode>(forStmt->update)) {
                    std::cout << "Some(" << std::endl;
                    indentLevel++;
                    printIndent();
                    std::cout << "Assign(AssignOp)" << std::endl;
                    indentLevel++;
                    printIndent();
                    std::cout << "\"" << assignStmt->variable << "\"" << std::endl;
                    printIndent();
                    printExpression(assignStmt->value);
                    std::cout << std::endl;
                    indentLevel--;
                    indentLevel--;
                    printIndent();
                    std::cout << ")," << std::endl;
                } else {
                    std::cout << "Some(" << std::endl;
                    indentLevel++;
                    printStatement(forStmt->update, "");
                    indentLevel--;
                    printIndent();
                    std::cout << ")," << std::endl;
                }
            } else {
                std::cout << "None," << std::endl;
            }
            printIndent();
            std::cout << "block: [" << std::endl;
            indentLevel++;
            if (forStmt->body) {
                for (const auto& s : forStmt->body->statements) {
                    printStatement(s);
                }
            }
            indentLevel--;
            printIndent();
            std::cout << "]," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }
        else if (auto breakStmt = std::dynamic_pointer_cast<BreakStatementNode>(stmt)) {
            std::cout << prefix;
            printIndent();
            std::cout << "Break," << std::endl;
        }
        else if (auto continueStmt = std::dynamic_pointer_cast<ContinueStatementNode>(stmt)) {
            std::cout << prefix;
            printIndent();
            std::cout << "Continue," << std::endl;
        }
        else {
            std::cout << prefix << "<other_statement>," << std::endl;
        }
    }

public:
    void printProgram(const std::shared_ptr<ProgramNode>& program) {
        std::cout << "[" << std::endl;
        indentLevel++;

        // Print functions FIRST
        for (const auto& func : program->functions) {
            printIndent();
            std::cout << "Fn(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "FnDecl {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "type_tok: " << func->returnType << "," << std::endl;
            printIndent();
            std::cout << "ident: \"" << func->name << "\"," << std::endl;
            printIndent();
            std::cout << "params: [" << std::endl;
            indentLevel++;
            for (const auto& param : func->parameters) {
                printIndent();
                std::cout << "Param {" << std::endl;
                indentLevel++;
                printIndent();
                std::cout << "type_tok: " << param->type << "," << std::endl;
                printIndent();
                std::cout << "ident: \"" << param->name << "\"," << std::endl;
                indentLevel--;
                printIndent();
                std::cout << "}," << std::endl;
            }
            indentLevel--;
            printIndent();
            std::cout << "]," << std::endl;
            printIndent();
            std::cout << "block: [" << std::endl;
            indentLevel++;
            if (func->body) {
                for (const auto& stmt : func->body->statements) {
                    printStatement(stmt);
                }
            }
            indentLevel--;
            printIndent();
            std::cout << "]," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }

        // Print global variables AFTER functions
        for (const auto& var : program->globalVariables) {
            printIndent();
            std::cout << "Var(" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "VarDecl {" << std::endl;
            indentLevel++;
            printIndent();
            std::cout << "type_tok: " << var->type << "," << std::endl;
            printIndent();
            std::cout << "ident: \"" << var->name << "\"," << std::endl;
            printIndent();
            std::cout << "expr: ";
            if (var->initializer) {
                std::cout << "Some(" << std::endl;
                indentLevel++;
                printIndent();
                printExpression(var->initializer);
                std::cout << std::endl;
                indentLevel--;
                printIndent();
                std::cout << ")," << std::endl;
            } else {
                std::cout << "None," << std::endl;
            }
            indentLevel--;
            printIndent();
            std::cout << "}," << std::endl;
            indentLevel--;
            printIndent();
            std::cout << ")," << std::endl;
        }

        indentLevel--;
        std::cout << "]" << std::endl;
    }
};

#endif // AST_PRINTER_DETAILED_H
