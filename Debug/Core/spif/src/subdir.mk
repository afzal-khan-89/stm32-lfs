################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/spif/src/spif.c 

OBJS += \
./Core/spif/src/spif.o 

C_DEPS += \
./Core/spif/src/spif.d 


# Each subdirectory must supply rules for building sources it contributes
Core/spif/src/%.o Core/spif/src/%.su Core/spif/src/%.cyclo: ../Core/spif/src/%.c Core/spif/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/littlefs" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/littlefs/bd" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/spif/inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-spif-2f-src

clean-Core-2f-spif-2f-src:
	-$(RM) ./Core/spif/src/spif.cyclo ./Core/spif/src/spif.d ./Core/spif/src/spif.o ./Core/spif/src/spif.su

.PHONY: clean-Core-2f-spif-2f-src

