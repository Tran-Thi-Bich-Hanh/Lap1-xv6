#ifndef _SYSINFO_H_
#define _SYSINFO_H_

#include "types.h"

struct sysinfo {
  uint64 freemem;   // Số byte bộ nhớ trống
  uint64 nproc;     // Số tiến trình có trạng thái khác UNUSED
};

#endif