#include "local_storage.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "spif.h"
#include "lfs.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#define FIRMWARE_INFO_FILE_NAME  	"info_firmware.txt"
#define STORAGE_INFO_FILE_NAME   	"info_storage.txt"

#define FIRMWARE_DIR_NAME			"firmware_vts"
#define FIRMWARE_FILE_NAME			"vts.bin"
#define FIRMWARE_VERSION			"00.00.01"
#define FIRMWARE_REMOTE_URL			"www.google.com"

#define FOLDER_MAX_COUNT 			12
#define FILE_MAX_COUNT 				2048
#define FOLDER_SEEK_START   		1
#define FILE_SEEK_START 			0

#define STORAGE_INFO_FILE_SIZE      256
#define FIRMWARE_INFO_FILE_SIZE     256

#define INFO_STORAGE_KEY_SIZE 		6
#define INFO_FIRMWARE_KEY_SIZE 		5



static int mount();
Storage_error_t storate_reset();
static int get_store_info();
static int set_store_info();

static Storage_error_t save_data(uint8_t* , uint16_t );
static Storage_error_t read_data(uint8_t* , uint16_t );

static int firmware_write(uint8_t *, uint16_t , uint32_t );
static int firmware_read(uint8_t *, uint16_t , uint32_t );
static Storage_error_t firmware_delete_dir();



const char info_storage_keys[INFO_STORAGE_KEY_SIZE][18] = {"folder-max-count", "file-max-count", "w-folder-seek", "w-file-seek", "r-folder-seek", "r-file-seek"};
const char info_firmware_keys[INFO_FIRMWARE_KEY_SIZE][12] = {"version", "size", "checksum", "remote-url", "local-url"};


typedef enum{
	version,
	size,
	checksum,
	remote_url,
	local_url
}Info_Firmware_keys_t ;

typedef enum{
	folder_max_count,
	file_max_count,
	w_folder_seek,
	w_file_seek,
	r_folder_seek,
	r_file_seek
}Info_Storage_keys_t ;

typedef struct{
	uint16_t folder_max_count;
	uint16_t file_max_count;

	uint16_t r_folder_seek;
	uint16_t w_folder_seek;
	uint16_t r_file_seek;
	uint16_t w_file_seek;

}Info_storate_t;





lfs_t littlefs;
static Storage_t _storage ;
static Info_storate_t _info_storage ;
static int is_initialized = 0 ;


#if defined(FLASH_CHEAP_W25Q16)
SPIF_HandleTypeDef spif_handle = {
		  .PageCnt = 256 ,
		  .SectorCnt = 512 ,
		  .BlockCnt = 32 ,
};
#elif defined(FLASH_CHEAP_W25Q32)
#error FLASH_CHEAP_W25Q32 feature not implemented .
};
#elif defined(FLASH_CHEAP_W25Q128)
SPIF_HandleTypeDef spif_handle = {
		  .PageCnt = 256 ,
		  .SectorCnt = 4096 ,
		  .BlockCnt = 256 ,
};
#else
#error Please select a flash cheap .
#endif




int lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	if(SPIF_ReadAddress(&spif_handle, 4096*block+off, buffer, size) !=  true) return -1;
	return 0;
}

int lfs_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	SPIF_WriteAddress(&spif_handle, 4096*block+off, (void *)buffer, size);
	return 0;
}

int lfs_erase(const struct lfs_config *c, lfs_block_t block)
{
	SPIF_EraseSector(&spif_handle, block);
	return 0;
}

int lfs_sync(const struct lfs_config *c)
{
	return 0;
}




struct lfs_config littlefs_config = {
    .read  = lfs_read,
    .prog  = lfs_write,
    .erase = lfs_erase,
    .sync  = lfs_sync,

