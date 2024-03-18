#ifndef _storage_h
#define _storage_h

#include "main.h"


/* Total string 21846 */

typedef enum{
	version_info,
	folder_max_count,
	file_max_count,
	w_folder_seek,
	w_file_seek,
	r_folder_seek,
	r_file_seek
}info_keys_t ;

typedef enum {
	STORAGE_OK = 0 ,
	STORATE_FULL = -1 ,
	STORATE_EMPTY = -2,
	STORAGE_MOUNT_FAIL = -3 ,
	STORAGE_FILE_OPEN_FAIL = -4,
	STORAGE_WRITE_FAIL = -5,
	STORAGE_FLASH_ERROR = -6
}storage_error_t;

typedef struct{

	char version[16];
	uint16_t folder_max_count;
	uint16_t file_max_count;

	uint16_t r_folder_seek;
	uint16_t w_folder_seek;
	uint16_t r_file_seek;
	uint16_t w_file_seek;

}Info_storate_t;

typedef struct{

	SPI_HandleTypeDef *spi_handle ;
	GPIO_TypeDef *gpio_handle ;
	uint16_t pin;

	int(*kv_set)(info_keys_t, char*);
	int(*kv_get)(info_keys_t, char*);

	storage_error_t(*push)(uint8_t *, int);
	storage_error_t(*pop)(uint8_t *, int);

} Storage_t ;

extern storage_error_t storage_init(Storage_t * _storage);
extern storage_error_t storate_reset();


#endif




































