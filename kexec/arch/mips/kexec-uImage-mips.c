/*
 * uImage support added by Marc Andre Tanner <mat@brain-dump.org>
 */
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <image.h>
#include <kexec-uImage.h>
#include "../../kexec.h"
#include "kexec-mips.h"
#ifdef HAVE_LIBLZMA
#include "../../kexec-lzma.h"
#endif

int uImage_mips_probe(const char *buf, off_t len)
{
	return uImage_probe_kernel(buf, len, IH_ARCH_MIPS);
}

int uImage_mips_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
{
        dbgprintf("uimage: %p %lld %p\n", buf, len, info);
        struct image_header *header = (struct image_header *) buf;
        buf += sizeof(struct image_header);
        len -= sizeof(struct image_header);

        switch(header->ih_comp) {
#ifdef HAVE_LIBLZMA
        case IH_COMP_LZMA:
                dbgprintf("detected uimage lzma compression\n");
                buf = lzma_decompress_mem(buf, len, &len);
                /* fallthrough */
#endif
        case IH_COMP_NONE:
                dbgprintf("decompressed uimage: %p %lld %p\n", buf, len, info);
                return binary_mips_load(argc, argv, buf, len, info);
        default:
                /* reasonably sure we shouldn't be here as the probe already failed */
                printf("unsupported uimage compression method (bug?): %d\n", header->ih_comp);
                return -1;
        }
}


int binary_mips_probe(const char *buf, off_t len)
{
        return 0;                     /* yes, of course 0 is "success" */
}

int binary_mips_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
{
        dbgprintf("binary: %p %lld %p\n", buf, len, info);
        add_segment(info, buf, len, 0, len);
        info->entry = 0;
        return 0;
}
