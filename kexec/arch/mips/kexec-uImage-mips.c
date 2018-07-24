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
#include "../../fs2dt.h"
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

#define COMMAND_LINE_SIZE 512
static char cmdline_buf[COMMAND_LINE_SIZE] = "";
extern struct arch_options_t arch_options;

int binary_mips_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
{
	off_t dtb_length;
	char *dtb_buf;
	unsigned long long kernel_addr = 0;
	unsigned long pagesize = getpagesize();

	dbgprintf("binary: %p %lld %p\n", buf, len, info);
	add_segment(info, buf, len, kernel_addr, len);
	info->entry = 0;

	if (arch_options.command_line) {
		strlcpy(cmdline_buf, arch_options.command_line, COMMAND_LINE_SIZE);

		dbgprintf("command oine %s\n", cmdline_buf);
		create_flatten_tree(&dtb_buf, &dtb_length, cmdline_buf);

		add_buffer(info, dtb_buf, dtb_length, dtb_length, 0,
			   _ALIGN_UP(kernel_addr + len, pagesize),
			   0x0fffffff, 1);

	}

	return 0;
}
