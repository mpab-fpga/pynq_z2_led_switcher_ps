@echo off

if "%~1" equ "firmware" (
    call :REMOVE_FIRMWARE
    goto :EOF
)

if "%~1" equ "fpga" (
    call :REMOVE_FPGA
    goto :EOF
)

if "%~1" neq "" (
    echo unknown option %~1
    echo use "firmware" or "fpga" or leave empty for all
    goto :EOF
)

REM remove all
call :REMOVE .\fpga_generated
call :REMOVE_FPGA

:REMOVE_FIRMWARE
echo removing .\firmware
call :REMOVE .\firmware
mkdir .\firmware
touch .\firmware\.keep
goto :EOF

:REMOVE_FPGA
echo removing .\fpga
call :REMOVE .\fpga
mkdir .\fpga
touch .\fpga\.keep
goto :EOF

goto :EOF

:REMOVE
if exist %~1 rd /S /Q %~1 > nul 2>&1
if exist %~1 del %~1 > nul 2>&1
goto :EOF
