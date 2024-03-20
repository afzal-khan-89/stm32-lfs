################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/storage/local_storage.c 

OBJS += \
./Core/storage/local_storage.o 

C_DEPS += \
./Core/storage/local_storage.d 


# Each subdirectory must supply rules for building sources it contributes
Core/storage/%.o Core/storage/%.su Core/storage/%.cyclo: ../Core/storage/%.c Core/storage/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage/littlefs" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage/littlefs/bd" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage/spif/inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-storage

clean-Core-2f-storage:
	-$(RM) ./Core/storage/local_storage.cyclo ./Core/storage/local_storage.d ./Core/storage/local_storage.o ./Core/storage/local_storage.su

.PHONY: clean-Core-2f-storage

