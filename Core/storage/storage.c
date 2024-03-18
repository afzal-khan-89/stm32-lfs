#include "storage.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "spif.h"
#include "lfs.h"


static int set_info(info_keys_t , char * );
static int get_info(info_keys_t , char * );
static storage_error_t save_data(uint8_t* buffer, int size);
static storage_error_t read_data(uint8_t* buffer, int size);

static int get_store_info();


lfs_t littlefs;
//SPIF_HandleTypeDef spif_handle = {
//		  .PageCnt = 256 ,
//		  .SectorCnt = 512 ,
//		  .BlockCnt = 32 ,
//};
SPIF_HandleTypeDef spif_handle = {
		  .PageCnt = 256 ,
		  .SectorCnt = 4096 ,
		  .BlockCnt = 256 ,
};


#define FOLDER_MAX_COUNT 	12
#define FILE_MAX_COUNT 		2048

#define FOLDER_SEEK_START   1
#define FILE_SEEK_START 	1


const uint8_t vts_info[]="{\"version\":\"0.0.00\",\"folder-max-count\":\"12\",\"file-max-count\":\"2048\",\"w-folder-seek\":\"1\","
"\"w-file-seek\":\"0\",\"r-folder-seek\":\"1\",\"r-file-seek\":\"0\"}" ;

static Info_storate_t _info_storage ;


#define INFO_KEY_SIZE 7
const char info_keys[INFO_KEY_SIZE][18] = {"version", "folder-max-count", "file-max-count", "w-folder-seek", "w-file-seek", "r-folder-seek", "r-file-seek"};


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
//	printf("LittleFS Sync");
	return 0;
}


struct lfs_config littlefs_config = {
    .read  = lfs_read,
    .prog  = lfs_write,
    .erase = lfs_erase,
    .sync  = lfs_sync,

    .read_size = 256,
    .prog_size = 256,
    .block_size = 4096, //65536, //4096, // lets use sector size . which is 4kB
    .block_count = 4096, //256, //512,	// lets try with sector count . each block has 16 sector . thus 32 block has 512 sector .
    .cache_size = 256,
    .lookahead_size = 8,
    .block_cycles = 100,
};