    .read_size = 256,
    .prog_size = 256,
//    .block_size = 4096, //byte per sector . which is 4kB
//    .block_count = 4096, // total number of sectors in 25q128
    .block_size = 4096, //byte per sector . which is 4kB
    .block_count = 512,	// total number of sectors in 25q16
    .cache_size = 256,
    .lookahead_size = 8,
    .block_cycles = 100,
};





Storage_error_t init_storage(SPI_HandleTypeDef *spi_handle ,GPIO_TypeDef *gpio_handle, uint16_t pin)
{
	Storage_error_t err = STORAGE_OK ; 				  Info_firmware_t firmware ;
	is_initialized = 0 ;
	if(SPIF_Init(&spif_handle, spi_handle, gpio_handle, pin) == true )
	{
		  if(mount() < 0)
		  {
#ifdef DEBUG
			  printf("::init_storage : lfs mount fail \n"); while(1);
#endif
			  err = STORAGE_MOUNT_FAIL ;
		  }
		  else
		  {
#ifdef DEBUG
			  printf("::init_storage : lfs mount ok \n");   printf("::init_storage : lfs mount ok \n");
#endif
			  if(get_store_info() != 1 )
			  {
  				  _info_storage.folder_max_count = FOLDER_MAX_COUNT ;
  				  _info_storage.file_max_count = FILE_MAX_COUNT ;
  				  _info_storage.w_file_seek = FILE_SEEK_START;
  				  _info_storage.w_folder_seek = FOLDER_SEEK_START;
  				  _info_storage.r_file_seek = FILE_SEEK_START;
  				  _info_storage.r_folder_seek = FOLDER_SEEK_START ;
  				  if(set_store_info() == 1)
  				  {
  					  for(int bcnt=1; bcnt<=12; bcnt++)
  					  {
  						  char b[3] ; memset(b, 0, 3);
  						  itoa(bcnt, b, 10);
  						  if(lfs_mkdir(&littlefs, b) < 0 )
  						  {
#ifdef DEBUG
  							  printf("fail to create storage dir %s\n", b);  while(1);
#endif
  							  err =  STORAGE_DIR_FAIL ;
  							  break;
  						  }
#ifdef DEBUG
  						  printf("create storage dir %s\n", b);
#endif
  					  }
  				  }
  				  else
  				  {
  					  err = STORAGE_ERROR;;
  				  }
			  }
			  if(err == STORAGE_OK)
			  {
				  if(get_firmware_info(&firmware) != 1 )
				  {
					  if(lfs_mkdir(&littlefs,FIRMWARE_DIR_NAME) < 0 )
					  {
#ifdef DEBUG
							  printf("fail to create firmware dir \n");  while(1);
#endif
							  err = STORAGE_DIR_FAIL;
					  }
					  else
					  {
						  firmware.checksum = 210;
						  firmware.size = 0 ;
						  memset(firmware.remote_firmware_url, 0,  sizeof(firmware.remote_firmware_url));
						  memset(firmware.local_firmwre_dir, 0, sizeof(firmware.local_firmwre_dir));
						  memset(firmware.version, 0 ,sizeof(firmware.version));
						  strcpy((char *)firmware.remote_firmware_url, (char*)FIRMWARE_REMOTE_URL);
						  strcpy((char*)firmware.local_firmwre_dir, (char*)FIRMWARE_DIR_NAME);
						  strcpy((char*)firmware.version, (char*)FIRMWARE_VERSION) ;

		  				  if(set_firmware_info(&firmware) != 1)
		  				  {
		  					  err = STORAGE_ERROR;
		  				  }
					  }
				  }
			  }
		  }
		  if(err == STORAGE_OK)
		  {

#ifdef DEBUG
			  printf("storage folder_max_count : %d\n ", _info_storage.folder_max_count);
			  printf("storage file_max_count : %d\n ", _info_storage.file_max_count);
			  printf("storage r_folder_seek : %d\n ", _info_storage.r_folder_seek);
			  printf("storage w_folder_seek : %d\n ", _info_storage.w_folder_seek);
			  printf("storage r_file_seek : %d\n ", _info_storage.r_file_seek);
			  printf("storage w_file_seek : %d\n ", _info_storage.w_file_seek);

			  printf("firmware version : %s\n ", firmware.version);
			  printf("firmware size : %lu \n ", firmware.size);
			  printf("firmware checksum : %lu \n ", firmware.checksum);
			  printf("firmware remote_firmware_url : %s\n ", firmware.remote_firmware_url);
			  printf("firmware local_firmwre_dir : %s\n ", firmware.local_firmwre_dir);
#endif
			 _storage.push = save_data ;
			 _storage.pop = read_data ;

			 is_initialized = 1 ;
		  }
	}
	else
	{
#ifdef DEBUG
		  printf("Flash chip  error ... \n");	while(1);
#endif
		  err = STORAGE_FLASH_ERROR ;
	}
	return err ;
}
Storage_t* get_storage_instance()
{
	return ((is_initialized == 1) ? &_storage : NULL) ;
}

