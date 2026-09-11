#include <stdint.h>
#include "pci.h"
#include "drivers/serial.h"
#include "io.h"

#define CONTROL_PORT 0xCF8      // control port // replace with better comment
#define FORWARDING_PORT 0xCFA   // specifies the bus number the configuration will target


static uint16_t current_port = 0;

inline void setOffset(uint8_t offset) {
    if (offset & 0x03) {
        write_serial_str("setRegister recived a value that was not DWORD aligned returning without updating register\n"
                "this can cause major issues\n");
        return;
    }
    current_port = (current_port & 0xFF03) | offset;
}


inline void selectDevice(uint8_t bus, uint8_t device, uint8_t function) {
                                    uint8_t data = 0;
        /* sets function number */  data = 0xF0 | ((function & 0x7) << 1); /* and enables access mechanism */
                                    outb(CONTROL_PORT, data);
                                    outb(FORWARDING_PORT, bus);
                                    //                  set firt two bits to 1 | set device
                                    current_port = 0xC000 | ((device & 0x0F) << 8);
}

inline uint32_t readPort() {
    return inl(current_port);
}

/*
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
*/

struct pcieDeviceStruct checkDevice(uint16_t bus, uint8_t device) {
    struct pcieDeviceStruct pciDevice = {0};
    pciDevice.bus = bus;
    pciDevice.device = device;
    selectDevice(bus, device, 0);

    setOffset(0x0);
    uint32_t data = readPort();
    pciDevice.vendor_id = data & 0xFFFF;
    pciDevice.device_id = (data >> 16) & 0xFFFF;

    setOffset(0x8);
    data = readPort();
    pciDevice.subclass = (data >> 16) & 0xFF;
    pciDevice.class_code = (data >> 24) & 0xFF;
    pciDevice.prog_if = (data >> 8) & 0xFF;
    pciDevice.revision_id = data & 0xFF;

    setOffset(0xC);
    data = readPort();
    pciDevice.header_type = (data >> 16) & 0xFF;

    pciDevice.multifunction = (pciDevice.header_type & 0x80) != 0;

    return pciDevice;
}

uint32_t pci_enumeratePci(struct pcieDeviceStruct devices[256]) {
    int devicecount = 0;
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 16; device++) {
            struct pcieDeviceStruct pciDevice = checkDevice(bus, device);
            if (pciDevice.vendor_id == 0xFFFF)
                continue;

            if (devicecount >= 256)
                return devicecount;

            write_serial_str("PCI DEVICE FOUND\n");
            devices[devicecount++] = pciDevice;
        }
    }
    return devicecount;
};
