#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <Windows.h>

#define TRUE 1
#define FALSE 0

FILETIME ft_start;
FILETIME ft_end;

#define TIMER_START() GetSystemTimePreciseAsFileTime(&ft_start)
#define TIMER_END() GetSystemTimePreciseAsFileTime(&ft_end)
#define TIMER_ELAPSE_SEC() ((double)((((ULONGLONG)ft_end.dwHighDateTime << 32) | (ULONGLONG)ft_end.dwLowDateTime ) - (((ULONGLONG)ft_start.dwHighDateTime << 32) | (ULONGLONG)ft_start.dwLowDateTime )) / 10000000.0)

double sec;
TCHAR log_filepath[1024];

#define BUF_SIZE 1024*1024 // 1mb
#define STR_BUF_SIZE 1024

void rw_writefile_benchmark(TCHAR * filepath, int block_size, int count, BOOL isFsyncOn) {
	TCHAR filepath_r[1024];
	HANDLE file_handle;
	BOOL error_flag;
	DWORD dwRetBytes = 0;
	int str_len;
	char * buf;
	char str_buf[STR_BUF_SIZE];

	double* elapse_time_ary; // To save sec dif and nsec dif

	buf = (char*)malloc(sizeof(char)*BUF_SIZE);
	elapse_time_ary = (double*)malloc(sizeof(double)*count);

	for (int i = 0; i<BUF_SIZE; i++) {
		buf[i] = i % 2;
	}

	for (int i = 0; i < count; i++) {
		swprintf(filepath_r, STR_BUF_SIZE, TEXT("%s_%d"), filepath, i);

		// Create file
		file_handle = CreateFile(filepath_r,	// filename
			GENERIC_READ | GENERIC_WRITE,					// write only
			0,								// dont share
			NULL,							// default security
			CREATE_ALWAYS,					// always create new file
			FILE_ATTRIBUTE_NORMAL,			// normal file
			NULL);							// no attr.

		if (file_handle == INVALID_HANDLE_VALUE) {
			printf("Failed to create file\n");
			exit(1);
		}

		TIMER_START();

		error_flag = WriteFile(file_handle,	// file handle to write
			buf,							// write buffer
			block_size,						// how many bytes
			&dwRetBytes,				// number of bytes
			NULL);							// no overlapped structrue

		if (error_flag == FALSE || dwRetBytes != block_size) {
			printf("Failed to write a block\n");
			exit(1);
		}

		if (isFsyncOn == TRUE) {
			error_flag = FlushFileBuffers(file_handle);
			if (error_flag == FALSE) {
				printf("Failed to flush file\n");
				exit(1);
			}
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		elapse_time_ary[i] = sec;

		CloseHandle(file_handle);
	}

	// Create file
	file_handle = CreateFile(log_filepath,	// filename
		GENERIC_READ | GENERIC_WRITE,					// write only
		0,								// dont share
		NULL,							// default security
		CREATE_ALWAYS,					// always create new file
		FILE_ATTRIBUTE_NORMAL,			// normal file
		NULL);							// no attr.

	if (file_handle == INVALID_HANDLE_VALUE) {
		printf("Failed to create file\n");
		exit(1);
	}

	str_len = sprintf(str_buf, "%d bytes write() time\n", block_size);
	error_flag = WriteFile(file_handle,	// file handle to write
		str_buf,							// write buffer
		str_len,						// how many bytes
		&dwRetBytes,				// number of bytes
		NULL);							// no overlapped structrue

	if (error_flag == FALSE || dwRetBytes != str_len) {
		printf("Failed to write a block\n");
		exit(1);
	}

	for (int i = 0; i<count; i++) {
		str_len = sprintf(str_buf, "%f\n", elapse_time_ary[i]);
		error_flag = WriteFile(file_handle,	// file handle to write
			str_buf,							// write buffer
			str_len,						// how many bytes
			&dwRetBytes,				// number of bytes
			NULL);							// no overlapped structrue

		if (error_flag == FALSE || dwRetBytes != str_len) {
			printf("Failed to write a block\n");
			exit(1);
		}
	}

	CloseHandle(file_handle);

	free(buf);
	free(elapse_time_ary);
}
void rw_readfile_benchmark(TCHAR * filepath, int block_size, int count) {
	TCHAR filepath_r[1024];
	HANDLE file_handle;
	BOOL error_flag;
	DWORD dwRetBytes = 0;
	int str_len;
	char * buf;
	char str_buf[STR_BUF_SIZE];

	double* first_elapse_time_ary; // To save sec dif and nsec dif
	double* second_elapse_time_ary; // To save sec dif and nsec dif

	buf = (char*)malloc(sizeof(char)*BUF_SIZE);

	first_elapse_time_ary = (double*)malloc(sizeof(double)*count);
	second_elapse_time_ary = (double*)malloc(sizeof(double)*count);

	for (int i = 0; i<BUF_SIZE; i++) {
		buf[i] = i % 2;
	}

	for (int i = 0; i<count; i++) {
		swprintf(filepath_r, STR_BUF_SIZE, TEXT("%s_%d"), filepath, i);

		// Create file
		file_handle = CreateFile(filepath_r,	// filename
			GENERIC_READ,					// write only
			0,								// dont share
			NULL,							// default security
			OPEN_EXISTING,					// always create new file
			FILE_ATTRIBUTE_NORMAL,			// normal file
			NULL);							// no attr.

		if (file_handle == INVALID_HANDLE_VALUE) {
			printf("Failed to create file\n");
			exit(1);
		}

		// first read
		TIMER_START();

		// Read data
		error_flag = ReadFile(file_handle,	// file handle to write
			buf,							// read buffer
			block_size,						// how many bytes
			&dwRetBytes,				// number of bytes
			NULL);							// no overlapped structrue

		if (error_flag == FALSE || dwRetBytes != block_size) {
			printf("Failed to read a block\n");
			exit(1);
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		first_elapse_time_ary[i] = sec;

		//rewind file pointer
		dwRetBytes = SetFilePointer(file_handle,
			0,
			NULL,
			FILE_BEGIN);
		if (dwRetBytes == INVALID_SET_FILE_POINTER) {
			printf("Failed to set file pointer\n");
			exit(1);
		}

		// second read
		TIMER_START();

		// Read data
		error_flag = ReadFile(file_handle,	// file handle to write
			buf,							// read buffer
			block_size,						// how many bytes
			&dwRetBytes,				// number of bytes
			NULL);							// no overlapped structrue

		if (error_flag == FALSE || dwRetBytes != block_size) {
			printf("Failed to read a block\n");
			exit(1);
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		second_elapse_time_ary[i] = sec;

		// Close file
		CloseHandle(file_handle);
	}

	// Create file
	file_handle = CreateFile(log_filepath,	// filename
		GENERIC_READ | GENERIC_WRITE,					// write only
		0,								// dont share
		NULL,							// default security
		CREATE_ALWAYS,					// always create new file
		FILE_ATTRIBUTE_NORMAL,			// normal file
		NULL);							// no attr.

	if (file_handle == INVALID_HANDLE_VALUE) {
		printf("Failed to create file\n");
		exit(1);
	}

	str_len = sprintf(str_buf, "%d bytes read() time (not cached)\n", block_size);
	error_flag = WriteFile(file_handle,	// file handle to write
		str_buf,							// write buffer
		str_len,						// how many bytes
		&dwRetBytes,				// number of bytes
		NULL);							// no overlapped structrue

	if (error_flag == FALSE || dwRetBytes != str_len) {
		printf("Failed to write a block\n");
		exit(1);
	}

	for (int i = 0; i<count; i++) {
		str_len = sprintf(str_buf, "%f\n", first_elapse_time_ary[i]);
		error_flag = WriteFile(file_handle,	// file handle to write
			str_buf,							// write buffer
			str_len,						// how many bytes
			&dwRetBytes,				// number of bytes
			NULL);							// no overlapped structrue

		if (error_flag == FALSE || dwRetBytes != str_len) {
			printf("Failed to write a block\n");
			exit(1);
		}
	}

	str_len = sprintf(str_buf, "%d bytes write() time (cached)\n", block_size);
	error_flag = WriteFile(file_handle,	// file handle to write
		str_buf,							// write buffer
		str_len,						// how many bytes
		&dwRetBytes,				// number of bytes
		NULL);							// no overlapped structrue

	if (error_flag == FALSE || dwRetBytes != str_len) {
		printf("Failed to write a block\n");
		exit(1);
	}

	for (int i = 0; i<count; i++) {
		str_len = sprintf(str_buf, "%f\n", second_elapse_time_ary[i]);
		error_flag = WriteFile(file_handle,	// file handle to write
			str_buf,							// write buffer
			str_len,						// how many bytes
			&dwRetBytes,				// number of bytes
			NULL);							// no overlapped structrue

		if (error_flag == FALSE || dwRetBytes != str_len) {
			printf("Failed to write a block\n");
			exit(1);
		}
	}

	CloseHandle(file_handle);

	free(buf);

	free(first_elapse_time_ary);
	free(second_elapse_time_ary);
}

void rw_remove(TCHAR * filepath, int count) {
	TCHAR filepath_r[1024];
	BOOL error_flag;

	for (int i = 0; i<count; i++) {
		swprintf(filepath_r, STR_BUF_SIZE, TEXT("%s_%d"), filepath, i);

		error_flag = DeleteFile(filepath_r);
		if (error_flag == FALSE) {
			printf("Failed to delete file\n");
			exit(1);
		}
	}
}

#define FILENAME TEXT("test")

void main(int argc, char ** argv) {
	int repeat;
	int block_size;
	BOOL isFsyncOn;

	if (argc != 6) {
		printf("Usage : ./bm <write|read> <block_size> <repeat_count> <log_filename> <on|off(fsync)>\n");
		exit(1);
	}

	block_size = atoi(argv[2]);
	repeat = atoi(argv[3]);
	MultiByteToWideChar(CP_UTF8, 0, argv[4], strlen(argv[4]), log_filepath, 1024);

	if (strcmp(argv[5], "on") == 0) {
		isFsyncOn = TRUE;
	}
	else if (strcmp(argv[5], "off") == 0){
		isFsyncOn = FALSE;
	}
	else {
		printf("Usage : ./bm <write|read> <block_size> <repeat_count> <log_filename> <on|off(fsync)>\n");
		exit(1);
	}

	if (strcmp(argv[1], "write") == 0) {
		rw_writefile_benchmark(FILENAME, block_size, repeat, isFsyncOn);
	}
	else if (strcmp(argv[1], "read") == 0) {
		rw_readfile_benchmark(FILENAME, block_size, repeat);
	}
	else if (strcmp(argv[1], "remove") == 0) {
		rw_remove(FILENAME, repeat);
	}
	else {
		printf("Usage : ./bm <write|read> <block_size> <repeat_count> <log_filename> <on|off(fsync)>\n");
		exit(1);
	}
}