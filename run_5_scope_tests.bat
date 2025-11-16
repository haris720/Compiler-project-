@echo off
echo ========================================
echo    SCOPE ANALYSIS - 5 TEST SUITE
echo ========================================
echo.

echo [TEST 1] Undeclared Variable Access
echo Expected: UndeclaredVariableAccessed error
echo ----------------------------------------
.\output\program.exe scope_test_1_undeclared_var.txt
echo.
echo.

echo [TEST 2] Undefined Function Call
echo Expected: UndefinedFunctionCalled error
echo ----------------------------------------
.\output\program.exe scope_test_2_undefined_func.txt
echo.
echo.

echo [TEST 3] Variable Redefinition
echo Expected: VariableRedefinition error
echo ----------------------------------------
.\output\program.exe scope_test_3_var_redefinition.txt
echo.
echo.

echo [TEST 4] Function Redefinition
echo Expected: FunctionPrototypeRedefinition error
echo ----------------------------------------
.\output\program.exe scope_test_4_func_redefinition.txt
echo.
echo.

echo [TEST 5] Valid Program
echo Expected: Scope analysis passed
echo ----------------------------------------
.\output\program.exe scope_test_5_valid_program.txt
echo.
echo.

echo ========================================
echo    ALL TESTS COMPLETED
echo ========================================
pause
