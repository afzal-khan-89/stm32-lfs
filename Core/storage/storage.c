#include "storage.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "spif.h"
#include "lfs.h"


static int set_info(info_keys_t , char * );
static int get_info(info_keys_t , char * );

lfs_t littlefs;

SPIF_HandleTypeDef spif_handle = {
		  .PageCnt = 256 ,
		  .SectorCnt = 512 ,
		  .BlockCnt = 32 ,
};

const uint8_t vts_info[]="{\"version_info\":\"0.0.00\",\"folder-max\":\"32\",\"folder-seek\":\"6\",\"file-max\":\"32\",\"file-seek\":\"45\"}" ;
const char info_keys[5][16] = {"version_info","folder-seek","folder-max", "file-seek", "file-max"};


int littlefs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
	printf("LittleFS Read b = 0x%04lx o = 0x%04lx s = 0x%04lx", block, off, size);
	//if (w25qxx_read(w25qxx_handle, block * w25qxx_handle->sector_size + off, buffer, size) != W25QXX_Ok) return -1;
	if(SPIF_ReadAddress(&spif_handle, 4096*block+off, buffer, size) !=  true) return -1;
	return 0;
}

int littlefs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
	printf("LittleFS Prog b = 0x%04lx o = 0x%04lx s = 0x%04lx", block, off, size);
	//if (w25qxx_write(w25qxx_handle, block * w25qxx_handle->sector_size + off, (void *)buffer, size) != W25QXX_Ok) return -1;
	SPIF_WriteAddress(&spif_handle, 4096*block+off, buffer, size);
	return 0;
}

int littlefs_erase(const struct lfs_config *c, lfs_block_t block) {
	printf("LittleFS Erase b = 0x%04lx", block);
	//if (w25qxx_erase(w25qxx_handle, block * w25qxx_handle->sector_size, w25qxx_handle->sector_size) != W25QXX_Ok) return -1;
	SPIF_EraseSector(&spif_handle, 4096*block);
	return 0;
}

int littlefs_sync(const struct lfs_config *c) {
	printf("LittleFS Sync");
	return 0;
}


struct lfs_config littlefs_config = {
    .read  = littlefs_read,
    .prog  = littlefs_prog,
    .erase = littlefs_erase,
    .sync  = littlefs_sync,

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
	err = lfs_mount(&littlefs, &littlefs_config);
	if(err < 0)
	{
		printf("lsf mount fail formatting ...  \n");
		HAL_Delay(5000);
        lfs_format(&littlefs, &littlefs_config);
        err = lfs_mount(&littlefs, &littlefs_config);
	}
	if(err)
		printf("lsf mount fail \n");
	else
		printf("------lfs mount ok ------------\n");

    return err;

}

int storage_init(Storage * _storage)
{
	int err = 0 ;
	lfs_file_t file;
	if(SPIF_Init(&spif_handle, _storage->spi, _storage->gpio, _storage->pin) == true )
	{
		  if(mount() < 0)
		  {
			  printf("w25qxx init fail .. \n");
			  err = -1 ;
		  }
		  else
		  {
  			  if( lfs_file_open(&littlefs, &file, "info.txt", LFS_O_RDWR | LFS_O_CREAT) < 0 )
  			  {
  				 printf("fail to open info.txt \n");
  				 err = -2 ;
  				 while(1);
  			  }
  			  else
  			  {
  				  char buff[48];
  				  if(get_info(version_info , buff) != 1)
  				  {
  	  				  if(lfs_file_write(&littlefs, &file, vts_info, sizeof(vts_info)) < 0 )
  	  				  {
  	  						  printf("fail to write info.txt from main\n");
  	  						  err = -3 ;
  	  				  }
  				  }
  				  else
  				  {
  					  printf("version info %s\n", buff);
  				  }
  				  lfs_file_close(&littlefs, &file);
  			  }
		  }
	}
	if(err == 0)
	{
		_storage->kv_get = get_info ;
		_storage->kv_set = set_info ;
	}
	return err ;
}


static int set_info(info_keys_t key, char * value)
{
	int i = 0 , err = 0 , k_next = 0, v_next=0 , k_index =0 , v_index = 0 ;;

	char lfs_read[256];
	lfs_file_t info_file;

	char k[5][16];
	char v[5][48];
	int  kv_index = -1 ;
	uint8_t is_key_exists = 0 ;

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
			  printf("info.txt  >>  :::  %s \n", lfs_read);
			  HAL_Delay(5000);

		      while(lfs_read[i])
		      {
	        	  if(lfs_read[i] == '\"')
	        	  {

	        	  }
	        	  else if(lfs_read[i] == '{' || lfs_read[i] == ',' || lfs_read[i] == '}')
		          {
			          if(lfs_read[i] == ',' || lfs_read[i] == '}')
			          {
			        	  printf("key:%s   value:%s\n", k[kv_index], v[kv_index]);
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
			          k_next = 1 ; v_next = 0 ; k_index =0, v_index=0 ;
		              kv_index++ ;
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
		    		  strcat(lfs_read, (char*)&k[indx]);	  strcat(lfs_read, (char*)":");	strcat(lfs_read, (char*)&v[indx]);
		    		  if(indx < kv_index)	 strcat(lfs_read, ",");
		    	  }
		    	  strcat(lfs_read, (char*)"}");

		    	  printf("new info string : %s\n", lfs_read);
//
//  				  if(lfs_file_write(&littlefs, &info_file, lfs_read, sizeof(vts_info)) < 0 )
//  				  {
//  						  printf("fail to write new info.txt from main\n");
//  						  while(1);
//  				  }
		      }
		  }
		  lfs_file_close(&littlefs, &info_file);
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
			  printf("info.txt  >>  :::  %s \n", lfs_read);
			  HAL_Delay(5000);

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
			            	  memcpy(value, value_buff, sizeof(value_buff));
			            	  res = 1 ;
			            	  break;
			              }
			          }
			          if(lfs_read[i] == '}')
			          {
			        	  break;
			          }
			          memset(key_buff, 0, sizeof(key_buff));
		              got_key = 1 ; got_value = 0 ; key_index =0 ;
		          }
		          else if(lfs_read[i] == ':')
		          {
		        	  got_value = 1 ;	got_key = 0, vaue_index=0 ;
		        	  memset(value_buff, 0, sizeof(value_buff));
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
