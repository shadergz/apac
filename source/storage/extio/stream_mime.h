#ifndef APAC_STORAGE_EXTIO_STREAM_MIME_H
#define APAC_STORAGE_EXTIO_STREAM_MIME_H

#include <api.h>

typedef enum stream_mime_idx
{
  STREAM_MIME_IDX_PLAIN_TEXT,
  STREAM_MIME_IDX_ANDROID_PACKAGE,
  STREAM_MIME_IDX_APPLE_IOS_PACKAGE,

  STREAM_MIME_IDX_APAC_CACHE,

  STREAM_MIME_IDX_UNKNOWN

} stream_mime_idx_e;

extern const stream_mime_t g_mime_list[];

const stream_mime_t *mime_fromfile (storage_fio_t *file);

const char **mime_tostrlist (const stream_mime_t *mime);

#endif