static int mount() {
	int err = 0 ;
 //   lfs_format(&littlefs, &littlefs_config);
	err = lfs_mount(&littlefs, &littlefs_config);
	if(err < 0)
	{
#ifdef DEBUG
		printf("mount:lfs mount fail ... formating ..\n");
#endif
        lfs_format(&littlefs, &littlefs_config);
        err = lfs_mount(&littlefs, &littlefs_config);
	}
    return err;
}


Storage_error_t storate_reset()
{
	lfs_format(&littlefs, &littlefs_config);
	if(mount() < 0)
	{
#ifdef DEBUG
		 printf("storate_reset:lfs mount fail \n");
#endif
		 return  STORAGE_MOUNT_FAIL ;
	}
	return STORAGE_OK ;
}



static int get_store_info()
{
	int success = 0 ;
	int index = 0 , got_key = 0, got_value=0 , key_index =0 , value_index = 0 ;

	uint8_t lfs_read_buffer[STORAGE_INFO_FILE_SIZE];
	lfs_file_t info_file;

	char key_buff[48], value_buff[48];

	memset(lfs_read_buffer, 0, STORAGE_INFO_FILE_SIZE);

	if( lfs_file_open(&littlefs, &info_file, STORAGE_INFO_FILE_NAME, LFS_O_RDWR ) < 0 )
	{
#ifdef DEBUG
			printf("get_store_info -> fail to open %s from save_info \n", STORAGE_INFO_FILE_NAME);
#endif
	}
	else
	{
		  if( lfs_file_read(&littlefs, &info_file, lfs_read_buffer, STORAGE_INFO_FILE_SIZE) < 0 )
		  {
#ifdef DEBUG
			  printf("get_store_info -> fail to read %s from save_info \n", STORAGE_INFO_FILE_NAME); while(1);
#endif
		  }
		  else
		  {
#ifdef DEBUG
			  printf("get_store_info ->  %s  :::  %s \n", STORAGE_INFO_FILE_NAME, lfs_read_buffer);
#endif
		      while(lfs_read_buffer[index])
		      {
	        	  if(lfs_read_buffer[index] == '\"')
	        	  {

	        	  }
	        	  else  if(lfs_read_buffer[index] == '{' || lfs_read_buffer[index] == ',' || lfs_read_buffer[index] == '}')
		          {
			          if(lfs_read_buffer[index] == ',' || lfs_read_buffer[index] == '}')
			          {
#ifdef DEBUG
			        	  printf("get_store_info ->  key:%s   value:%s\n", key_buff, value_buff);
#endif
			        	  for(int i=0; i<INFO_STORAGE_KEY_SIZE; i++)
			        	  {
				              if(memcmp(key_buff, info_storage_keys[i], key_index) == 0)
				              {
				            	  switch(i)
				            	  {
				            	  	  case 0:
				            			  _info_storage.folder_max_count = atoi(value_buff);
				            			  break;
				            		  case 1:
				            			  _info_storage.file_max_count = atoi(value_buff);
				            			  break;
				            		  case 2:
				            			  _info_storage.w_folder_seek = atoi(value_buff);
				            			  break;
				            		  case 3:
				            			  _info_storage.w_file_seek = atoi(value_buff);
				            			  break;
				            		  case 4:
				            			  _info_storage.r_folder_seek = atoi(value_buff);
				            			  break;
				            		  case 5:
				            			  _info_storage.r_file_seek = atoi(value_buff);
				            			  success = 1 ;
				            			  break;
				            		  default:
				            			  break;
				            	  }
				            	  break;
				              }
			        	  }
			          }
			          if(lfs_read_buffer[index] == '}')
			          {
			        	  break;
			          }
			          memset(key_buff, 0, sizeof(key_buff));
			          memset(value_buff, 0, sizeof(value_buff));
		              got_key = 1 ; got_value = 0 ; key_index =0 ; value_index=0  ;
		          }
		          else if(lfs_read_buffer[index] == ':')
		          {
		        	  got_value = 1 ;	got_key = 0;
		          }
		          else
		          {
			          if(got_key)
			          {
			              key_buff[key_index++]= lfs_read_buffer[index] ;
			          }
			          else if(got_value == 1)
			          {
			        	  value_buff[value_index++] = lfs_read_buffer[index];
			          }
		          }
	        	  index++ ;
		      }
		  }
		  lfs_file_close(&littlefs, &info_file);
	}
	return success ;
}

