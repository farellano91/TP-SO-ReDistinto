################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/so-commons-library/src/commons/collections/dictionary.c \
../src/so-commons-library/src/commons/collections/list.c \
../src/so-commons-library/src/commons/collections/queue.c 

OBJS += \
./src/so-commons-library/src/commons/collections/dictionary.o \
./src/so-commons-library/src/commons/collections/list.o \
./src/so-commons-library/src/commons/collections/queue.o 

C_DEPS += \
./src/so-commons-library/src/commons/collections/dictionary.d \
./src/so-commons-library/src/commons/collections/list.d \
./src/so-commons-library/src/commons/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
src/so-commons-library/src/commons/collections/%.o: ../src/so-commons-library/src/commons/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


