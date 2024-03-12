################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/littlefs/bd/lfs_emubd.c \
../Core/littlefs/bd/lfs_filebd.c \
../Core/littlefs/bd/lfs_rambd.c 

OBJS += \
./Core/littlefs/bd/lfs_emubd.o \
./Core/littlefs/bd/lfs_filebd.o \
./Core/littlefs/bd/lfs_rambd.o 

C_DEPS += \
./Core/littlefs/bd/lfs_emubd.d \
./Core/littlefs/bd/lfs_filebd.d \
./Core/littlefs/bd/lfs_rambd.d 


# Each subdirectory must supply rules for building sources it contributes
Core/littlefs/bd/%.o Core/littlefs/bd/%.su Core/littlefs/bd/%.cyclo: ../Core/littlefs/bd/%.c Core/littlefs/bd/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/storage" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/littlefs" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/littlefs/bd" -I"/home/afzal/STM32CubeIDE/ws/Lfs_103/Core/spif/inc" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-littlefs-2f-bd

clean-Core-2f-littlefs-2f-bd:
	-$(RM) ./Core/littlefs/bd/lfs_emubd.cyclo ./Core/littlefs/bd/lfs_emubd.d ./Core/littlefs/bd/lfs_emubd.o ./Core/littlefs/bd/lfs_emubd.su ./Core/littlefs/bd/lfs_filebd.cyclo ./Core/littlefs/bd/lfs_filebd.d ./Core/littlefs/bd/lfs_filebd.o ./Core/littlefs/bd/lfs_filebd.su ./Core/littlefs/bd/lfs_rambd.cyclo ./Core/littlefs/bd/lfs_rambd.d ./Core/littlefs/bd/lfs_rambd.o ./Core/littlefs/bd/lfs_rambd.su

.PHONY: clean-Core-2f-littlefs-2f-bd

