################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/so-commons-library/tests/unit-tests/test_bitarray.c \
../src/so-commons-library/tests/unit-tests/test_config.c \
../src/so-commons-library/tests/unit-tests/test_dictionary.c \
../src/so-commons-library/tests/unit-tests/test_list.c \
../src/so-commons-library/tests/unit-tests/test_queue.c \
../src/so-commons-library/tests/unit-tests/test_string.c 

OBJS += \
./src/so-commons-library/tests/unit-tests/test_bitarray.o \
./src/so-commons-library/tests/unit-tests/test_config.o \
./src/so-commons-library/tests/unit-tests/test_dictionary.o \
./src/so-commons-library/tests/unit-tests/test_list.o \
./src/so-commons-library/tests/unit-tests/test_queue.o \
./src/so-commons-library/tests/unit-tests/test_string.o 

C_DEPS += \
./src/so-commons-library/tests/unit-tests/test_bitarray.d \
./src/so-commons-library/tests/unit-tests/test_config.d \
./src/so-commons-library/tests/unit-tests/test_dictionary.d \
./src/so-commons-library/tests/unit-tests/test_list.d \
./src/so-commons-library/tests/unit-tests/test_queue.d \
./src/so-commons-library/tests/unit-tests/test_string.d 


# Each subdirectory must supply rules for building sources it contributes
src/so-commons-library/tests/unit-tests/%.o: ../src/so-commons-library/tests/unit-tests/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