static int set_store_info()
{
	int success = 0 ;

	char lfs_read_buffer[STORAGE_INFO_FILE_SIZE];
	char value_buff[48];
	lfs_file_t file;

	memset(lfs_read_buffer, 0, STORAGE_INFO_FILE_SIZE);
	lfs_read_buffer[0] = '{';
	for(int indx = 0 ; indx < INFO_STORAGE_KEY_SIZE; indx++)
	{
		  strcat(lfs_read_buffer, "\""); strcat(lfs_read_buffer, info_storage_keys[indx]);  strcat(lfs_read_buffer, "\"");
		  strcat(lfs_read_buffer, (char*)":");
		  memset(value_buff, 0, sizeof(value_buff));
		  switch(indx)
		  {
	  	  	  case 0:
	  	  		  itoa(_info_storage.folder_max_count, value_buff, 10);
	  	  		  break;
	  	  	  case 1:
	  	  		  itoa(_info_storage.file_max_count, value_buff, 10);
	  	  		  break;
	  	  	  case 2:
	  	  		  itoa(_info_storage.w_folder_seek, value_buff, 10);
	  	  		  break;
	  	  	  case 3:
	  	  		  itoa(_info_storage.w_file_seek, value_buff, 10);
	  	  		  break;
	  	  	  case 4:
	  	  		  itoa(_info_storage.r_folder_seek, value_buff, 10);
	  	  		  break;
	  	  	  case 5:
	  	  		  itoa(_info_storage.r_file_seek, value_buff, 10);
	  	  		  break;
	  	  	  default:
	  	  		  break;
		  }
		  strcat(lfs_read_buffer, "\""); strcat(lfs_read_buffer, value_buff);  strcat(lfs_read_buffer, "\"");
		  if(indx < INFO_STORAGE_KEY_SIZE-1)	 strcat(lfs_read_buffer, ",");
	}
	strcat(lfs_read_buffer, (char*)"}");

#ifdef DEBUG
	printf("set_store_info :  new info string : %s\n", lfs_read_buffer);
#endif

	if( lfs_file_open(&littlefs, &file, STORAGE_INFO_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT ) < 0 )
	{
#ifdef DEBUG
		  printf("set_store_info: fail to open %s from save_info\n", STORAGE_INFO_FILE_NAME);	while(1);
#endif
	}
	else
	{
		  if(lfs_file_write(&littlefs, &file, lfs_read_buffer, strlen(lfs_read_buffer)) < strlen(lfs_read_buffer))
		  {
#ifdef DEBUG
			  printf("set_store_info :  fail to write new %s from main\n", STORAGE_INFO_FILE_NAME); while(1);
#endif
		  }
		  else
		  {
			  success = 1 ;
		  }
		  lfs_file_close(&littlefs, &file);
	}
    return success ;
}

