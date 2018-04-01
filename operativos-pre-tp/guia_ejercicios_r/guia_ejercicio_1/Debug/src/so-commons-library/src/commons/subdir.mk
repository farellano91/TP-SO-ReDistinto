################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/so-commons-library/src/commons/bitarray.c \
../src/so-commons-library/src/commons/config.c \
../src/so-commons-library/src/commons/error.c \
../src/so-commons-library/src/commons/log.c \
../src/so-commons-library/src/commons/process.c \
../src/so-commons-library/src/commons/string.c \
../src/so-commons-library/src/commons/temporal.c \
../src/so-commons-library/src/commons/txt.c 

OBJS += \
./src/so-commons-library/src/commons/bitarray.o \
./src/so-commons-library/src/commons/config.o \
./src/so-commons-library/src/commons/error.o \
./src/so-commons-library/src/commons/log.o \
./src/so-commons-library/src/commons/process.o \
./src/so-commons-library/src/commons/string.o \
./src/so-commons-library/src/commons/temporal.o \
./src/so-commons-library/src/commons/txt.o 

C_DEPS += \
./src/so-commons-library/src/commons/bitarray.d \
./src/so-commons-library/src/commons/config.d \
./src/so-commons-library/src/commons/error.d \
./src/so-commons-library/src/commons/log.d \
./src/so-commons-library/src/commons/process.d \
./src/so-commons-library/src/commons/string.d \
./src/so-commons-library/src/commons/temporal.d \
./src/so-commons-library/src/commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
src/so-commons-library/src/commons/%.o: ../src/so-commons-library/src/commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


