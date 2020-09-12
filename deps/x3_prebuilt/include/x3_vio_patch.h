/***************************************************************************
 *   Copyright (C) 2019 by horizon.                                        *
 *   xudong.du@horizon.ai                                                  *
 *                                                                         *
 *   header file.                                                          *
 *                                                                         *
 ***************************************************************************/
#ifndef INCLUDE_X3_VIO_PATCH_H_
#define INCLUDE_X3_VIO_PATCH_H_

#include "hb_vio_common.h"
#include "hb_vio_interface.h"

#define SRC_MAX 4
#define PYM_MAX 4

typedef struct mult_src_info_s {
  int src_num;
  src_img_info_t src_img_info[SRC_MAX];
} mult_src_info_t;

typedef struct mult_img_info_s {
  int img_num;
  img_info_t img_info[PYM_MAX];
} mult_img_info_t;

inline int hb_vio_free(img_info_t *img_info) {
  hb_vio_free_info(HB_VIO_PYM_INFO, img_info);
  return 0;
}
inline int hb_vio_mult_free(mult_img_info_t *mult_img_info) { return 0; }
inline int hb_vio_mult_src_free(mult_src_info_t *mult_src_info) { return 0; }
inline int hb_vio_mult_pym_process(mult_src_info_t *mult_src_info) { return 0; }
#endif  // INCLUDE_X3_VIO_PATCH_H_
