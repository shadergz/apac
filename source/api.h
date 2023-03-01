#ifndef APAC_API_H
#define APAC_API_H

#include <stdbool.h>
#include <stdint.h>

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/opencl.h>

typedef uint8_t u8;

typedef int32_t i32;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct vecdie {
	void* vec_dynamic;
	u64 vec_capa;
	u64 vec_dsize;
	u64 vec_used;

	u64 vec_cursor;

} vecdie_t;

typedef struct doublydie {
	u32 node_crc;

	void* node_data;

	struct doublydie* next;
	struct doublydie* prev;

	struct doublydie* cursor;
} doublydie_t;

typedef struct spinlocker {
	/* Our dedicated atomic variable by the way, using explicitly 
	 * the atomic keyword! */
	_Atomic u32 flag_spawn;
} spinlocker_t;

typedef struct schedthread {
	u32 thread_id;
	const char* thread_name;

	const char* echo_message;
	u64 echo_size;

} schedthread_t;

typedef struct schedgov {
	vecdie_t* thread_info_vec;
} schedgov_t;

typedef struct storage_dirio {
	const char* dir_path;
	const char* dir_relative;

	const char* dir_name;

	i32 dir_fd;

	u64 buf_pos, buf_end, position;

	#define DIR_INFO_SZ 1 << 6

	u8 dir_info[DIR_INFO_SZ];
} storage_dirio_t;

typedef struct storage_fio {
	const char* file_path;
	const char* file_name;
	const char* real_filename;
	const char* file_rel;

	i32 file_fd;
	i32 recerror;

	bool is_link, is_locked;

	// The size of a memory page `chunk/block` (4096 bytes)
	#define FIO_DATA_CACHE_SZ 1 << 12

	u8 rw_cache[FIO_DATA_CACHE_SZ];

} storage_fio_t;

typedef enum storage_node_id {
	STORAGE_NODE_ID_DIR =  1000,
	STORAGE_NODE_ID_FILE = 1111,
} storage_node_id_e;

typedef struct storage_tree {
	storage_node_id_e node_id;
	u64 node_level;

	union {
		storage_dirio_t* node_dir;
		storage_fio_t* node_file;
	};

	doublydie_t* leafs;
	struct storage_tree* parent;
} storage_tree_t;

typedef struct user_options {
	bool dsp_help;
	bool dsp_banner;
	bool enb_log_system;

	i32 echo_level;
	bool enb_colors;

} __attribute__((aligned(4))) user_options_t;

typedef struct echo_ctx {
	i32 (*event_announce)(void* apac_ctx, i32 level, const char* message, 
		const char* thread_message);
	
	_Atomic u64 dispatch_count;
	_Atomic u64 cnt_dft;
	#if defined(APAC_IS_UNDER_DEBUG)
	#endif
	
} echo_ctx_t;

typedef struct session_ctx {
	user_options_t* user_options;

} session_ctx_t;

typedef void* external_module_t;
typedef void* external_func_t;

typedef external_module_t opencl_driver_t;

typedef cl_int (*OCL_GETDEVICEIDS_FUNC)(
	cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, 
	cl_device_id* devices, cl_uint* num_devices);

typedef struct opencl_int {
	opencl_driver_t ocl_driver;

	OCL_GETDEVICEIDS_FUNC clGetDeviceIDs;

} opencl_int_t;

typedef struct backend_ctx {
	opencl_int_t* ocl_interface;
	storage_fio_t* ocl_shared;

} backend_ctx_t;

typedef struct lockerproc {
	u16 locker_pid;
	u16 saved_st;
	u16 time;

} lockerproc_t;

typedef struct apac_ctx {
	session_ctx_t* user_session;
	
	echo_ctx_t* echo_system;
	
	schedgov_t* governor;
	
	backend_ctx_t* core_backend;

	storage_tree_t* root;

	lockerproc_t* locker;
} apac_ctx_t;

#endif

