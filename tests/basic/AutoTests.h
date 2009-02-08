/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_AUTOTESTS_H
#define  COCO_AUTOTESTS_H

#define COCO_ASSERT(x)  if (!x) coco_assert_failed(__FILE__, __LINE__)

void coco_assert_failed(const char * fname, int lineno);

#endif  /* COCO_AUTOTESTS_H */
