#ifndef _storage_h
#define _storage_h

#include "main.h"

typedef enum{
	version_info,
	folder_seek,
	folder_max,
	file_seek,
	file_max
}info_keys_t ;



typedef struct{

	SPI_HandleTypeDef *spi ;
	GPIO_TypeDef *gpio ;
	uint16_t pin;

	int(*kv_set)(info_keys_t, char*);
	int(*kv_get)(info_keys_t, char*);
	int(*save_location)(uint8_t *);
	int(*get_location)(uint8_t *);

} Storage ;

extern int storage_init(Storage * _storage);


#endif
