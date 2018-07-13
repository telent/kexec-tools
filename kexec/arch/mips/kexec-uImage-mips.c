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

int uImage_mips_probe(const char *buf, off_t len)
{
	return uImage_probe_kernel(buf, len, IH_ARCH_MIPS);
}

int uImage_mips_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
{
	return binary_mips_load(argc, argv, buf + sizeof(struct image_header),
	                        len - sizeof(struct image_header), info);
}


int binary_mips_probe(const char *buf, off_t len)
{
  return 0;                     /* yes, of course 0 is "success" */
}

int binary_mips_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
{
  printf("bml %x %ld %x\n", buf, len, info);
  add_segment(info, buf, len, 0, len);
  info->entry = 0;
  return 0;
}
