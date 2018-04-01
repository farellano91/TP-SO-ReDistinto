################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/so-commons-library/tests/unit-tests/build/subdir.mk
-include src/so-commons-library/tests/unit-tests/subdir.mk
-include src/so-commons-library/tests/integration-tests/logger/subdir.mk
-include src/so-commons-library/src/commons/collections/subdir.mk
-include src/so-commons-library/src/commons/subdir.mk
-include src/so-commons-library/src/build/commons/collections/subdir.mk
-include src/so-commons-library/src/build/commons/subdir.mk
-include src/so-commons-library/cspec/release/cspecs/subdir.mk
-include src/so-commons-library/cspec/cspecs/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: guia_ejercicio_1

# Tool invocations
guia_ejercicio_1: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "guia_ejercicio_1" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) guia_ejercicio_1
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
