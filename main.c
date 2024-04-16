#include <efi.h>

typedef void * efi_handle_t;

EFI_STATUS efi_main(efi_handle_t handle, EFI_SYSTEM_TABLE *sys_table)
{
	uint16_t msg[] = u"Hello";
	EFI_STATUS status;

	status = sys_table->ConOut->ClearScreen(sys_table->ConOut);
	if (status)
		return status;

	status = sys_table->ConOut->OutputString(sys_table->ConOut, msg);
	if (status)
		return status;

	return 0;
}