static Storage_error_t save_data(uint8_t* buffer, uint16_t size)
{
	lfs_file_t file ;
	Storage_error_t err = STORAGE_WRITE_FAIL ;

	char data_path[68];
	char folder_seek[3],file_seek[5];

	int written = 0 ;

	if((_info_storage.w_folder_seek == _info_storage.r_folder_seek) &&  \
			(_info_storage.r_file_seek >_info_storage.w_file_seek ) && \
			(_info_storage.r_file_seek -_info_storage.w_file_seek ) < 2 )
	{
		err = STORATE_FULL;
    }
	else
	{
		memset(folder_seek, 0, sizeof(folder_seek)); 	itoa(_info_storage.w_folder_seek, folder_seek, 10);
		memset(file_seek,   0, sizeof(file_seek));   	itoa(_info_storage.w_file_seek, file_seek, 10);
		memset(data_path,   0, sizeof(data_path));

		strcpy(data_path, folder_seek);	strcat(data_path, "/");	strcat(data_path, file_seek);	strcat(data_path, ".txt");

#ifdef DEBUG
		printf("save_data :  path %s\n", data_path); //HAL_Delay(3000);
#endif

		if( lfs_file_open(&littlefs, &file, data_path, LFS_O_RDWR | LFS_O_CREAT ) < 0 )
		{
#ifdef DEBUG
			 printf("save_data :fail to open data path :  %s\n", data_path);
#endif
			 err = STORAGE_FILE_OPEN_FAIL ;
		}
		else
		{
			  if(lfs_file_write(&littlefs, &file, buffer, size) < 0 )
			  {
#ifdef DEBUG
				  printf("fail to write info.txt \n"); while(1);
#endif
			  }
			  else
			  {
#ifdef DEBUG
				  printf("successfully written to %s \n ", data_path);  //HAL_Delay(3000);
#endif
				  written = 1 ;
			  }
			  lfs_file_close(&littlefs, &file);
		}
		if(written==1)
		{
			  int w_file_seek = _info_storage.w_file_seek;
			  int w_folder_seek = _info_storage.w_folder_seek;

			  if(_info_storage.w_file_seek++ > FILE_MAX_COUNT)
			  {
				  _info_storage.w_file_seek = FILE_SEEK_START ;
				  if(_info_storage.w_folder_seek++ > FOLDER_MAX_COUNT)
				  {
					  _info_storage.w_folder_seek = FOLDER_SEEK_START ;
				  }
			  }
			  if(set_store_info() != 1)
			  {
				  _info_storage.w_file_seek = w_file_seek;
				  _info_storage.w_folder_seek = w_folder_seek;
#ifdef DEBUG
				  printf("save_data : fail to update storage info \n"); while(1);
#endif
			  }
			  else
			  {
				  err = STORAGE_OK;
#ifdef DEBUG
				  printf("save_data : info updated successfully ..  \n");
#endif
			  }
		 }
	}
	return err ;
}

