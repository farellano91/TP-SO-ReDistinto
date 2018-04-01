################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/so-commons-library/tests/integration-tests/logger/main.c 

OBJS += \
./src/so-commons-library/tests/integration-tests/logger/main.o 

C_DEPS += \
./src/so-commons-library/tests/integration-tests/logger/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/so-commons-library/tests/integration-tests/logger/%.o: ../src/so-commons-library/tests/integration-tests/logger/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


