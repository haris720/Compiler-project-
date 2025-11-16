@echo off
echo ==========================================
echo Running Type Checking Tests
echo ==========================================
echo.

echo Test 1: Function Parameter Count Mismatch
echo ------------------------------------------
.\output\program.exe type_test_1_param_count.txt
echo.
echo.

echo Test 2: Function Parameter Type Mismatch
echo ------------------------------------------
.\output\program.exe type_test_2_param_type.txt
echo.
echo.

echo Test 3: Missing Return Statement
echo ------------------------------------------
.\output\program.exe type_test_3_no_return.txt
echo.
echo.

echo Test 4: Wrong Return Type
echo ------------------------------------------
.\output\program.exe type_test_4_wrong_return.txt
echo.
echo.

echo Test 5: Variable Declaration Type Mismatch
echo ------------------------------------------
.\output\program.exe type_test_5_var_decl.txt
echo.
echo.

echo Test 6: Boolean Operations on Non-Boolean
echo ------------------------------------------
.\output\program.exe type_test_6_bool_ops.txt
echo.
echo.

echo Test 7: Bitwise Operations on Non-Integer
echo ------------------------------------------
.\output\program.exe type_test_7_bitwise.txt
echo.
echo.

echo Test 8: Valid Program (Should Pass)
echo ------------------------------------------
.\output\program.exe type_test_8_valid.txt
echo.
echo.

echo ==========================================
echo Type Checking Tests Complete
echo ==========================================
pause
