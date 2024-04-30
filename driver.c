#include <efi.h>
#include <efilib.h>

#include "lvgl.h"
#include "driver.h"
#include "demos/lv_demos.h"

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

	simple_drv.gout->Blt(simple_drv.gout, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)buf, EfiBltBufferToVideo,
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
	int sens = simple_drv.scr_w / MOUSE_SCALE;

	LV_UNUSED(indev);
	status = simple_drv.pmouse->GetState(simple_drv.pmouse, &state);
	if (status == EFI_DEVICE_ERROR) {
		Print(L"Mouse callback failed.\n\r");
		return;
	//} else if (status == EFI_NOT_READY) {
	} else if (status == EFI_SUCCESS) {
		// TODO: if button pressed: LeftButton/RightButton
		//if (!state.LeftButton && !state.RightButton)
		//	return;

		data->point.x += sens * state.RelativeMovementX / simple_drv.pmouse->Mode->ResolutionX;
		data->point.y += sens * state.RelativeMovementY / simple_drv.pmouse->Mode->ResolutionY;
		data->state = LV_INDEV_STATE_PRESSED;

		if (data->point.x < 0)
			data->point.x = 0;
		else if (data->point.x >= (int32_t)(simple_drv.scr_w - 4))
			data->point.x = (int32_t)(simple_drv.scr_w - 4);
		if (data->point.y < 0)
			data->point.y = 0;
		else if (data->point.y >= (int32_t)(simple_drv.scr_h - 8))
			data->point.y = (int32_t)(simple_drv.scr_h - 8);

		return;
	} else
		data->state = LV_INDEV_STATE_RELEASED;
}

void efi_touchpad_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
	EFI_STATUS status;
	EFI_ABSOLUTE_POINTER_STATE state;
	int sens = simple_drv.scr_w / (simple_drv.ptouchpad->Mode->AbsoluteMaxX - simple_drv.ptouchpad->Mode->AbsoluteMinX) / MOUSE_SCALE;

	LV_UNUSED(indev);
	status = simple_drv.ptouchpad->GetState(simple_drv.ptouchpad, &state);
	if (status == EFI_DEVICE_ERROR) {
		Print(L"Touchpad callback failed.\n\r");
		return;
	//} else if (status == EFI_NOT_READY) {
	} else if (status == EFI_SUCCESS) {
		//if (!state.ActiveButton)
		//	return;

		data->point.x += sens * (state.CurrentX - simple_drv.ptouchpad->Mode->AbsoluteMinX);
		data->point.y += sens * (state.CurrentY - simple_drv.ptouchpad->Mode->AbsoluteMinY);
		data->state = LV_INDEV_STATE_PRESSED;
		return;
	} else
		data->state = LV_INDEV_STATE_RELEASED;
}
