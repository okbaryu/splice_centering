// update.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include <ctype.h>
#include <unistd.h>
#include "script.h"

#define  MAX_PATH             (260)
#define  PART_DATA_OFFSET     (256 * 1024)
//#define  PART_DATA_OFFSET     (512 * 1024)
#define  BUFFER_SIZE_MAX      (1024 * 1024)


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int IsFullName(const char *FilePath)
{
	if (isalpha(FilePath[0]) && ':' == FilePath[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void GetFullPath(char *dName, const char *sName)
{
    char Buffer[MAX_PATH];

	if(IsFullName(sName))
	{
		strcpy(dName, sName);
		return ;
	}
	/* Get the current working directory: */
    if(getcwd(Buffer, MAX_PATH ) == NULL)
    {
    	perror( "getcwd error" );
    	return ;
   	}
    sprintf(dName, "%s/%s", Buffer, sName);
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("merge_package target_file_name [boot0] [uboot] [mbr] [sys_partition]\n\n");
}

int main(int argc, char* argv[])
{
	char   input_full_path[MAX_PATH];
	char   output_full_path[MAX_PATH];
	char   script_full_path[MAX_PATH];
	FILE   *input_file = NULL, *script_file = NULL;
	FILE   *output_file = NULL;
	int    file_fill_index = 0;
	int    uboot_filled = 0;
	int    mbr_filled   = 0;
	int    ret = -1;
	int    file_len;
	int    i;
	char   *buffer = NULL, *script_buff = NULL;
	int    total_part_ofs = 0;
	int    partition_index = 0;

	if(argc <= 2)
	{
		Usage();

		return __LINE__;
	}

	GetFullPath(output_full_path, argv[1]);
	output_file = fopen(output_full_path, "wb");
	if(!output_file)
	{
		printf("fail to open file %s\n", output_full_path);

		return -1;
	}
	buffer = (char *)malloc(1024 * 1024);
	if(!buffer)
	{
		printf("fail to malloc memory\n");

		goto __merge_package_err;
	}

	for(i=2;i<argc;i++)
	{
		GetFullPath(input_full_path,  argv[i]);

		input_file = fopen(input_full_path, "rb");
		if(input_file == NULL)
		{
			printf("fail to open file %s\n", input_full_path);

			goto __merge_package_err;
		}
		fseek(input_file, 0, SEEK_END);
		file_len = ftell(input_file);
		fseek(input_file, 0, SEEK_SET);
		if(!file_len)
		{
			printf("the input file %s size is 0\n", input_full_path);

			goto __merge_package_err;
		}
		if(!strncmp(argv[i], "boot0", strlen("boot0")))
		{
			printf("input file : %s\n", argv[i]);
			printf("file len=%d\n",file_len);
			fread(buffer, file_len, 1, input_file);
			fseek(output_file, 0, SEEK_SET);
			fwrite(buffer, file_len, 1, output_file);
		}
		else if(!strcmp(argv[i], "sunxi_mbr.fex"))
		{
			printf("input file : %s\n", argv[i]);
			printf("file len=%d\n",file_len);
			fread(buffer, file_len, 1, input_file);
			fseek(output_file, PART_DATA_OFFSET - 16 * 1024, SEEK_SET);
			fwrite(buffer, 16 * 1024, 1, output_file);
		}
		else if(!strncmp(argv[i], "u-boot", strlen("u-boot")))
		{
			printf("input file : %s\n", argv[i]);
			fread(buffer, file_len, 1, input_file);
			fseek(output_file, 24 * 1024, SEEK_SET);
			fwrite(buffer, file_len, 1, output_file);
		}
		else if(!strcmp(argv[i], "sys_partition.bin"))
		{
			int part_size;

			script_buff = (char *)malloc(file_len);
			if(!script_buff)
			{
				printf("fail to malloc memory for merge script\n");

				goto __merge_package_err;
			}

			fread(script_buff, file_len, 1, input_file);
			script_parser_init(script_buff);
			while(1)
			{
				int part_sz;

				partition_index = script_parser_fetch_partition();
				if(partition_index < 0)
				{
					printf("invalid sys partition script\n");

					goto __merge_package_err;
				}
				if(partition_index == 0)
				{
					printf("sys parition end\n");

					break;
				}
				memset(script_full_path, 0, MAX_PATH);
				if(!script_parser_fetch_mainkey_sub("downloadfile", partition_index, (int *)script_full_path))
				{
					int tmp_file_len;

					memset(input_full_path, 0, MAX_PATH);
					printf("input file : %s\n", script_full_path);
					GetFullPath(input_full_path, script_full_path);
					script_file = fopen(input_full_path, "rb");
					if(!script_file)
					{
						printf("fail to open script file %s\n", input_full_path);

						goto __merge_package_err;
					}
					fseek(script_file, 0, SEEK_END);
					file_len = ftell(script_file);
					fseek(script_file, 0, SEEK_SET);
					if(!file_len)
					{
						printf("the input file %s size is 0\n", input_full_path);

						goto __merge_package_err;
					}
					tmp_file_len = file_len;
					fseek(output_file, PART_DATA_OFFSET + total_part_ofs, SEEK_SET);
					while(tmp_file_len >= BUFFER_SIZE_MAX)
					{
						fread(buffer, BUFFER_SIZE_MAX, 1, script_file);
						fwrite(buffer, BUFFER_SIZE_MAX, 1, output_file);
						tmp_file_len -= BUFFER_SIZE_MAX;
					}
					if(tmp_file_len)
					{
						fread(buffer, tmp_file_len, 1, script_file);
						fwrite(buffer, tmp_file_len, 1, output_file);
					}
					fclose(script_file);
					script_file = NULL;
				}
				if(!script_parser_fetch_mainkey_sub("size", partition_index, &part_sz))
				{
					total_part_ofs += part_sz * 512;
				}
				else
				{
					printf("invalid sys partition script, for the part size is not configed\n");
				}
			}
		}
		else
		{
			printf("invalid file\n");
		}

		fclose(input_file);
		input_file = NULL;
	}
	ret = 0;

__merge_package_err:
	//close output file
	if(output_file)
	{
		fclose(output_file);
		input_file = NULL;
	}
	//close input file
	if(input_file)
	{
		fclose(input_file);
		input_file = NULL;
	}
	//free buffer
	if(buffer)
	{
		free(buffer);
	}
	buffer = NULL;

	if(script_buff)
	{
		free(script_buff);
	}
	script_buff = NULL;

	return ret;
}


