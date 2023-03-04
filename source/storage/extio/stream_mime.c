#include <string.h>

#include <memctrlext.h>

#include <storage/fio.h>

#include <storage/extio/stream_mime.h>

enum magic_header_idx {
	MAGIC_HEADER_PKZIP
};

struct magic_header {
	u8 magic_size;
	const u8* magic;
};

static const struct magic_header s_magic_headers[] = {
	[MAGIC_HEADER_PKZIP] = {0x4, (const u8[]){0x50, 0x4b, 0x03, 0x04}},
	{}
};

const char** mime_tostrlist(const stream_mime_t* mime) {
	static const char* mime_str[][3] = {
		[MAGIC_HEADER_PKZIP] = {".apk", ".ipa"},
		{}
	};

	const char** mime_arr = NULL;

	const stream_mime_idx_e id = mime - g_mime_list;
	
	if (id > STREAM_MIME_IDX_APPLE_IOS_PACKAGE) 
		goto mime_ret;
	
	switch (id) {
	case STREAM_MIME_IDX_ANDROID_PACKAGE:
	case STREAM_MIME_IDX_APPLE_IOS_PACKAGE:
		mime_arr = mime_str[MAGIC_HEADER_PKZIP]; 
		break;
	case STREAM_MIME_IDX_PLAIN_TEXT: 
	case STREAM_MIME_IDX_UNKNOWN: 
		goto mime_ret;
	}

mime_ret:
	return mime_arr;
}

const stream_mime_t* mime_fromfile(storage_fio_t* file) {

	u32 points = 0;
	const char* file_pathname = file->file_path;
	if (file_pathname == NULL) return NULL;

	const char* file_path = strdup(file_pathname);
	const stream_mime_t* mime = &g_mime_list[STREAM_MIME_IDX_UNKNOWN];

	const char* ext = strrchr(file_path, '.');
	if (ext == NULL) 
		goto checks_magic;

#define IF_MIME_CHECK(ext, desired)\
	if ((strncasecmp(ext, desired, strlen(desired))) == 0)

	IF_MIME_CHECK(ext, ".ipa") points += 10;
	IF_MIME_CHECK(ext, ".apk") points += 20;

checks_magic: __attribute__((hot));
	u64 file_magic[2] = {};

	fio_read(file, file_magic, sizeof(file_magic));

	for (const struct magic_header* magic_idx = s_magic_headers; 
		magic_idx->magic_size > 0; magic_idx++) {
		if (memcmp(&file_magic, magic_idx,
				magic_idx->magic_size) != 0) continue;

		const i32 magic_id = magic_idx - s_magic_headers;
		switch (magic_id) {
		case MAGIC_HEADER_PKZIP: points *= 2;
		}
	}

	switch (points) {
	case 20: mime = &g_mime_list[STREAM_MIME_IDX_APPLE_IOS_PACKAGE]; break;
	case 40: mime = &g_mime_list[STREAM_MIME_IDX_ANDROID_PACKAGE];   break;
	default: mime = &g_mime_list[STREAM_MIME_IDX_PLAIN_TEXT];
	}

	fio_seekbuffer(file, 0, FIO_SEEK_SET);

	apfree((char*)file_path);

	return mime;
}

const stream_mime_t g_mime_list[] = {
	[STREAM_MIME_IDX_PLAIN_TEXT]        = {"text", "plain"},
	[STREAM_MIME_IDX_ANDROID_PACKAGE]   = {"application", "vnd.android.package-archive"},
	[STREAM_MIME_IDX_APPLE_IOS_PACKAGE] = {"application", "octet-stream"},
	[STREAM_MIME_IDX_UNKNOWN]           = {},
	
	{}
};

