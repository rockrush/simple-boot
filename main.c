#include <efi.h>
#include <efilib.h>
#include "lvgl.h"

void efi_flush_cb(lv_display_t *disp, const lv_area_t *area, unsigned char *pixmap)
{
	uint32_t *buf = (uint32_t *)pixmap;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gout;
	EFI_STATUS status;
	UINTN width, height;

	status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &gout);
	if (EFI_ERROR(status))
		return;

	if (area->x2 >= area->x1)
		width = area->x2 - area->x1;
	else
		width = area->x1 - area->x2;
	if (area->y2 >= area->y1)
		height = area->y2 - area->y1;
	else
		height = area->y1 - area->y2;
	width++, height++;

	status = gout->Blt(gout, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)buf, EfiBltBufferToVideo,
			0, 0, area->x1, area->y1, width, height, 0);

	lv_display_flush_ready(disp);
}

EFI_STATUS efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE *sys_table)
{
	CHAR16 msg[] = L"Hello";
	EFI_STATUS status;
	lv_display_t *display;
	static uint16_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX / 10];

	if (!handle)
		return EFI_INVALID_PARAMETER;

	// TODO: EFI Console output test
	status = sys_table->ConOut->ClearScreen(sys_table->ConOut);
	if (EFI_ERROR(status))
		return status;

	status = sys_table->ConOut->OutputString(sys_table->ConOut, msg);
	if (EFI_ERROR(status))
		return status;

	status = sys_table->ConIn->Reset(sys_table->ConIn, FALSE);
	if (EFI_ERROR(status))
		return status;

	// LVGL
	lv_init();
	display = lv_display_create(LV_HOR_RES_MAX, LV_VER_RES_MAX);
	lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
	//lv_display_set_resolution(display, 1920, 1080);
	lv_display_set_flush_cb(display, efi_flush_cb);

	// TODO: register input device

	lv_obj_t *m = lv_msgbox_create(lv_scr_act());
	lv_msgbox_add_text(m, "Xingyou Chen <rockrush@rockwork.org>");

	while (TRUE) {
		lv_tick_inc(5);
		lv_task_handler();
	}


	return EFI_SUCCESS;
}
