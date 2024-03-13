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
	STORAGE_MOUNT_FAIL = -1 ,
	STORAGE_FILE_OPEN_FAIL = -2,
	STORAGE_WRITE_FAIL = -3,
	STORAGE_FLASH_ERROR = -4
}storage_error_t;

typedef struct{

	char version[16];
	char folder_max_count[3];
	char file_max_count[5];

	char r_folder_seek[3];
	char w_folder_seek[3];
	char r_file_seek[5];
	char w_file_seek[5];

}Info_storate_t;

typedef struct{

	SPI_HandleTypeDef *spi_handle ;
	GPIO_TypeDef *gpio_handle ;
	uint16_t pin;

	Info_storate_t info ;

	int(*kv_set)(info_keys_t, char*);
	int(*kv_get)(info_keys_t, char*);
	storate_error_t(*push)(uint8_t *, int);
	storate_error_t(*pop)(uint8_t *, int);

} Storage ;

extern storate_error_t storage_init(Storage * _storage);
extern storate_error_t storate_reset();


#endif
