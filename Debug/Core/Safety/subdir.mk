################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Safety/safety.c 

OBJS += \
./Core/Safety/safety.o 

C_DEPS += \
./Core/Safety/safety.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Safety/%.o Core/Safety/%.su Core/Safety/%.cyclo: ../Core/Safety/%.c Core/Safety/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Config -I../Core/App -I../Core/Safety -I../Core/Control -I../Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Safety

clean-Core-2f-Safety:
	-$(RM) ./Core/Safety/safety.cyclo ./Core/Safety/safety.d ./Core/Safety/safety.o ./Core/Safety/safety.su

.PHONY: clean-Core-2f-Safety

