@echo off
where /q vivado.bat || echo "ERROR: missing vivado.bat - install the Xilinx toolchain" && exit /B
where /q xsct.bat || echo "ERROR: missing xsct.bat - install the Xilinx toolchain" && exit /B

if "%~1" equ "firmware" (
    call :CREATE_FIRMWARE
    goto :EOF
)

if "%~1" equ "fpga" (
    call :CREATE_FPGA
    goto :EOF
)

if "%~1" neq "" (
    echo unknown option %~1
    echo use "firmware" or "fpga" or leave empty for all
    goto :EOF
)

REM create all
call :CREATE_FPGA

:CREATE_FIRMWARE
call :EXIT_IF_EXISTS .\firmware\pynq_z2_pfm
call :EXIT_IF_EXISTS .\firmware\led_switcher_app
call :EXIT_IF_EXISTS .\firmware\led_switcher_app_system
pushd firmware
call xsct.bat ..\create-firmware.tcl
popd
goto :EOF

:CREATE_FPGA
call :EXIT_IF_EXISTS .\fpga\fpga.gen
call :EXIT_IF_EXISTS .\fpga\fpga.hw
call :EXIT_IF_EXISTS .\fpga\fpga.srcs
rd /S /Q fpga_generated > nul 2>&1
mkdir fpga_generated
pushd fpga_generated
call vivado -mode batch -nojournal -source ../create-fpga.tcl -tclargs --origin_dir ../assets/
popd

robocopy .\fpga_generated\fpga .\fpga /S /MOV
rd /S /Q fpga_generated > nul 2>&1

pushd fpga
call vivado -mode batch -nojournal -source ../generate-xsa.tcl
popd
goto :EOF

:EXIT_IF_EXISTS
if exist  %~1 (
    echo WARNING: found %~1 - run clean.bat?
    (goto) 2>nul & endlocal & exit /b %ERRORLEVEL%
)
goto :EOF
