################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/littlefs/lfs.c \
../Core/littlefs/lfs_util.c 

OBJS += \
./Core/littlefs/lfs.o \
./Core/littlefs/lfs_util.o 

C_DEPS += \
./Core/littlefs/lfs.d \
./Core/littlefs/lfs_util.d 


# Each subdirectory must supply rules for building sources it contributes
Core/littlefs/%.o Core/littlefs/%.su Core/littlefs/%.cyclo: ../Core/littlefs/%.c Core/littlefs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/littlefs" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/littlefs/bd" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/spif/inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-littlefs

clean-Core-2f-littlefs:
	-$(RM) ./Core/littlefs/lfs.cyclo ./Core/littlefs/lfs.d ./Core/littlefs/lfs.o ./Core/littlefs/lfs.su ./Core/littlefs/lfs_util.cyclo ./Core/littlefs/lfs_util.d ./Core/littlefs/lfs_util.o ./Core/littlefs/lfs_util.su

.PHONY: clean-Core-2f-littlefs

