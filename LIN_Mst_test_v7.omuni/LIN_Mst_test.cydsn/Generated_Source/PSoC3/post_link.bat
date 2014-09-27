@REM This script allows a 3rd party IDE to use CyHexTool to perform
@REM any post processing that is necessary to convert the raw flash
@REM image into a complete hex file to use in programming the PSoC.
@REM USAGE: post_link.bat
@REM    arg1: Persistant path back to the directory containing the app project.
@REM    arg2: Path (relative to arg1) of the directory where the hex files go.
@REM    arg3: Name of the project.
@REM NOTE: This script is auto generated. Do not modify.

"C:\Program Files\Cypress\PSoC Creator\3.0\PSoC Creator\bin\cyvalidateide.exe" -dev CY8C3245AXI-158 -ide "%~1\%~3" -flsAddr 0x0 -flsSize 0x8000 -sramAddr 0x0 -sramSize 0x1000
@IF %errorlevel% NEQ 0 EXIT /b %errorlevel% 
move "%~1\%~2\%~n3.hex" "%~1\%~2\%~n3.ihx"
@IF %errorlevel% NEQ 0 EXIT /b %errorlevel% 
"C:\Program Files\Cypress\PSoC Creator\3.0\PSoC Creator\import\gnu_cs\c8051\1.0\bin\c8051-elf-omf2elf.exe" "%~1\%~2\%~n3" "%~1\%~2\%~n3.elf"
@IF %errorlevel% NEQ 0 EXIT /b %errorlevel% 
"C:\Program Files\Cypress\PSoC Creator\3.0\PSoC Creator\bin\cyhextool" -o "%~1\%~2\%~n3.hex" -f "%~1\%~2\%~n3.ihx" -prot "%~dp0protect.hex" -id 1E09E069 -a EEPROM=90200000:400,PROGRAM=00000000:8000,CONFIG=80000000:1000,PROTECT=90400000:20 -meta 0301 -cunv 00004005 -wonv BC90ACAF -ecc "%~dp0config.hex" 
@IF %errorlevel% NEQ 0 EXIT /b %errorlevel% 
CD /D "C:\Keil\UV4"
@IF %errorlevel% NEQ 0 EXIT /b %errorlevel% 
IF NOT EXIST "C:\Users\tatsuya\Documents\GitHub\omuni_base\LIN_Mst_test_v7.omuni\LIN_Mst_test.cydsn\LIN_Mst_test.svd" rem "C:\Users\tatsuya\Documents\GitHub\omuni_base\LIN_Mst_test_v7.omuni\LIN_Mst_test.cydsn\LIN_Mst_test.sfr"
@IF %errorlevel% NEQ 0 EXIT /b %errorlevel% 