static Storage_error_t read_data(uint8_t* buffer, uint16_t size)
{
	  lfs_file_t file ;
	  Storage_error_t err = STORAGE_WRITE_FAIL ;

	  char data_path[68];
	  char folder_seek[3], file_seek[5];

	  int read_ok = 0 ;

	  if((_info_storage.w_folder_seek == _info_storage.r_folder_seek) && (_info_storage.r_file_seek ==_info_storage.w_file_seek ))
	  {
		  err = STORATE_EMPTY  ;
#ifdef DEBUG
		  printf("read_data : storage empty ... \n");
#endif
	  }
	  else
	  {
		  memset(folder_seek, 0, sizeof(folder_seek)); itoa(_info_storage.r_folder_seek, folder_seek, 10);
		  memset(file_seek,   0, sizeof(file_seek));   itoa(_info_storage.r_file_seek, file_seek, 10);
		  memset(data_path,   0, sizeof(data_path));

		  strcpy(data_path, folder_seek);	strcat(data_path, "/");	 strcat(data_path, file_seek); strcat(data_path, ".txt");

#ifdef DEBUG
		  printf("read_data :  path %s\n", data_path);
#endif

		  if( lfs_file_open(&littlefs, &file, data_path, LFS_O_RDWR ) < 0 )
		  {
#ifdef DEBUG
			  printf("read_data :fail to open data_path \n"); while(1);
#endif
			  err = STORAGE_FILE_OPEN_FAIL ;
		  }
		  else
		  {
			  if(lfs_file_read(&littlefs, &file, buffer, size) < 0 )
			  {
#ifdef DEBUG
				  printf("read_data : fail to read info.txt \n"); while(1);
#endif
				  err = STORAGE_WRITE_FAIL;
			  }
			  else
			  {
#ifdef DEBUG
				  printf("successfully read from %s   \n", data_path);
#endif
				  read_ok = 1;
			  }
			  lfs_file_close(&littlefs, &file);
		  }
		  if(read_ok == 1)
		  {
			  int r_file_seek = _info_storage.r_file_seek;
			  int r_folder_seek = _info_storage.r_folder_seek;

			  if(_info_storage.r_file_seek++ > FILE_MAX_COUNT)
			  {
				  _info_storage.r_file_seek = FILE_SEEK_START ;
				  if(_info_storage.r_folder_seek++ > FOLDER_MAX_COUNT)
				  {
					  _info_storage.r_folder_seek = FOLDER_SEEK_START ;
				  }
			  }
			  if(set_store_info() != 1)
			  {
				  _info_storage.r_file_seek = r_file_seek;
				  _info_storage.r_folder_seek = r_folder_seek;
#ifdef DEBUG
				  printf("read_data : fail to update storage info \n"); while(1);
#endif
			  }
			  else
			  {
				  err = STORAGE_OK;
			  }
		  }
	  }
	  return err ;
}

