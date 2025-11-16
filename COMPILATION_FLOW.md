# Complete Compilation Flow: Lexer → Parser → Scope Analyzer

## Overview

This document demonstrates the **three-phase compilation process** used by the compiler:

1. **Lexical Analysis (Tokenization)**
2. **Syntax Analysis (Parsing)** 
3. **Semantic Analysis (Scope Checking)**

---

## Example: test_undeclared_var.txt

### Source Code
```cpp
// Test 1: UndeclaredVariableAccessed

function int test1() {
    int a = 5;
    int b = undeclaredVar + 10;  // ERROR: undeclaredVar not declared
    return b;
}
```

---

## Phase 1: Lexical Analysis (TOKENIZATION) ✅

### Purpose
Convert source code text into a stream of typed tokens.

### Input
Raw source code characters

### Output
Token stream (23 tokens)

### Token Stream
```
[T_FUNCTION("function"), Line: 4, Col: 1]
[T_INT("int"), Line: 4, Col: 10]
[T_IDENTIFIER("test1"), Line: 4, Col: 14]
[T_LPAREN("("), Line: 4, Col: 19]
[T_RPAREN(")"), Line: 4, Col: 20]
[T_LBRACE("{"), Line: 4, Col: 22]
[T_INT("int"), Line: 5, Col: 5]
[T_IDENTIFIER("a"), Line: 5, Col: 9]
[T_ASSIGN("="), Line: 5, Col: 11]
[T_INT_LITERAL("5"), Line: 5, Col: 13]
[T_SEMICOLON(";"), Line: 5, Col: 14]
[T_INT("int"), Line: 6, Col: 5]
[T_IDENTIFIER("b"), Line: 6, Col: 9]
[T_ASSIGN("="), Line: 6, Col: 11]
[T_IDENTIFIER("undeclaredVar"), Line: 6, Col: 13]  ← Key token!
[T_PLUS("+"), Line: 6, Col: 27]
[T_INT_LITERAL("10"), Line: 6, Col: 29]
[T_SEMICOLON(";"), Line: 6, Col: 31]
[T_RETURN("return"), Line: 7, Col: 5]
[T_IDENTIFIER("b"), Line: 7, Col: 12]
[T_SEMICOLON(";"), Line: 7, Col: 13]
[T_RBRACE("}"), Line: 8, Col: 1]
[T_EOF(""), Line: 9, Col: 2]
```

### What Happened
- Each word, symbol, and number recognized
- Keywords identified (function, int, return)
- Identifiers extracted (test1, a, b, **undeclaredVar**)
- Operators and punctuation categorized
- Line and column information recorded

### Status: ✅ SUCCESS
The lexer completed without errors.

---

## Phase 2: Syntax Analysis (PARSING) ✅

### Purpose
Build an Abstract Syntax Tree (AST) from the token stream.

### Input
Token stream from Phase 1

### Output
Abstract Syntax Tree

### AST Structure
```
Program
└─ Functions: [1]
    └─ Function: int test1()
        ├─ Return Type: int
        ├─ Parameters: []
        └─ Body: Block
            ├─ Statement 1: VarDecl
            │   ├─ Type: int
            │   ├─ Name: a
            │   └─ Initializer: Literal(5)
            │
            ├─ Statement 2: VarDecl
            │   ├─ Type: int
            │   ├─ Name: b
            │   └─ Initializer: BinaryOp(+)
            │       ├─ Left: Identifier(undeclaredVar)  ← Referenced here!
            │       └─ Right: Literal(10)
            │
            └─ Statement 3: Return
                └─ Expression: Identifier(b)
```

### What Happened
- Tokens organized into hierarchical structure
- Function declaration parsed correctly
- Variable declarations identified
- Expression tree built for `undeclaredVar + 10`
- Return statement linked to identifier `b`

### Status: ✅ SUCCESS
Parser built complete AST.

---

## Phase 3: Semantic Analysis (SCOPE CHECKING) ❌

### Purpose
Validate variable and function references using scope rules.

### Input
Abstract Syntax Tree from Phase 2

### Scope Analysis Process

#### Step 1: Initialize Global Scope
```
Scope Level 0 (Global)
├─ Variables: (none)
└─ Functions: [test1]
```

#### Step 2: Enter Function Scope
```
Enter function: test1()
Create new scope (Level 1)
Parent: Global Scope (Level 0)
```

#### Step 3: Process Function Body

**Statement 1: `int a = 5;`**
```
Action: Declare variable 'a'
Scope: Level 1 (function test1)
Type: int
✅ Variable 'a' added to current scope
```

