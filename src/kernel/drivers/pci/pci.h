#pragma once
#include <stdint.h>

struct pcieDeviceStruct {
    uint8_t     bus;            // The bus that the device is in
    uint8_t     device;         // The device that the device is in
    uint8_t     class_code;     // Specifies the type of function the device performs
    uint8_t     subclass;       // Specifies the specific function the device performs
    uint8_t     revision_id;    // Specifies a revision identifier for a particular device
    uint8_t     header_type;    // If bit 7 of this header is set the device performs multiple functions
    uint8_t     prog_if;        // idk what this does but i'll figure it out
    bool        multifunction;  // does device have mor then 1 function
    uint16_t    device_id;      // Identifies the device
    uint16_t    vendor_id;      // Identifies the manufacturer of the device
};

uint32_t pci_enumeratePci(struct pcieDeviceStruct devices[256]);
