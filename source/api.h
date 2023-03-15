#ifndef APAC_API_H
#define APAC_API_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/opencl.h>

typedef uint8_t u8;

typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct stream_mime
{
  // https://en.wikipedia.org/wiki/Media_type
  // mime-type = type "/" [tree "."] subtype ["+" suffix]* [";" parameter];
  const char *type;
  const char *tree;
  const char *subtype;
  const char *suffix;
  const char *parameter;

} stream_mime_t;

typedef struct vecdie
{
  void *vec_dynamic;
  u64 vec_capa;
  u64 vec_dsize;
  u64 vec_used;

  u64 vec_cursor;

} vecdie_t;

typedef struct doublydie
{
  u32 node_crc;

  void *node_data;

  struct doublydie *next;
  struct doublydie *prev;

  struct doublydie *cursor;
} doublydie_t;

typedef struct spinlocker
{
  /* Our dedicated atomic variable by the way, using explicitly
   * the atomic keyword! */
  atomic_flag locked;

  _Atomic u32 recursive_lcount;
  _Atomic pthread_t owner_thread;

} spinlocker_t;

typedef struct schedthread
{
  pthread_t thread_handler;
  const char *thread_name;
  const char *context_name;

  _Atomic bool executing;

  char *echo_message;
  u64 echo_size;

  pid_t native_tid;
  u8 core_owner;
} schedthread_t;

typedef struct schedgov
{
  vecdie_t *threads_info;

  pthread_attr_t *thread_attrs;
  sigset_t thread_dflt;

  _Atomic u8 threads_count;
  u8 cores;
  spinlocker_t mutex;

} schedgov_t;

typedef struct storage_dirio
{
  const char *dir_path;
  const char *dir_relative;

  const char *dir_name;

  i32 dir_fd;

  u64 buf_pos, buf_end, position;

#define DIR_INFO_SZ 1 << 6

  u8 dir_info[DIR_INFO_SZ];
} storage_dirio_t;

typedef struct storage_fio
{
  const char *file_path;
  const char *file_name;
  const char *real_filename;
  const char *file_rel;

  const stream_mime_t *mime_identifier;
  i32 file_fd;
  i32 recerror;

  bool is_link, is_locked;

  // The size of a memory page `chunk/block` (4096 bytes)
#define FIO_DATA_CACHE_SZ 1 << 12

  u8 rw_cache[FIO_DATA_CACHE_SZ];
  u8 *cache_cursor;
  u64 cursor_offset;
  u64 cache_block;
  u64 cache_valid;

} storage_fio_t;

typedef enum storage_node_id
{
  STORAGE_NODE_ID_DIR = 1000,
  STORAGE_NODE_ID_FILE = 1111,
} storage_node_id_e;

typedef struct storage_tree
{
  storage_node_id_e node_id;
  u64 node_level;

  union
  {
    storage_dirio_t *node_dir;
    storage_fio_t *node_file;
  };

  doublydie_t *leafs;
  struct storage_tree *parent;
} storage_tree_t;

typedef struct user_options
{
  bool dsp_help;
  bool dsp_banner;
  bool enb_log_system;

  i32 echo_level;
  bool enb_colors;

} __attribute__ ((aligned (4))) user_options_t;

typedef struct echo_ctx
{
  i32 (*event_announce) (void *apac_ctx, i32 level, const char *message,
                         const char *thread_message);

  _Atomic u64 dispatch_count;
  _Atomic u64 cnt_dft;
#if defined(APAC_IS_UNDER_DEBUG)
#endif

} echo_ctx_t;

typedef struct config_user
{
  const char *default_input;
  const char *default_output;

  const char *exec_script;
  const char *structure_model;

} config_user_t;

typedef void *external_module_t;
typedef void *external_func_t;

typedef external_module_t opencl_driver_t;

typedef cl_int (*OCL_GETDEVICEIDS_FUNC) (cl_platform_id platform,
                                         cl_device_type device_type,
                                         cl_uint num_entries,
                                         cl_device_id *devices,
                                         cl_uint *num_devices);
typedef cl_int (*OCL_GETDEVICEINFO_FUNC) (cl_device_id device,
                                          cl_device_info param_name,
                                          size_t param_value_size,
                                          void *param_value,
                                          size_t *param_value_size_ret);

typedef struct opencl_int
{
  opencl_driver_t ocl_driver;

  OCL_GETDEVICEIDS_FUNC clGetDeviceIDs;
  OCL_GETDEVICEINFO_FUNC clGetDeviceInfo;

} opencl_int_t;

typedef struct backend_ctx
{
  opencl_int_t *ocl_interface;
  storage_fio_t *ocl_shared;

} backend_ctx_t;

typedef struct lockerproc
{
  u16 locker_pid;
  u16 saved_st;
  u16 time;

} lockerproc_t;

typedef struct pkg_settings
{
  struct
  {
    const char *execute_prompt;
  };
} pkg_settings_t;

typedef enum pkg_type
{
  PKG_TYPE_UNDEFINED_PKZIP,
  PKG_TYPE_ANDROID_APK,
  PKG_TYPE_ANDROID_XAPK,
  PKG_TYPE_ANDROID_OBB,
  PKG_TYPE_IOS_API

} pkg_type_e;

typedef struct pkg_container
{
  pkg_type_e pkgtype;

  u32 pkg_uuid;

  storage_fio_t *pkg_file;

  u8 *pkg_umap;

  const pkg_settings_t *todo;
} pkg_container_t;

typedef struct pkg_manager
{
  spinlocker_t *mgr_remutex;

  u64 pkg_done;
  u64 pkg_missing;

  doublydie_t *pkg_dynamiclist;

} pkg_manager_t;

typedef struct rule_selector
{
  const char *rule_pkglist;
  pkg_settings_t *rule_settings;

  const char *rule_outdirs;
  const char *structure_model;
  const char *run_script;

} rule_selector_t;

typedef enum cache_type
{
  CACHE_TYPE_STRING,
  CACHE_TYPE_RAW_DATA
} cache_type_e;

typedef enum cache_special_id
{
  CACHE_SPECIAL_ID_OCL_BINARIES
} cache_special_id_e;

typedef struct cache_entry
{
  cache_type_e entry_type;
  cache_special_id_e entry_special;

#define ENTRY_DESC_SZ 0xb
  char entry_desc[ENTRY_DESC_SZ];

  u64 stream_size;
  u8 stream[];
} cache_entry_t;

typedef struct cache_header_t
{
  u16 cache_magic;
  u8 cache_version;

  time_t creation_date;
  time_t last_access;
  time_t last_moder;

#define SHA256_BLOCK_SIZE 512 / 8

  u8 sha2_entrsum[SHA256_BLOCK_SIZE];

  u64 entries_count;

  u16 string_entcount;
  u16 ocl_binary_entcount;

  cache_entry_t entries[];
} cache_header_t;

typedef struct fast_cache
{
  cache_header_t *header;
  doublydie_t *entries;

#define CACHE_AUX_BSZ (sizeof (typeof (cache_entry_t)) + 256)

  u8 *cache_ptr;
  u8 aux_cache[CACHE_AUX_BSZ];

} fast_cache_t;

typedef struct session_ctx
{
  user_options_t *user_options;
  config_user_t *user_config;

  doublydie_t *selectors;
} session_ctx_t;

typedef struct apac_ctx
{
  session_ctx_t *user_session;

  echo_ctx_t *echo_system;

  schedgov_t *governor;

  backend_ctx_t *core_backend;

  storage_tree_t *root;

  lockerproc_t *locker;

  fast_cache_t *fastc;
} apac_ctx_t;

#endif
