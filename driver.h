#define MOUSE_SCALE     380.0   // larger be more sensible

struct simple_boot_drv_s {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gout;
	EFI_SIMPLE_POINTER_PROTOCOL *pmouse;
	EFI_ABSOLUTE_POINTER_PROTOCOL *ptouchpad;

	lv_display_t *display;
	UINTN scr_w, scr_h;

	lv_indev_t *kbd, *mouse, *touchpad;
	lv_obj_t *mouse_cursor;
	UINT64 mouse_w, mouse_h;

	uint16_t *buf;

	// UI setting
	uint16_t locale;
	uint16_t timeout_percent;
	lv_obj_t *timeout;

	lv_group_t *g;
	lv_obj_t *win, *menu, *set;
	lv_obj_t *dbg;
};

extern struct simple_boot_drv_s simple_drv;

void efi_flush_cb(lv_display_t *disp, const lv_area_t *area, unsigned char *pixmap);
void efi_kbd_cb(lv_indev_t *indev, lv_indev_data_t *data);
void efi_mouse_cb(lv_indev_t *indev, lv_indev_data_t *data);
void efi_touchpad_cb(lv_indev_t *indev, lv_indev_data_t *data);
