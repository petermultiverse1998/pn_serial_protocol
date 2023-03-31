################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../user/adapter.c \
../user/uart.c 

OBJS += \
./user/adapter.o \
./user/uart.o 

C_DEPS += \
./user/adapter.d \
./user/uart.d 


# Each subdirectory must supply rules for building sources it contributes
user/%.o user/%.su: ../user/%.c user/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I"C:/Users/NIRUJA/Desktop/Github/np_serial_protocol/np_serial_protocol/user" -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-user

clean-user:
	-$(RM) ./user/adapter.d ./user/adapter.o ./user/adapter.su ./user/uart.d ./user/uart.o ./user/uart.su

.PHONY: clean-user

