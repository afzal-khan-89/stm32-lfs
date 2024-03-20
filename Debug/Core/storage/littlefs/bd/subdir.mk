################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/storage/littlefs/bd/lfs_emubd.c \
../Core/storage/littlefs/bd/lfs_filebd.c \
../Core/storage/littlefs/bd/lfs_rambd.c 

OBJS += \
./Core/storage/littlefs/bd/lfs_emubd.o \
./Core/storage/littlefs/bd/lfs_filebd.o \
./Core/storage/littlefs/bd/lfs_rambd.o 

C_DEPS += \
./Core/storage/littlefs/bd/lfs_emubd.d \
./Core/storage/littlefs/bd/lfs_filebd.d \
./Core/storage/littlefs/bd/lfs_rambd.d 


# Each subdirectory must supply rules for building sources it contributes
Core/storage/littlefs/bd/%.o Core/storage/littlefs/bd/%.su Core/storage/littlefs/bd/%.cyclo: ../Core/storage/littlefs/bd/%.c Core/storage/littlefs/bd/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage/littlefs" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage/littlefs/bd" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage/spif/inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-storage-2f-littlefs-2f-bd

clean-Core-2f-storage-2f-littlefs-2f-bd:
	-$(RM) ./Core/storage/littlefs/bd/lfs_emubd.cyclo ./Core/storage/littlefs/bd/lfs_emubd.d ./Core/storage/littlefs/bd/lfs_emubd.o ./Core/storage/littlefs/bd/lfs_emubd.su ./Core/storage/littlefs/bd/lfs_filebd.cyclo ./Core/storage/littlefs/bd/lfs_filebd.d ./Core/storage/littlefs/bd/lfs_filebd.o ./Core/storage/littlefs/bd/lfs_filebd.su ./Core/storage/littlefs/bd/lfs_rambd.cyclo ./Core/storage/littlefs/bd/lfs_rambd.d ./Core/storage/littlefs/bd/lfs_rambd.o ./Core/storage/littlefs/bd/lfs_rambd.su

.PHONY: clean-Core-2f-storage-2f-littlefs-2f-bd

