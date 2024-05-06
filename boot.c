#include <efi.h>
#include <efilib.h>
//#include <efishell.h>

#include "driver.h"
#include "boot.h"

/* Loop over ESP or more filesystems, for EFI files, Linux kernels, etc. */
/* TODO(Refact): only apps under shell use EFI_SHELL_PROTOCOL */
/*
int find_entries(void)
{
	int amount = 0;
	EFI_FILE_HANDLE fs_handle;
	EFI_SHELL_FILE_INFO *fs_list;
	EFI_STATUS status;

	status = simple_drv.shell->OpenFileByName(L"FS0:", (SHELL_FILE_HANDLE *)&fs_handle, EFI_FILE_MODE_READ);
	if (EFI_ERROR(status))
		return 0;

	// FindFiles
	status = simple_drv.shell->FindFilesInDir(fs_handle, &fs_list);
	return amount;
}*/
