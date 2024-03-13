#include "storage.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "spif.h"
#include "lfs.h"


static int set_info(info_keys_t , char * );
static int get_info(info_keys_t , char * );
static storate_error_t save_data(uint8_t* buffer, int size);
static storate_error_t read_data(uint8_t* buffer, int size);


lfs_t littlefs;

SPIF_HandleTypeDef spif_handle = {
		  .PageCnt = 256 ,
		  .SectorCnt = 512 ,
		  .BlockCnt = 32 ,
};

const uint8_t vts_info[]="{\"version\":\"0.0.00\",\"folder-max-count\":\"12\",\"file-max-count\":\"2048\",\"w-folder-seek\":\"1\","
"\"w-file-seek\":\"0\",\"r-folder-seek\":\"1\",\"r-file-seek\":\"0\"}" ;

const char info_keys[7][18] = {"version", "folder-max-count", "file-max-count", "w-folder-seek", "w-file-seek", "r-folder-seek", "r-file-seek"};


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
	SPIF_EraseSector(&spif_handle, 4096*block);
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
    .block_size = 4096, // lets use sector size . which is 4kB
    .block_count = 512,	// lets try with sector count . each block has 16 sector . thus 32 block has 512 sector .
    .cache_size = 256,
    .lookahead_size = 8,
    .block_cycles = 100,
};




