################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/adc_acq.c \
../Core/Src/buttons.c \
../Core/Src/dac.c \
../Core/Src/debounce.c \
../Core/Src/dma.c \
../Core/Src/gpio.c \
../Core/Src/lia_config.c \
../Core/Src/lia_core.c \
../Core/Src/main.c \
../Core/Src/nco_lut.c \
../Core/Src/ref_output.c \
../Core/Src/serial_if.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tim.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/adc_acq.o \
./Core/Src/buttons.o \
./Core/Src/dac.o \
./Core/Src/debounce.o \
./Core/Src/dma.o \
./Core/Src/gpio.o \
./Core/Src/lia_config.o \
./Core/Src/lia_core.o \
./Core/Src/main.o \
./Core/Src/nco_lut.o \
./Core/Src/ref_output.o \
./Core/Src/serial_if.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tim.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/adc_acq.d \
./Core/Src/buttons.d \
./Core/Src/dac.d \
./Core/Src/debounce.d \
./Core/Src/dma.d \
./Core/Src/gpio.d \
./Core/Src/lia_config.d \
./Core/Src/lia_core.d \
./Core/Src/main.d \
./Core/Src/nco_lut.d \
./Core/Src/ref_output.d \
./Core/Src/serial_if.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tim.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/adc_acq.cyclo ./Core/Src/adc_acq.d ./Core/Src/adc_acq.o ./Core/Src/adc_acq.su ./Core/Src/buttons.cyclo ./Core/Src/buttons.d ./Core/Src/buttons.o ./Core/Src/buttons.su ./Core/Src/dac.cyclo ./Core/Src/dac.d ./Core/Src/dac.o ./Core/Src/dac.su ./Core/Src/debounce.cyclo ./Core/Src/debounce.d ./Core/Src/debounce.o ./Core/Src/debounce.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/lia_config.cyclo ./Core/Src/lia_config.d ./Core/Src/lia_config.o ./Core/Src/lia_config.su ./Core/Src/lia_core.cyclo ./Core/Src/lia_core.d ./Core/Src/lia_core.o ./Core/Src/lia_core.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/nco_lut.cyclo ./Core/Src/nco_lut.d ./Core/Src/nco_lut.o ./Core/Src/nco_lut.su ./Core/Src/ref_output.cyclo ./Core/Src/ref_output.d ./Core/Src/ref_output.o ./Core/Src/ref_output.su ./Core/Src/serial_if.cyclo ./Core/Src/serial_if.d ./Core/Src/serial_if.o ./Core/Src/serial_if.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