static int mount() {
	int err = 0 ;
    //lfs_format(&littlefs, &littlefs_config);
	err = lfs_mount(&littlefs, &littlefs_config);
	if(err < 0)
	{
		printf("lfs mount fail ... formating ..\n");
        lfs_format(&littlefs, &littlefs_config);
        err = lfs_mount(&littlefs, &littlefs_config);
	}
    return err;
}
storage_error_t storate_reset()
{
	lfs_format(&littlefs, &littlefs_config);
	if(mount() < 0)
	{
		 printf("lfs mount fail \n"); while(1);
		 return  STORAGE_MOUNT_FAIL ;
	}
	return STORAGE_OK ;
}
storage_error_t storage_init(Storage_t * _storage)
{
	storage_error_t err = STORAGE_OK ;
	lfs_file_t file;

	if(SPIF_Init(&spif_handle, _storage->spi_handle, _storage->gpio_handle, _storage->pin) == true )
	{
		  if(mount() < 0)
		  {
			  printf("lfs mount fail \n"); while(1);
			  err = STORAGE_MOUNT_FAIL ;
		  }
		  else
		  {
			  printf("lfs mount ok \n");
			  if(get_store_info() != 1 )
			  {
	  			  if( lfs_file_open(&littlefs, &file, "info.txt", LFS_O_RDWR | LFS_O_CREAT) < 0 )
	  			  {
	  				 printf("fail to open info.txt \n"); while(1);
	  				 err = STORAGE_FILE_OPEN_FAIL ;
	  			  }
	  			  else
	  			  {
  	  				  if(lfs_file_write(&littlefs, &file, vts_info, sizeof(vts_info)) < 0 )
  	  				  {
  	  					 err = STORAGE_WRITE_FAIL ;
  	  					 printf("storage_init : fail write initial info  \n"); while(1);
  	  				  }
  	  				  else
  	  				  {
  	  					  for(int bcnt=1; bcnt<=12; bcnt++)
  	  					  {
  	  						  char b[3] ; memset(b, 0, 3);
  	  						  itoa(bcnt, b, 10);
    	  	  				  if(lfs_mkdir(&littlefs, b) < 0 )
    	  	  				  {
    	  	  					  printf("fail to create storage dir %s\n", b); while(1);
    	  	  					  err = STORAGE_WRITE_FAIL;
    	  	  					  break;
    	  	  				  }
    	  	  				  printf("create storage dir %s\n", b);
  	  					  }
  	  				  }
  	  				  lfs_file_close(&littlefs, &file);

  	  				  if(err == STORAGE_OK)
  	  				  {
  	  					  if(get_store_info() != 1 )
  	  					  {
  	  							 printf("storage_init : fail to get storage info \n"); while(1);
  	  					  }
  	  				  }
	  			  }
			  }
		  }
		  if(err == STORAGE_OK)
		  {
#ifdef DEBUG

			  printf("storage version : %s\n ", _info_storage.version);
			  printf("storage folder_max_count : %d\n ", _info_storage.folder_max_count);
			  printf("storage file_max_count : %d\n ", _info_storage.file_max_count);
			  printf("storage r_folder_seek : %d\n ", _info_storage.r_folder_seek);
			  printf("storage w_folder_seek : %d\n ", _info_storage.w_folder_seek);
			  printf("storage r_file_seek : %d\n ", _info_storage.r_file_seek);
			  printf("storage w_file_seek : %d\n ", _info_storage.w_file_seek);

#endif
			 _storage->kv_get = get_info ;
			 _storage->kv_set = set_info ;
			 _storage->push = save_data ;
			 _storage->pop = read_data ;
		  }
	}
	else
	{
		  printf("Flash chip  error ... \n");
		  err = STORAGE_FLASH_ERROR ;
	}
	return err ;
}
static int get_store_info()
{
	int success = 0 ;
	int index = 0 , got_key = 0, got_value=0 , key_index =0 , value_index = 0 ;

	uint8_t lfs_read[256];
	lfs_file_t info_file;

	char key_buff[32], value_buff[48];

	memset(lfs_read, 0, 256);
	if( lfs_file_open(&littlefs, &info_file, "info.txt", LFS_O_RDWR ) < 0 )
	{
		printf("fail to open info.txt from save_info\n");
	}
	else
	{
		  if( lfs_file_read(&littlefs, &info_file, lfs_read, 256) < 0 )
		  {
			  printf("get_store_info:fail to read info.txt from save_info \n");
		  }
		  else
		  {
			  printf("get_store_info : info.txt  :::  %s \n", lfs_read);
		      while(lfs_read[index])
		      {
	        	  if(lfs_read[index] == '\"')
	        	  {

	        	  }
	        	  else  if(lfs_read[index] == '{' || lfs_read[index] == ',' || lfs_read[index] == '}')
		          {
			          if(lfs_read[index] == ',' || lfs_read[index] == '}')
			          {
			        	  printf("key:%s   value:%s\n", key_buff, value_buff);
			        	  for(int i=0; i<INFO_KEY_SIZE; i++)
			        	  {
				              if(memcmp(key_buff, info_keys[i], sizeof(info_keys[i])) == 0)
				              {
				            	  switch(i)
				            	  {
				            	  	  case 0:
				            	  		  memcpy(_info_storage.version, value_buff, value_index);
				            	  		  break;
				            		  case 1:
				            			  _info_storage.folder_max_count = atoi(value_buff);
				            			  break;
				            		  case 2:
				            			  _info_storage.file_max_count = atoi(value_buff);
				            			  break;
				            		  case 3:
				            			  _info_storage.w_folder_seek = atoi(value_buff);
				            			  break;
				            		  case 4:
				            			  _info_storage.w_file_seek = atoi(value_buff);
				            			  break;
				            		  case 5:
				            			  _info_storage.r_folder_seek = atoi(value_buff);
				            			  break;
				            		  case 6:
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
			          if(lfs_read[index] == '}')
			          {
			        	  break;
			          }
			          memset(key_buff, 0, sizeof(key_buff));
			          memset(value_buff, 0, sizeof(value_buff));
		              got_key = 1 ; got_value = 0 ; key_index =0 ; value_index=0  ;
		          }
		          else if(lfs_read[index] == ':')
		          {
		        	  got_value = 1 ;	got_key = 0;
		          }
		          else
		          {
			          if(got_key)
			          {
			              key_buff[key_index++]= lfs_read[index] ;
			          }
			          else if(got_value == 1)
			          {
			        	  value_buff[value_index++] = lfs_read[index];
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

	char lfs_read[256];
	char value_buff[48];
	lfs_file_t file;

	memset(lfs_read, 0, 256);
    lfs_read[0] = '{';
	for(int indx = 0 ; indx < INFO_KEY_SIZE; indx++)
	{
		  strcat(lfs_read, "\""); strcat(lfs_read, info_keys[indx]);  strcat(lfs_read, "\"");
		  strcat(lfs_read, (char*)":");
		  memset(value_buff, 0, sizeof(value_buff));
		  switch(indx)
		  {
	  	  	  case 0:
	  	  		  strcpy(value_buff, _info_storage.version);
	  	  		  break;
	  	  	  case 1:
	  	  		  itoa(_info_storage.folder_max_count, value_buff, 10);
	  	  		  break;
	  	  	  case 2:
	  	  		  itoa(_info_storage.file_max_count, value_buff, 10);
	  	  		  break;
	  	  	  case 3:
	  	  		  itoa(_info_storage.w_folder_seek, value_buff, 10);
	  	  		  break;
	  	  	  case 4:
	  	  		  itoa(_info_storage.w_file_seek, value_buff, 10);
	  	  		  break;
	  	  	  case 5:
	  	  		  itoa(_info_storage.r_folder_seek, value_buff, 10);
	  	  		  break;
	  	  	  case 6:
	  	  		  itoa(_info_storage.r_file_seek, value_buff, 10);
	  	  		  break;
	  	  	  default:
	  	  		  break;
		  }
		  strcat(lfs_read, "\""); strcat(lfs_read, value_buff);  strcat(lfs_read, "\"");
		  if(indx < INFO_KEY_SIZE-1)	 strcat(lfs_read, ",");
	}
	strcat(lfs_read, (char*)"}");
	printf("set_store_info :  new info string : %s\n", lfs_read);

	if( lfs_file_open(&littlefs, &file, "info.txt", LFS_O_RDWR ) < 0 )
	{
		  printf("set_store_info: fail to open info.txt from save_info\n");	while(1);
	}
	else
	{
		  if(lfs_file_write(&littlefs, &file, lfs_read, sizeof(vts_info)) < strlen(lfs_read))
		  {
			  printf("set_store_info :  fail to write new info.txt from main\n"); while(1);
		  }
		  else
		  {
			  success = 1 ;
		  }
		  lfs_file_close(&littlefs, &file);
	}
    return success ;
}

static storage_error_t save_data(uint8_t* buffer, int size)
{
	lfs_file_t file ;
	storage_error_t err = STORAGE_WRITE_FAIL ;

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

		printf("save_data :  path %s\n", data_path); //HAL_Delay(3000);

		if( lfs_file_open(&littlefs, &file, data_path, LFS_O_RDWR | LFS_O_CREAT ) < 0 )
		{
			 printf("save_data :fail to open data path :  %s\n", data_path);
			 err = STORAGE_FILE_OPEN_FAIL ;
		}
		else
		{
			  if(lfs_file_write(&littlefs, &file, buffer, size) < 0 )
			  {
				  printf("fail to write info.txt \n"); while(1);
			  }
			  else
			  {
				  printf("successfully written to %s \n ", data_path);  //HAL_Delay(3000);
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
				  printf("save_data : fail to update storage info \n"); while(1);
			  }
			  else
			  {
				  err = STORAGE_OK;
				  printf("save_data : info updated successfully ..  \n");
				  get_store_info();
			  }
		 }
	}
	return err ;
}
static storage_error_t read_data(uint8_t* buffer, int size)
{
	  lfs_file_t file ;
	  storage_error_t err = STORAGE_WRITE_FAIL ;

	  char data_path[68];
	  char folder_seek[3], file_seek[5];

	  int read_ok = 0 ;

	  if((_info_storage.w_folder_seek != _info_storage.r_folder_seek) && (_info_storage.r_file_seek !=_info_storage.w_file_seek ))
	  {
		  memset(folder_seek, 0, sizeof(folder_seek)); itoa(_info_storage.r_folder_seek, folder_seek, 10);
		  memset(file_seek,   0, sizeof(file_seek));   itoa(_info_storage.r_file_seek, file_seek, 10);
		  memset(data_path,   0, sizeof(data_path));

		  strcpy(data_path, folder_seek);	strcat(data_path, "/");	 strcat(data_path, file_seek); strcat(data_path, ".txt");

		  printf("read_data :  path %s\n", data_path);

		  if( lfs_file_open(&littlefs, &file, data_path, LFS_O_RDWR ) < 0 )
		  {
			  printf("read_data :fail to open data_path \n"); while(1);
			  err = STORAGE_FILE_OPEN_FAIL ;
		  }
		  else
		  {
			  if(lfs_file_read(&littlefs, &file, buffer, size) < 0 )
			  {
				  printf("read_data : fail to write info.txt \n"); while(1);
				  err = STORAGE_WRITE_FAIL;
			  }
			  else
			  {
				  printf("successfully read  ");
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
				  printf("read_data : fail to update storage info \n"); while(1);
			  }
			  else
			  {
				  err = STORAGE_OK;
			  }
		  }
	  }
	  else
		  err = STORATE_EMPTY  ;
	  return err ;
}



static int set_info(info_keys_t key, char * value)
{
	int i = 0, err = 0 , k_next = 0, v_next=0 , k_index =0 , v_index = 0 ;;

	char lfs_read[256];
	lfs_file_t file;

	char k[INFO_KEY_SIZE][18];
	char v[INFO_KEY_SIZE][48];
	int  kv_index = -1 ;
	uint8_t is_key_exists = 0 ;

	memset(lfs_read, 0, 256);
	if( lfs_file_open(&littlefs, &file, "info.txt", LFS_O_RDWR | LFS_O_CREAT) < 0 )
	{
		printf("set_info: fail to open info.txt from save_info\n");
		while(1);
	}
	else
	{
		  if( lfs_file_read(&littlefs, &file, lfs_read, 256) < 0)
		  {
			  printf("set_info : fail to read info.txt from save_info \n");
			  while(1);
		  }
		  else
		  {
			  printf("info.txt   :::  %s \n", lfs_read);
		      while(lfs_read[i])
		      {
	        	  if(lfs_read[i] == '\"')
	        	  {

	        	  }
	        	  else if(lfs_read[i] == '{' || lfs_read[i] == ',' || lfs_read[i] == '}')
		          {
			          if(lfs_read[i] == ',' || lfs_read[i] == '}')
			          {
			        	  printf("key:%s   value:%s \n", k[kv_index], v[kv_index]);
			              if(memcmp(k[kv_index], info_keys[key], k_index) == 0)
			              {
			            	  printf("key match for %s \n", k[kv_index]);
			            	  memset(v[kv_index], 0, sizeof(v[kv_index]));
			            	  strcpy(v[kv_index], value);
			            	  is_key_exists = 1 ;
			              }
			          }
			          if(lfs_read[i] == '}')
			          {
			        	  break;
			          }
			          kv_index++ ; k_next = 1 ; v_next = 0 ; k_index =0, v_index=0 ;

		              memset(k[kv_index], 0, sizeof(k[kv_index]));
		              memset(v[kv_index], 0, sizeof(v[kv_index]));
		          }
		          else if(lfs_read[i] == ':')
		          {
		        	  v_next = 1 ;	k_next = 0;
		          }
		          else
		          {
			          if(k_next)
			          {
			              k[kv_index][k_index++]= lfs_read[i] ;
			          }
			          else if(v_next == 1)
			          {
			        	  v[kv_index][v_index++] = lfs_read[i];
			          }
		          }
		          i++ ;
		      }

		      if(is_key_exists == 1)
		      {
		    	  memset(lfs_read, 0, 256);
		    	  lfs_read[0] = '{';
		    	  for(int indx = 0 ; indx <= kv_index; indx++)
		    	  {
		    		  strcat(lfs_read, "\""); strcat(lfs_read, (char*)k[indx]);  strcat(lfs_read, "\"");
		    		  strcat(lfs_read, (char*)":");
		    		  strcat(lfs_read, "\""); strcat(lfs_read, (char*)v[indx]);  strcat(lfs_read, "\"");
		    		  if(indx < kv_index)	 strcat(lfs_read, ",");
		    	  }
		    	  strcat(lfs_read, (char*)"}");

		    	  printf("new info string : %s\n", lfs_read);
		    	  lfs_file_rewind(&littlefs, &file);
  				  if(lfs_file_write(&littlefs, &file, lfs_read, sizeof(vts_info)) < 0 )
  				  {
  					  printf("fail to write new info.txt from main\n");	 err = 1 ;  while(1);
  				  }
		      }
		  }
		  lfs_file_close(&littlefs, &file);
	}
    return err ;
}

static int get_info(info_keys_t key, char * value)
{
	int i = 0 , res = 0 , got_key = 0, got_value=0 , key_index =0 , vaue_index = 0 ;;

	uint8_t lfs_read[256];
	lfs_file_t info_file;

	char key_buff[32], value_buff[48];


	memset(lfs_read, 0, 256);
	if( lfs_file_open(&littlefs, &info_file, "info.txt", LFS_O_RDWR | LFS_O_CREAT) < 0 )
	{
		printf("fail to open info.txt from save_info\n");
		while(1);
	}
	else
	{
		  if( lfs_file_read(&littlefs, &info_file, lfs_read, 256) < 0)
		  {
			  printf("fail to read info.txt from save_info \n");
			  while(1);
		  }
		  else
		  {
			  printf("info.txt  :::  %s \n", lfs_read);
		      while(lfs_read[i])
		      {
	        	  if(lfs_read[i] == '\"') {}
	        	  else  if(lfs_read[i] == '{' || lfs_read[i] == ',' || lfs_read[i] == '}')
		          {
			          if(lfs_read[i] == ',' || lfs_read[i] == '}')
			          {
			        	  printf("key:%s   value:%s\n", key_buff, value_buff);
			              if(memcmp(key_buff, info_keys[key], sizeof(info_keys[key])) == 0)
			              {
			            	  memcpy(value, value_buff, vaue_index);
			            	  res = 1 ;
			            	  break;
			              }
			          }
			          if(lfs_read[i] == '}')
			          {
			        	  break;
			          }
			          memset(key_buff, 0, sizeof(key_buff));
			          memset(value_buff, 0, sizeof(value_buff));
		              got_key = 1 ; got_value = 0 ; key_index =0 ; vaue_index=0  ;
		          }
		          else if(lfs_read[i] == ':')
		          {
		        	  got_value = 1 ;	got_key = 0;
		          }
		          else
		          {
			          if(got_key)
			          {
			              key_buff[key_index++]= lfs_read[i] ;
			          }
			          else if(got_value == 1)
			          {
			        	  value_buff[vaue_index++] = lfs_read[i];
			          }
		          }
		          i++ ;
		      }
		  }
		  lfs_file_close(&littlefs, &info_file);
	}

    return res ;
}













