#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "scope_analyzer.h"
#include "type_checker.h"
#include "ir_generator.h"
#include "ast_printer_detailed.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }

    std::cout << "========================================================" << std::endl;
    std::cout << "           COMPILER PIPELINE                            " << std::endl;
    std::cout << "========================================================\n" << std::endl;

    // ===== PHASE 1: LEXICAL ANALYSIS =====
    std::cout << "=== PHASE 1: LEXICAL ANALYSIS ===" << std::endl;
    Lexer lexer(argv[1], true);  // true = read from file
    std::vector<Token> tokens = lexer.tokenize();
    
    if (tokens.empty() || tokens.back().getType() != TokenType::T_EOF) {
        std::cerr << "[X] Lexical analysis failed!" << std::endl;
        return 1;
    }
    
    std::cout << "[OK] Lexical analysis completed successfully" << std::endl;
    std::cout << "     Total tokens generated: " << tokens.size() << std::endl;
    
    // Show ALL tokens
    std::cout << "\n     Complete Token Stream:" << std::endl;
    std::cout << "     --------------------------------------------------------" << std::endl;
    for (size_t i = 0; i < tokens.size(); i++) {
        std::cout << "     [" << (i + 1) << "] " << tokens[i].toString() << std::endl;
    }
    std::cout << "     --------------------------------------------------------" << std::endl;

    // ===== PHASE 2: PARSING =====
    std::cout << "\n=== PHASE 2: PARSING ===" << std::endl;
    Parser parser(tokens);
    std::shared_ptr<ProgramNode> program_root = parser.parse();
    
    if (!program_root || parser.hasErrors()) {
        std::cerr << "[X] Parsing failed!" << std::endl;
        return 1;
    }
    
    std::cout << "[OK] Parsing completed successfully" << std::endl;
    std::cout << "     Functions: " << program_root->functions.size() << std::endl;
    std::cout << "     Global Variables: " << program_root->globalVariables.size() << std::endl;
    
    // Print Detailed Parse Tree
    std::cout << "\n     Abstract Syntax Tree (Parse Tree):" << std::endl;
    std::cout << "     --------------------------------------------------------" << std::endl;
    DetailedASTPrinter astPrinter;
    astPrinter.printProgram(program_root);
    std::cout << "     --------------------------------------------------------" << std::endl;

    // ===== PHASE 3: SCOPE ANALYSIS =====
    std::cout << "\n=== PHASE 3: SCOPE ANALYSIS ===" << std::endl;
    ScopeAnalyzer scopeAnalyzer;
    bool scopeSuccess = scopeAnalyzer.analyze(program_root);
    
    if (!scopeSuccess) {
        std::cout << "[X] Scope analysis failed!" << std::endl;
        scopeAnalyzer.printErrors();
        return 1;
    }
    
    std::cout << "[OK] Scope analysis passed successfully" << std::endl;
    
    // Print symbol tables
    std::cout << "\n     Symbol Tables:" << std::endl;
    std::cout << "     --------------------------------------------------------" << std::endl;
    std::cout << "     Global Scope:" << std::endl;
    
    // Count global variables and functions
    int globalVarCount = program_root->globalVariables.size();
    int functionCount = program_root->functions.size();
    
    std::cout << "       |-- Variables: " << globalVarCount << std::endl;
    for (const auto& var : program_root->globalVariables) {
        std::cout << "       |   \\-- " << var->name << ": " << var->type << std::endl;
    }
    
    std::cout << "       \\-- Functions: " << functionCount << std::endl;
    for (size_t i = 0; i < program_root->functions.size(); i++) {
        const auto& func = program_root->functions[i];
        bool isLast = (i == program_root->functions.size() - 1);
        std::cout << "           " << (isLast ? "\\--" : "|--") << " " << func->name 
                 << ": " << func->returnType << "(";
        for (size_t j = 0; j < func->parameters.size(); j++) {
            if (j > 0) std::cout << ", ";
            std::cout << func->parameters[j]->type;
        }
        std::cout << ")" << std::endl;
    }
    std::cout << "     --------------------------------------------------------" << std::endl;

    // ===== PHASE 4: TYPE CHECKING =====
    std::cout << "\n=== PHASE 4: TYPE CHECKING ===" << std::endl;
    TypeChecker typeChecker(&scopeAnalyzer);
    bool typeSuccess = typeChecker.check(program_root);
    
    if (!typeSuccess) {
        std::cout << "[X] Type checking failed!" << std::endl;
        typeChecker.printErrors();
        return 1;
    }
    
    std::cout << "[OK] Type checking passed successfully" << std::endl;
    std::cout << "     All type constraints satisfied" << std::endl;
    std::cout << "     No type errors detected" << std::endl;

    // ===== PHASE 5: IR GENERATION =====
    std::cout << "\n=== PHASE 5: IR GENERATION ===" << std::endl;
    IRGenerator irGenerator;
    irGenerator.generate(program_root);
    
    std::cout << "[OK] IR generation completed successfully" << std::endl;
    
    // Print TAC
    irGenerator.printTAC();

    // ===== SUCCESS =====
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  [OK] ALL COMPILATION PHASES PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "========================================================" << std::endl;

    return 0;
}