uint8_t  get_firmware_info(Info_firmware_t * _firmware)
{
	int success = 0 ;
	int index = 0 , got_key = 0, got_value=0 , key_index =0 , value_index = 0 ;

	uint8_t lfs_read_buffer[FIRMWARE_INFO_FILE_SIZE];
	lfs_file_t info_file;

	char key_buff[48], value_buff[48];

	memset(lfs_read_buffer, 0, FIRMWARE_INFO_FILE_SIZE);

	if( lfs_file_open(&littlefs, &info_file, FIRMWARE_INFO_FILE_NAME, LFS_O_RDWR ) < 0 )
	{
#ifdef DEBUG
			printf("get_store_info -> fail to open %s from save_info \n", FIRMWARE_INFO_FILE_NAME);
#endif
	}
	else
	{
		  if( lfs_file_read(&littlefs, &info_file, lfs_read_buffer, FIRMWARE_INFO_FILE_SIZE) < 0 )
		  {
#ifdef DEBUG
			  printf("get_store_info -> fail to read %s from save_info \n", FIRMWARE_INFO_FILE_NAME); while(1);
#endif
		  }
		  else
		  {
#ifdef DEBUG
			  printf("get_store_info ->  %s  :::  %s \n", FIRMWARE_INFO_FILE_NAME, lfs_read_buffer);
#endif
		      while(lfs_read_buffer[index])
		      {
	        	  if(lfs_read_buffer[index] == '\"')
	        	  {

	        	  }
	        	  else  if(lfs_read_buffer[index] == '{' || lfs_read_buffer[index] == ',' || lfs_read_buffer[index] == '}')
		          {
			          if(lfs_read_buffer[index] == ',' || lfs_read_buffer[index] == '}')
			          {
#ifdef DEBUG
			        	  printf("get_store_info ->  key:%s   value:%s\n", key_buff, value_buff);
#endif
			        	  for(int i=0; i<INFO_FIRMWARE_KEY_SIZE; i++)
			        	  {
				              if(memcmp(key_buff, info_firmware_keys[i], key_index) == 0)
				              {
				            	  switch(i)
				            	  {
				            	  	  case 0:
				            	  		  memset(_firmware->version, 0, sizeof(_firmware->version));
				            	  		  memcpy(_firmware->version, value_buff, value_index);
				            			  break;
				            		  case 1:
				            			  _firmware->size = atoi(value_buff);
				            			  break;
				            		  case 2:
				            			  _firmware->checksum = atoi(value_buff);
				            			  break;
				            		  case 3:
				            			  memset(_firmware->remote_firmware_url, 0, sizeof(_firmware->remote_firmware_url));
				            			  memcpy(_firmware->remote_firmware_url, value_buff, value_index);
				            			  break;
				            		  case 4:
				            			  memset(_firmware->local_firmwre_dir, 0, sizeof(_firmware->local_firmwre_dir));
				            			  memcpy(_firmware->local_firmwre_dir, value_buff, value_index);
				            			  success = 1 ;
				            			  break;
				            		  default:
				            			  break;
				            	  }
				            	  break;
				              }
			        	  }
			          }
			          if(lfs_read_buffer[index] == '}')
			          {
			        	  break;
			          }
			          memset(key_buff, 0, sizeof(key_buff));
			          memset(value_buff, 0, sizeof(value_buff));
		              got_key = 1 ; got_value = 0 ; key_index =0 ; value_index=0  ;
		          }
		          else if(lfs_read_buffer[index] == ':')
		          {
		        	  got_value = 1 ;	got_key = 0;
		          }
		          else
		          {
			          if(got_key)
			          {
			              key_buff[key_index++]= lfs_read_buffer[index] ;
			          }
			          else if(got_value == 1)
			          {
			        	  value_buff[value_index++] = lfs_read_buffer[index];
			          }
		          }
	        	  index++ ;
		      }
		  }
		  lfs_file_close(&littlefs, &info_file);
	}
	if(success == 1)
	{
		_firmware->read = firmware_read;
		_firmware->write = firmware_write;
		_firmware->remove = firmware_delete_dir ;
	}
	return success ;
}

