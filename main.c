#include <efi.h>
#include <efilib.h>

#include "lvgl.h"
#include "demos/lv_demos.h"

#define	MOUSE_SCALE	5

UINT64 mouse_w = 0, mouse_h = 0;
extern lv_obj_t *label_debug;

static UINTN scr_width, scr_height;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *gout;
static EFI_SIMPLE_POINTER_PROTOCOL *pointer;

void lv_efi_entry(EFI_SYSTEM_TABLE *table);

void efi_flush_cb(lv_display_t *disp, const lv_area_t *area, unsigned char *pixmap)
{
	uint32_t *buf = (uint32_t *)pixmap;
	UINTN width, height, x, y;

	if (area->x2 >= area->x1) {
		width = area->x2 - area->x1;
		x = area->x1;
	} else {
		width = area->x1 - area->x2;
		x = area->x2;
	}
	if (area->y2 >= area->y1) {
		height = area->y2 - area->y1;
		y = area->y1;
	} else {
		height = area->y1 - area->y2;
		y = area->y2;
	}
	width++, height++;

	gout->Blt(gout, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)buf, EfiBltBufferToVideo,
			0, 0, x, y, width, height, 0);
	lv_display_flush_ready(disp);
}

uint32_t keycode_to_ascii(EFI_INPUT_KEY key)
{
	switch (key.ScanCode) {
	case SCAN_UP:	return LV_KEY_UP;
	case SCAN_DOWN:	return LV_KEY_DOWN;
	case SCAN_LEFT:	return LV_KEY_LEFT;
	case SCAN_RIGHT:return LV_KEY_RIGHT;
	case SCAN_HOME:	return LV_KEY_HOME;
	case SCAN_END:	return LV_KEY_END;
	case SCAN_DELETE:	return LV_KEY_DEL;
	case SCAN_PAGE_UP:	return LV_KEY_PREV;
	case SCAN_PAGE_DOWN:	return LV_KEY_NEXT;
	case SCAN_ESC:	return LV_KEY_ESC;
	case SCAN_NULL:
		switch (key.UnicodeChar) {
		case 13:	return LV_KEY_ENTER;
		default:	return key.UnicodeChar;
		}
	default:	return key.UnicodeChar;
	}
}

void efi_kbd_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
	EFI_STATUS Status;
	EFI_INPUT_KEY Key;

	LV_UNUSED(indev);
	Status =  gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
	if (Status == EFI_SUCCESS) {
		data->key = keycode_to_ascii(Key);
		data->state = LV_INDEV_STATE_PRESSED;
		return;
	}
	data->state = LV_INDEV_STATE_RELEASED;
}

void efi_mouse_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
	EFI_STATUS status;
	EFI_SIMPLE_POINTER_STATE state;

	LV_UNUSED(indev);
	status = pointer->GetState(pointer, &state);
	if (status == EFI_DEVICE_ERROR) {
		Print(L"Mouse callback failed.\n\r");
		return;
	} else if (status == EFI_NOT_READY) {
		mouse_w = 1;
		mouse_h = 1;
	} else if (status == EFI_SUCCESS) {
		// TODO: if button pressed: LeftButton/RightButton
		if (!state.LeftButton && !state.RightButton)
			return;

		data->point.x += MOUSE_SCALE * state.RelativeMovementX / pointer->Mode->ResolutionX;
		data->point.y += MOUSE_SCALE * state.RelativeMovementY / pointer->Mode->ResolutionY;
		data->state = LV_INDEV_STATE_PRESSED;

		if (data->point.x < 0)
			data->point.x = 0;
		else if (data->point.x >= (int32_t)(scr_width - 4))
			data->point.x = (int32_t)(scr_width - 4);
		if (data->point.y < 0)
			data->point.y = 0;
		else if (data->point.y >= (int32_t)(scr_height - 8))
			data->point.y = (int32_t)(scr_height - 8);

		mouse_w = state.RelativeMovementX;
		mouse_h = state.RelativeMovementY;
		return;
	} else
		data->state = LV_INDEV_STATE_RELEASED;
}

EFI_STATUS efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE *sys_table)
{
	EFI_STATUS status;
	lv_display_t *display;
	lv_indev_t *kbd, *mouse;
	lv_obj_t *mouse_cursor;
	static uint16_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX / 10];

	LV_UNUSED(sys_table);
	if (!handle)
		return EFI_INVALID_PARAMETER;

	status = gBS->LocateProtocol(&SimplePointerProtocol, NULL, (VOID **)&pointer);
	if (EFI_ERROR(status))
		return EFI_UNSUPPORTED;
	status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &gout);
	if (EFI_ERROR(status))
		return EFI_UNSUPPORTED;
	scr_width = gout->Mode->Info->HorizontalResolution;
	scr_height = gout->Mode->Info->VerticalResolution;

	//status = sys_table->ConIn->Reset(sys_table->ConIn, FALSE);
	//if (EFI_ERROR(status))
	//	return status;
	//status =  gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

	lv_init();
	// LVGL display device
	display = lv_display_create(scr_width, scr_height);
	lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(display, efi_flush_cb);

	// LVGL input device
	// TODO: also support touchpad
	kbd = lv_indev_create();
	lv_indev_set_type(kbd, LV_INDEV_TYPE_KEYPAD);
	lv_indev_set_read_cb(kbd, efi_kbd_cb);

	//status = pointer->Reset(pointer, FALSE);
	//if (EFI_ERROR(status))
	//	return EFI_DEVICE_ERROR;
	mouse = lv_indev_create();
	mouse_cursor = lv_image_create(lv_screen_active()); 
	lv_indev_set_type(mouse, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(mouse, efi_mouse_cb);
	lv_image_set_src(mouse_cursor, LV_SYMBOL_HOME);
	lv_indev_set_cursor(mouse, mouse_cursor);

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
