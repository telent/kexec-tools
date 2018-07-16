#include <unistd.h>
#include <sys/types.h>

#include "kexec-lzma.h"
#include "config.h"
#include "kexec.h"

#ifdef HAVE_LIBLZMA
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <ctype.h>
#include <lzma.h>

static lzma_ret lzmem_prepare(lzma_stream *lstream, const char *compressed, ssize_t csize);
static ssize_t lzmem_read(lzma_stream *lstream, void *buf, size_t len);

static lzma_ret lzmem_prepare(lzma_stream *lstream, const char *compressed, ssize_t compressed_size)
{
	lzma_ret ret;

        ret = lzma_auto_decoder(lstream,
                                UINT64_C(64) * 1024 * 1024, 0);
	if (ret == LZMA_OK) {
                lstream->next_in = compressed;
                lstream->avail_in = compressed_size;
        } else {
                dbgprintf("not ok: %d\n", ret);
	}

        return ret;
}


static ssize_t lzmem_read(lzma_stream *lstream, void *buf, size_t len)
{
	lzma_ret ret;
	int eof = 0;

	if (lstream->next_in == NULL)
                /* previous call indicated the decoder had finished */
		return 0;

	lstream->next_out = buf;
	lstream->avail_out = len;

	for (;;) {
		if (!lstream->avail_in) {
                        /* ran out of input, but we haven't seen LZMA_STREAM_END?
                         * probably an error, but try once more round the loop in case
                         */
                        eof = 1;
                }

		ret = lzma_code(lstream, LZMA_RUN);
		if (ret == LZMA_STREAM_END) {
                        /* decoder got to the end, null out next_in so
                         * we don't do anything next time we're called
                         */
                        lstream->next_in = NULL;
			return len - lstream->avail_out;
		}

		if (ret != LZMA_OK)
			return -1;

                /* have we filled the output buffer? return with what we've got */
		if (lstream->avail_out == 0)
			return len;

		if (eof)
			return -1;
	}
}

const char *lzma_decompress_mem(const char *compressed, off_t compressed_size, off_t *r_size)
{
	char *buf;
	off_t size, allocated;
	off_t result;

	dbgprintf("Try LZMA in-memory decompression %p %lld\n", compressed, compressed_size);

	*r_size = 0;

        lzma_stream lstream = LZMA_STREAM_INIT;
	if(lzmem_prepare(&lstream, compressed, compressed_size) != LZMA_OK)
                return 0;

	size = 0;
	allocated = 65536;
	buf = xmalloc(allocated);
	do {
		if (size == allocated) {
			allocated <<= 1;
			buf = xrealloc(buf, allocated);
		}
                dbgprintf("buf, size=%p, %lld\n", buf, size);
		result = lzmem_read(&lstream, buf + size, allocated - size);
                dbgprintf("lzmem_read = %lld\n", result);
		if (result < 0) {
			dbgprintf("%s: unlzma %ld bytes failed\n",
                                  __func__, (long int) (allocated - size) + 0UL);
			break;
		}
		size += result;
	} while (result > 0);

	lzma_end(&lstream);
	if (result < 0)
		goto fail;

	*r_size =  size;
	return buf;
fail:
	free(buf);
	return NULL;
}
#else
const char *lzma_decompress_mem(const char *UNUSED(filename), off_t *UNUSED(r_size))
{
	return NULL;
}
#endif /* HAVE_LIBLZMA */
