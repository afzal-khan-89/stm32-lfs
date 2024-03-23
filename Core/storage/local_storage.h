#ifndef _local_storage_h
#define _local_storage_h

#include "main.h"

//#define FLASH_CHEAP_W25Q128
//#define FLASH_CHEAP_W25Q32
#define FLASH_CHEAP_W25Q16


typedef enum{
	STORAGE_OK = 0 ,
	STORAGE_ERROR = -1,
	STORATE_FULL = -2 ,
	STORATE_EMPTY = -3,
	STORAGE_MOUNT_FAIL = -43 ,
	STORAGE_FILE_OPEN_FAIL = -5,
	STORAGE_WRITE_FAIL = -6,
	STORAGE_READ_FAIL = -7,
	STORAGE_DIR_FAIL = -8,
	STORAGE_REMOVE_FAIL = -9,
	STORAGE_FLASH_ERROR = -10
}Storage_error_t ;

typedef struct{
	uint32_t size;
	uint32_t checksum;
	uint8_t version[16] ;
	uint8_t remote_firmware_url[128];
	uint8_t local_firmwre_dir[48];
	int ( *read )( uint8_t *, uint16_t , uint32_t );
	int ( *write )( uint8_t *, uint16_t );
	Storage_error_t(*remove)();
}Info_firmware_t;


typedef struct{

	Storage_error_t(*push)(uint8_t *, uint16_t );
	Storage_error_t(*pop)(uint8_t *, uint16_t );

}Storage_t;



extern Storage_error_t init_storage(SPI_HandleTypeDef *spi_handle ,GPIO_TypeDef *gpio_handle, uint16_t pin);
extern Storage_t * get_storage_instance();

extern uint8_t  get_firmware_info(Info_firmware_t * _firmware);
extern uint8_t  set_firmware_info(Info_firmware_t * _firmware);


#endif






























