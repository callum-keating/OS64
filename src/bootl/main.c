#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_INPUT_KEY Key;
    UINTN Index;
    EFI_STATUS Status;

    InitializeLib(ImageHandle, SystemTable);

    Print(L"Reading kernel file: BOOT:BOOT/kernel.elf\n");

    while (1) {}
    return EFI_SUCCESS;
}