uint8_t set_firmware_info(Info_firmware_t * _firmware)
{
	int success = 0 ;

	char lfs_read_buffer[FIRMWARE_INFO_FILE_SIZE];
	char value_buff[48];
	lfs_file_t file;

	memset(lfs_read_buffer, 0, FIRMWARE_INFO_FILE_SIZE);
	lfs_read_buffer[0] = '{';
	for(int indx = 0 ; indx < INFO_FIRMWARE_KEY_SIZE; indx++)
	{
		  strcat(lfs_read_buffer, "\""); strcat(lfs_read_buffer, info_firmware_keys[indx]);  strcat(lfs_read_buffer, "\"");
		  strcat(lfs_read_buffer, (char*)":");
		  memset(value_buff, 0, sizeof(value_buff));
		  switch(indx)
		  {
	  	  	  case 0:
	  	  		  strcat(value_buff, (char*)_firmware->version);
	  	  		  break;
	  	  	  case 1:
	  	  		  itoa(_firmware->size, value_buff, 10);
	  	  		  break;
	  	  	  case 2:
	  	  		  itoa(_firmware->checksum, value_buff, 10);
	  	  		  break;
	  	  	  case 3:
	  	  		  strcat(value_buff, (char*)_firmware->remote_firmware_url);
	  	  		  break;
	  	  	  case 4:
	  	  		  strcat(value_buff, (char*)_firmware->local_firmwre_dir);
	  	  		  break;
	  	  	  default:
	  	  		  break;
		  }
		  strcat(lfs_read_buffer, "\""); strcat(lfs_read_buffer, value_buff);  strcat(lfs_read_buffer, "\"");
		  if(indx < INFO_FIRMWARE_KEY_SIZE-1)	 strcat(lfs_read_buffer, ",");
	}
	strcat(lfs_read_buffer, (char*)"}");

#ifdef DEBUG
	printf("set_store_info :  new firmware string : %s\n", lfs_read_buffer);
#endif

	if( lfs_file_open(&littlefs, &file, FIRMWARE_INFO_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT ) < 0 )
	{
#ifdef DEBUG
		  printf("set_store_info: fail to open %s from save_info\n", FIRMWARE_INFO_FILE_NAME);	while(1);
#endif
	}
	else
	{
		  if(lfs_file_write(&littlefs, &file, lfs_read_buffer, strlen(lfs_read_buffer)) < strlen(lfs_read_buffer))
		  {
#ifdef DEBUG
			  printf("set_store_info :  fail to write new %s from main\n", FIRMWARE_INFO_FILE_NAME); while(1);
#endif
		  }
		  else
		  {
			  success = 1 ;
		  }
		  lfs_file_close(&littlefs, &file);
	}
    return success ;
}

static int firmware_write(uint8_t *buff, uint16_t size, uint32_t seek)
{
	  lfs_file_t file ;
	  int write_size = -1 ;
	  char firmware_path[64];

	  memset(firmware_path, 0, sizeof(firmware_path));

	  strcpy(firmware_path, FIRMWARE_DIR_NAME); strcat(firmware_path, FIRMWARE_FILE_NAME);

	  if( lfs_file_open( &littlefs, &file, firmware_path, LFS_O_RDWR )  < 0 )
	  {
#ifdef DEBUG
		  printf("firmware_read :fail to open firmware dir %s \n", firmware_path); while(1);
#endif
	  }
	  else
	  {
		  write_size = lfs_file_write(&littlefs, &file, firmware_path, size) ;
		  if(write_size < 0)
		  {
#ifdef DEBUG
			  printf("firmware_write :  fail to write firmware at seek %lu seek \n", seek); while(1);
#endif
		  }
		  else
		  {
#ifdef DEBUG
			  printf("firmware_read : successfully write %d byte at dir %s   \n", write_size, firmware_path);
#endif
		  }
		  lfs_file_close(&littlefs, &file);
	  }
	  return write_size ;
}
static int firmware_read(uint8_t *buff, uint16_t size, uint32_t seek)
{
	  lfs_file_t file ;
	  int read_size = -1 ;
	  char firmware_path[64];

	  memset(firmware_path, 0, sizeof(firmware_path));

	  strcpy(firmware_path, FIRMWARE_DIR_NAME); strcat(firmware_path, FIRMWARE_FILE_NAME);

	  if( lfs_file_open( &littlefs, &file, firmware_path, LFS_O_RDWR )  < 0 )
	  {
#ifdef DEBUG
		  printf("firmware_read :fail to open firmware dir %s \n", firmware_path); while(1);
#endif
	  }
	  else
	  {
		  read_size = lfs_file_read(&littlefs, &file, buff, size) ;
		  if(read_size < 0 )
		  {
#ifdef DEBUG
			  printf("firmware_read : fail to read firmware at seek %lu \n", seek); while(1);
#endif
		  }
		  else
		  {
#ifdef DEBUG
			  printf("firmware_read : successfully read from %s   \n", firmware_path);
#endif
		  }
		  lfs_file_close(&littlefs, &file);
	  }
	  return read_size ;
}
static Storage_error_t firmware_delete_dir()
{
	return STORAGE_OK ;
}




































































