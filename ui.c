#include <efi.h>
#include <efilib.h>
#include "demos/lv_demos.h"
#include "driver.h"

static void boot_linux(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED) {
		LV_UNUSED(obj);
		LV_LOG_USER("Selected month: %s\n", buf);
	}
}

void efi_lv_entry(EFI_SYSTEM_TABLE *sys_table)
{
	lv_obj_t *obj, *btn;
	lv_obj_t *parent = lv_screen_active();
	uint16_t h_margin = 260, v_margin = 100;
	//lv_style_t style;

	LV_UNUSED(sys_table);
	lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

	//lv_style_init(&style);
	//LV_IMG_DECLARE(img_cogwheel_argb);
	//lv_style_set_bg_image_src(&style, &img_cogwheel_argb);
	//lv_style_set_bg_image_tiled(&style, true);
	//lv_style_set_bg_image_opa(&style, LV_OPA_COVER);
	//lv_obj_add_style(parent, &style, LV_PART_MAIN|LV_STATE_DEFAULT);
	//lv_obj_update_layout(parent);

	// 1. Label
	obj = lv_label_create(parent);
	lv_label_set_text(obj, "Boot into one of the following:");
	lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

	// 2. menu
	obj = lv_list_create(parent);
	lv_obj_set_size(obj, simple_drv.scr_w - h_margin, simple_drv.scr_h - v_margin);
	lv_obj_center(obj);
	lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

	lv_list_add_text(obj, "Detected");
	btn = lv_list_add_button(obj, LV_SYMBOL_GPS, "Linux 6.9.0-rc6");
	lv_obj_add_event_cb(btn, boot_linux, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_button(obj, LV_SYMBOL_GPS, "Linux 6.6.0");
	lv_obj_add_event_cb(btn, boot_linux, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_button(obj, LV_SYMBOL_GPS, "Linux 6.1.0");
	lv_obj_add_event_cb(btn, boot_linux, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_button(obj, LV_SYMBOL_GPS, "Linux 5.10.0");
	lv_obj_add_event_cb(btn, boot_linux, LV_EVENT_CLICKED, NULL);

	lv_list_add_text(obj, "Custom");
	btn = lv_list_add_button(obj, LV_SYMBOL_GPS, "Windows 11 LSTC");
	lv_obj_add_event_cb(btn, boot_linux, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_button(obj, LV_SYMBOL_GPS, "Plan9 Front 9931");
	lv_obj_add_event_cb(btn, boot_linux, LV_EVENT_CLICKED, NULL);
	btn = lv_list_add_button(obj, LV_SYMBOL_GPS, "Firmware Setting");
	lv_obj_add_event_cb(btn, boot_linux, LV_EVENT_CLICKED, NULL);
	simple_drv.dbg = lv_list_add_text(obj, "[Debug message]");

	// 3. progress
	simple_drv.timeout = lv_bar_create(parent);
	lv_obj_set_size(simple_drv.timeout, simple_drv.scr_w - h_margin, 20);
	lv_obj_center(simple_drv.timeout);
	lv_bar_set_value(simple_drv.timeout, simple_drv.timeout_percent, LV_ANIM_ON);
	//lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

	// 4. status
	obj = lv_label_create(parent);
	lv_label_set_text(obj, "Version: 0.1            F2 for setup");
	lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}
