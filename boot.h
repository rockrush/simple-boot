#ifndef	_SIMPLEBOOT_BOOT_H
#define	_SIMPLEBOOT_BOOT_H

#include "lvgl.h"

struct menuentry {
	lv_obj_t *btn;
	struct menuentry *next;
};

#endif	//_SIMPLEBOOT_BOOT_H