static int mount() {
	int err = 0 ;
//  lfs_format(&littlefs, &littlefs_config);
	err = lfs_mount(&littlefs, &littlefs_config);
	if(err < 0)
	{
        lfs_format(&littlefs, &littlefs_config);
        err = lfs_mount(&littlefs, &littlefs_config);
	}
    return err;
}
storate_error_t storate_reset()
{
	lfs_format(&littlefs, &littlefs_config);
	if(mount() < 0)
	{
		 printf("lfs mount fail \n"); while(1);
		 return  STORAGE_MOUNT_FAIL ;
	}
	return STORAGE_OK ;
}
storate_error_t storage_init(Storage * _storage)
{
	storate_error_t err = STORAGE_OK ;
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
  			  if( lfs_file_open(&littlefs, &file, "info.txt", LFS_O_RDWR | LFS_O_CREAT) < 0 )
  			  {
  				 printf("fail to open info.txt \n"); while(1);
  				 err = STORAGE_FILE_OPEN_FAIL ;
  			  }
  			  else
  			  {
  				  char buff[48];
  				  if(get_info(version_info , buff) != 1)
  				  {
  	  				  if(lfs_file_write(&littlefs, &file, vts_info, sizeof(vts_info)) >= 0 )
  	  				  {
  	  					  printf("version info %s\n", buff);
  	  					  for(int bcnt=1; bcnt<=12; bcnt++)
  	  					  {
  	  						  char b[3] ; memset(b, 0, 3);
  	  						  itoa(bcnt, b, 10);
    	  	  				  if(lfs_mkdir(&littlefs, b) < 0 )
    	  	  				  {
    	  	  					  printf("fail to create storage dir %s\n", b); while(1);
    	  	  					  err = STORAGE_WRITE_FAIL;
    	  	  				  }
    	  	  				  printf("create storage dir %s\n", b); HAL_Delay(500);
  	  					  }
  	  				  }
  	  				  else
  	  				  {
	  					  printf("fail to write info.txt \n"); while(1);
  	  					  err = STORAGE_WRITE_FAIL;
  	  				  }
  				  }
  				  lfs_file_close(&littlefs, &file);
  			  }
		  }
		  if(err == STORAGE_OK)
		  {
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


static int set_info(info_keys_t key, char * value)
{
	int i = 0, err = 0 , k_next = 0, v_next=0 , k_index =0 , v_index = 0 ;;

	char lfs_read[256];
	lfs_file_t file;

	char k[7][18];
	char v[7][48];
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
static storage_error_t get_all_info(Info_storate_t * storage_info)
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
}
static storage_error_t save_data(uint8_t* buffer, int size)
{
	lfs_file_t file ;
	storage_error_t err = STORAGE_OK ;

	char data_path[68];
	char folder_seek[3], folder_count[3], file_seek[5], file_count[5];


	if( lfs_file_open(&littlefs, &file, "info.txt", LFS_O_RDWR | LFS_O_CREAT) < 0 )
	{
		printf("save_data : fail to open info.txt \n"); while(1);
		err = STORAGE_FILE_OPEN_FAIL ;
	}
	else
	{
		memset(folder_seek, 0, sizeof(folder_seek));
		if(get_info(w_folder_seek , folder_seek) != 1)
		{
			printf("fail to get folder_seek\n"); while(1);
		}


		memset(folder_count, 0, sizeof(folder_count));
		if(get_info(folder_max_count , folder_count) != 1)
		{
			printf("fail to get folder_max\n"); while(1);
		}


		memset(file_seek, 0, sizeof(file_seek));
		if(get_info(w_file_seek , file_seek) != 1)
		{
			printf("fail to get file_seek\n"); while(1);
		}


		memset(file_count, 0, sizeof(file_count));
		if(get_info(file_max_count , file_count) != 1)
		{
			printf("fail to get file_max\n"); while(1);
		}
		printf("folder count %s  folder seek %s   file count %s   file seek %s    \n",folder_count, folder_seek, file_count, file_seek);
		lfs_file_close(&littlefs, &file);
	}

	memset(data_path, 0, sizeof(data_path));	strcpy(data_path, folder_seek);	strcat(data_path, "/");	strcat(data_path, file_seek);
	printf("writing to path %s\n", data_path); HAL_Delay(1000);

	if( lfs_file_open(&littlefs, &file, data_path, LFS_O_RDWR | LFS_O_CREAT ) < 0 )
	{
		printf("fail to open data_path \n"); while(1);
		err = STORAGE_FILE_OPEN_FAIL ;
	}
	else
	{
		  if(lfs_file_write(&littlefs, &file, buffer, size) >= 0 )
		  {
			  printf("successfully written ");
			  int xf = atoi(file_seek);
			  xf++ ;
			  memset(file_seek, 0, sizeof(file_seek));
			  itoa(xf, file_seek, 10);
			  int e = set_info(w_file_seek, file_seek) ;
			  if(e == 1)
			  {
				  printf("fail to reset w-file-seek\n");
			  }
		  }
		  else
		  {
			  printf("fail to write info.txt \n"); while(1);
			  err = STORAGE_WRITE_FAIL;
		  }
		  lfs_file_close(&littlefs, &file);
	}
	return err ;
}
static storate_error_t read_data(uint8_t* buffer, int size)
{
	lfs_file_t file ;
	storate_error_t err = STORAGE_OK ;

	char data_path[68];
	char folder_seek[3], folder_count[3], file_seek[5], file_count[5];


	if( lfs_file_open(&littlefs, &file, "info.txt", LFS_O_RDWR | LFS_O_CREAT) < 0 )
	{
		printf("read_data : fail to open info.txt \n"); while(1);
		err = STORAGE_FILE_OPEN_FAIL ;
	}
	else
	{
		memset(folder_seek, 0, sizeof(folder_seek));
		if(get_info(r_folder_seek , folder_seek) != 1)
		{
			printf("fail to get folder_seek\n"); while(1);
		}


		memset(folder_count, 0, sizeof(folder_count));
		if(get_info(folder_max_count , folder_count) != 1)
		{
			printf("fail to get folder_max\n"); while(1);
		}


		memset(file_seek, 0, sizeof(file_seek));
		if(get_info(r_file_seek , file_seek) != 1)
		{
			printf("fail to get file_seek\n"); while(1);
		}


		memset(file_count, 0, sizeof(file_count));
		if(get_info(file_max_count , file_count) != 1)
		{
			printf("fail to get file_max\n"); while(1);
		}
		printf("folder count %s  folder seek %s   file count %s   file seek %s    \n",folder_count, folder_seek, file_count, file_seek);
		lfs_file_close(&littlefs, &file);
	}

	memset(data_path, 0, sizeof(data_path));
	strcpy(data_path, folder_seek);
	strcat(data_path, "/");
	strcat(data_path, file_seek);
	printf("read from   %s\n", data_path); HAL_Delay(5000);

	if( lfs_file_open(&littlefs, &file, data_path, LFS_O_RDWR | LFS_O_CREAT ) < 0 )
	{
		printf("fail to open data_path \n"); while(1);
		err = STORAGE_FILE_OPEN_FAIL ;
	}
	else
	{
		  if(lfs_file_read(&littlefs, &file, buffer, size) >= 0 )
		  {
			  printf("successfully read  ");
			  int lfs_remove(lfs_t *lfs, const char *path);
			  if(lfs_remove(&littlefs, data_path))
			  {
				  printf("successfully removed %s ", data_path);
				  int xf = atoi(file_seek);
				  xf++ ;
				  memset(file_seek, 0, sizeof(file_seek));
				  itoa(xf, file_seek, 10);
				  int e = set_info(w_file_seek, file_seek) ;
				  if(e == 1)
				  {
					  printf("fail to reset w-file-seek\n");
				  }
			  }
			  else
			  {

			  }
		  }
		  else
		  {
			  printf("fail to write info.txt \n"); while(1);
			  err = STORAGE_WRITE_FAIL;
		  }
		  lfs_file_close(&littlefs, &file);
	}
	return err ;
}


static int read_info_file()
{




}















