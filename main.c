#include <efi.h>
#include <efilib.h>

#include "lvgl.h"
#include "driver.h"
#include "demos/lv_demos.h"

UINT64 mouse_w = 0, mouse_h = 0;
extern lv_obj_t *label_debug;

struct simple_boot_drv_s simple_drv;

EFI_STATUS efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE *sys_table)
{
	EFI_STATUS status;
	int tmp_len;

	LV_UNUSED(sys_table);
	if (!handle)
		return EFI_INVALID_PARAMETER;

	status = gBS->LocateProtocol(&SimplePointerProtocol, NULL, (VOID **)&simple_drv.pmouse);
	if (EFI_ERROR(status))
		return EFI_UNSUPPORTED;
	status = gBS->LocateProtocol(&AbsolutePointerProtocol, NULL, (VOID **)&simple_drv.ptouchpad);
	if (EFI_ERROR(status))
		return EFI_UNSUPPORTED;
	status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &simple_drv.gout);
	if (EFI_ERROR(status))
		return EFI_UNSUPPORTED;
	simple_drv.scr_w = simple_drv.gout->Mode->Info->HorizontalResolution;
	simple_drv.scr_h = simple_drv.gout->Mode->Info->VerticalResolution;

	// Reset devices
	status = sys_table->ConIn->Reset(sys_table->ConIn, FALSE);
	if (EFI_ERROR(status))
		return status;
	status = simple_drv.pmouse->Reset(simple_drv.pmouse, FALSE);
	if (EFI_ERROR(status))
		return EFI_DEVICE_ERROR;
	status = simple_drv.ptouchpad->Reset(simple_drv.ptouchpad, FALSE);
	if (EFI_ERROR(status))
		return EFI_DEVICE_ERROR;
	//status =  gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

	// LVGL display device
	lv_init();
	simple_drv.display = lv_display_create(simple_drv.scr_w, simple_drv.scr_h);
	tmp_len = sizeof(uint16_t) * simple_drv.scr_w * simple_drv.scr_h / 10;
	gST->BootServices->AllocatePool(EfiLoaderData, tmp_len, (void **)&simple_drv.buf);
	lv_display_set_buffers(simple_drv.display, simple_drv.buf, NULL, tmp_len, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(simple_drv.display, efi_flush_cb);

	// LVGL keyboard device
	simple_drv.kbd = lv_indev_create();
	lv_indev_set_type(simple_drv.kbd, LV_INDEV_TYPE_KEYPAD);
	lv_indev_set_read_cb(simple_drv.kbd, efi_kbd_cb);

	// LVGL mouse device
	simple_drv.mouse = lv_indev_create();
	simple_drv.mouse_cursor = lv_image_create(lv_screen_active()); 
	lv_indev_set_type(simple_drv.mouse, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(simple_drv.mouse, efi_mouse_cb);
	lv_image_set_src(simple_drv.mouse_cursor, LV_SYMBOL_EDIT);	// TODO: cursor
	lv_indev_set_cursor(simple_drv.mouse, simple_drv.mouse_cursor);

	// LVGL touchpad device
	simple_drv.touchpad = lv_indev_create();
	lv_indev_set_type(simple_drv.touchpad, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(simple_drv.touchpad, efi_touchpad_cb);
	lv_indev_set_cursor(simple_drv.touchpad, simple_drv.mouse_cursor);

	// TODO: find and draw boot entries
	lv_efi_entry(sys_table);

	while (TRUE) {
		char dbg_msg[128];
		lv_snprintf(dbg_msg, 120, "Mouse: x=%d, y=%d", mouse_w, mouse_h);
		lv_label_set_text(label_debug, dbg_msg);
		lv_tick_inc(50);
		lv_task_handler();
	}


	return EFI_SUCCESS;
}
