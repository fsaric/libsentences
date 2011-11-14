# This module finds re2c executable and adds macro
#  RE2C(SOURCE TARGET)

find_program(re2c_exe re2c)

if(NOT re2c_exe)
    message(FATAL_ERROR "Can't find re2c.")
endif(NOT re2c_exe)

macro(re2c SOURCE TARGET)
    add_custom_command(
        OUTPUT "${TARGET}"
        COMMAND "${re2c_exe}"
        -s -o "${TARGET}"
        "${SOURCE}"
        DEPENDS "${SOURCE}")
endmacro(re2c)

mark_as_advanced(re2c_exe)
