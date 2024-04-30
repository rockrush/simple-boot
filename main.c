#include <efi.h>
#include <efilib.h>

#include "lvgl.h"
#include "driver.h"
#include "demos/lv_demos.h"

struct simple_boot_drv_s simple_drv;

void efi_lv_entry(EFI_SYSTEM_TABLE *table);

// Unit: 100 millisecond
EFI_STATUS efi_delay(void)
{
		EFI_STATUS status;
		EFI_EVENT event;
		void *wait_list[2];
		UINTN index = 0;
		status = gBS->CreateEvent(EVT_TIMER, TPL_APPLICATION, (EFI_EVENT_NOTIFY)NULL, (VOID *)NULL, &event);
		if (EFI_ERROR(status))
			lv_label_set_text(simple_drv.dbg, "EFI:CreateEvent");
		status = gBS->SetTimer(event, TimerPeriodic, 1000*1000);
		if (EFI_ERROR(status))
			lv_label_set_text(simple_drv.dbg, "EFI:SetTimer");

		wait_list[0] = event;
		status = gBS->WaitForEvent(1, wait_list, &index);
		return status;
}

EFI_STATUS efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE *sys_table)
{
	EFI_STATUS status;
	int buf_size;
	int countdown = 10;

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
	simple_drv.g = lv_group_create();
	lv_group_set_default(simple_drv.g);
	simple_drv.display = lv_display_create(simple_drv.scr_w, simple_drv.scr_h);
	buf_size = sizeof(uint16_t) * simple_drv.scr_w * simple_drv.scr_h / 10;
	gST->BootServices->AllocatePool(EfiLoaderData, buf_size, (void **)&simple_drv.buf);
	lv_display_set_buffers(simple_drv.display, simple_drv.buf, NULL, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(simple_drv.display, efi_flush_cb);

	// LVGL keyboard device
	simple_drv.kbd = lv_indev_create();
	lv_indev_set_type(simple_drv.kbd, LV_INDEV_TYPE_KEYPAD);
	lv_indev_set_read_cb(simple_drv.kbd, efi_kbd_cb);
	lv_indev_set_group(simple_drv.kbd, simple_drv.g);

	// LVGL mouse device
	simple_drv.mouse = lv_indev_create();
	simple_drv.mouse_cursor = lv_image_create(lv_screen_active()); 
	lv_indev_set_type(simple_drv.mouse, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(simple_drv.mouse, efi_mouse_cb);
	lv_image_set_src(simple_drv.mouse_cursor, LV_SYMBOL_EDIT);	// TODO: cursor
	lv_indev_set_cursor(simple_drv.mouse, simple_drv.mouse_cursor);
	lv_indev_set_group(simple_drv.mouse, simple_drv.g);

	// LVGL touchpad device
	simple_drv.touchpad = lv_indev_create();
	lv_indev_set_type(simple_drv.touchpad, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(simple_drv.touchpad, efi_touchpad_cb);
	lv_indev_set_cursor(simple_drv.touchpad, simple_drv.mouse_cursor);
	lv_indev_set_group(simple_drv.touchpad, simple_drv.g);

	simple_drv.timeout_percent = 100;
	efi_lv_entry(sys_table);

	while (TRUE) {
		char dbg_msg[32];

		efi_delay();
		lv_tick_inc(100);

		countdown--;
		if (countdown <= 0) {
			countdown = 10;
			simple_drv.timeout_percent -= 2;
			if (simple_drv.timeout_percent == 0)
				simple_drv.timeout_percent = 100;
			lv_bar_set_value(simple_drv.timeout, simple_drv.timeout_percent, LV_ANIM_ON);

			lv_snprintf(dbg_msg, 30, "[Debug] seconds left: %d", simple_drv.timeout_percent / 2);
			if (simple_drv.dbg)
				lv_label_set_text(simple_drv.dbg, dbg_msg);
		}

		lv_task_handler();
	}

	return EFI_SUCCESS;
}