Current Symbol Table:
```
Scope Level 1 (test1)
└─ Variables: {a: int}
```

**Statement 2: `int b = undeclaredVar + 10;`**
```
Action: Declare variable 'b'
First, evaluate initializer: undeclaredVar + 10

Check identifier: 'undeclaredVar'
  1. Look in current scope (Level 1): NOT FOUND
  2. Look in parent scope (Level 0): NOT FOUND
  3. No more parent scopes
  
❌ ERROR: Variable 'undeclaredVar' not declared!
```

Current Symbol Table:
```
Scope Level 1 (test1)
├─ Variables: {a: int, b: int}
└─ Errors: [UndeclaredVariableAccessed]
```

**Statement 3: `return b;`**
```
Action: Return statement
Check identifier: 'b'
  1. Look in current scope (Level 1): FOUND ✅
  
✅ Variable 'b' is declared
```

#### Step 4: Exit Function Scope
```
Exit function: test1()
Pop scope (Level 1)
Return to: Global Scope (Level 0)
```

### Final Symbol Tables

**Global Scope:**
```
Functions:
  - int test1()
    
Global Variables: (none)
```

**Function Scope (test1):**
```
Local Variables:
  - int a (declared line 5)
  - int b (declared line 6)

Parameters: (none)
```

### Scope Analysis Result

**Status:** ❌ **FAILED**

**Errors Detected:** 1

```
[UndeclaredVariableAccessed]
  Variable: 'undeclaredVar'
  Location: Line 6
  Context: Used in expression but never declared
  Scope Chain Searched: Level 1 → Level 0 → (none found)
```

---

## Spaghetti Stack Visualization

The scope analyzer uses a **spaghetti stack** where each scope points to its parent:

```
┌─────────────────────────────┐
│  Global Scope (Level 0)     │
│  ───────────────────────     │
│  Functions:                  │
│    - test1: int()            │
│  Variables: (none)           │
└───────────────┬─────────────┘
                │ parent
                │
┌───────────────▼─────────────┐
│  Function Scope (Level 1)   │
│  test1                       │
│  ───────────────────────     │
│  Variables:                  │
│    - a: int                  │
│    - b: int                  │
│  Parameters: (none)          │
│                              │
│  Lookup "undeclaredVar":     │
│    1. Check Level 1 ❌       │
│    2. Check Level 0 ❌       │
│    3. Not found → ERROR!     │
└──────────────────────────────┘
```

---

## Complete Compilation Flow Summary

| Phase | Input | Output | Status | Time |
|-------|-------|--------|--------|------|
| **1. Lexer** | Source code | 23 tokens | ✅ Success | ~1ms |
| **2. Parser** | 23 tokens | AST (3 statements) | ✅ Success | ~2ms |
| **3. Scope Analyzer** | AST | Errors + Symbol tables | ❌ 1 Error | ~1ms |

---

## Error Report

```
═══════════════════════════════════════════════════════
               SCOPE ANALYSIS FAILED
═══════════════════════════════════════════════════════

Errors: 1

[1] UndeclaredVariableAccessed
    Line 6: int b = undeclaredVar + 10;
                    ^^^^^^^^^^^^^
    Variable 'undeclaredVar' is not declared in any 
    accessible scope.
    
Compilation stopped. Cannot proceed to type checking.
═══════════════════════════════════════════════════════
```

---

## Key Concepts Demonstrated

### 1. **Three-Phase Compilation**
   - Separation of concerns
   - Each phase validates different aspects
   - Errors caught at appropriate stage

### 2. **Spaghetti Stack**
   - Efficient scope nesting
   - Parent pointer chain
   - O(depth) lookup time

### 3. **Scope Rules**
   - Variables must be declared before use
   - Lookup searches current scope → parent → grandparent...
   - Declarations add symbols to current scope

### 4. **Error Context**
   - Line and column information preserved from lexer
   - Clear error messages with context
   - Scope chain information for debugging

---

## How to Run

```powershell
# Compile the compiler
C:\msys64\usr\bin\bison.exe -d parser.y
g++ -std=c++17 main.cpp lexer.cpp token.cpp parser.cpp lexer_wrapper.cpp parser.tab.c scope_analyzer.cpp -o output/program.exe

# Run analysis
.\output\program.exe test_undeclared_var.txt
```

---

## Next Phase: Type Checking

The symbol tables created by scope analysis will be used for:
- Type compatibility checking
- Type inference
- Type coercion rules
- Function signature validation

**The scope analyzer is ready for type checking integration!**
