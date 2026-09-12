#include "limine/boot_data.h"
#include <stdint.h>
#include "log.h"
#include "hcf.h"

// structure for revision 0 (version 1.0)
struct RSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __attribute__ ((packed));

// structure for revision 2 (version 2.0+)
struct XSDP_t {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;      // deprecated since version 2.0

 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));

struct SDT_header {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed));

struct ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};

enum header_types {
    APIC,   // Multiple APIC Description Table (MADT)
    BERT,   // Boot Error Record Table (BERT)
    CPEP,   // Corrected Platform Error Polling Table (CPEP)
    DSDT,   // Differentiated System Description Table (DSDT)
    ECDT,   // Embedded Controller Boot Resources Table (ECDT)
    EINJ,   // Error Injection Table (EINJ)
    ERST,   // Error Record Serialization Table (ERST)
    FACP,   // Fixed ACPI Description Table (FADT)
    FACS,   // Firmware ACPI Control Structure (FACS)
    HEST,   // Hardware Error Source Table (HEST)
    MSCT,   // Maximum System Characteristics Table (MSCT)
    MPST,   // Memory Power State Table (MPST)
    OEMx,   // OEM Specific Information Tables (Any table with a signature beginning with "OEM" falls into this definition)
    PMTT,   // Platform Memory Topology Table (PMTT)
    PSDT,   // Persistent System Description Table (PSDT)
    RASF,   // ACPI RAS Feature Table (RASF)
    RSDT,   // Root System Description Table (This wiki page; included for completeness)
    SBST,   // Smart Battery Specification Table (SBST)
    SLIT,   // System Locality System Information Table (SLIT)
    SRAT,   // System Resource Affinity Table (SRAT)
    SSDT,   // Secondary System Description Table (SSDT)
    XSDT,   // Extended System Description Table (XSDT; 64-bit version of the RSDT)
};

static void handle_rsdp(struct RSDP_t *rsdp) {
    uintptr_t rsdt_phys = rsdp->RsdtAddress;
    uintptr_t rsdt_virt = rsdt_phys + boot_data_get_hhdm_response()->offset;
    uint32_t *RSDT = (uint32_t *)rsdt_virt;
    
}

static void handle_xsdp(struct XSDP_t *xsdp) {
    
}

void name(void) {
    void *addr = boot_data_get_rsdp_response()->address;

    struct RSDP_t *rsdp = (struct RSDP_t *)addr;
    struct XSDP_t *xsdp = (struct XSDP_t *)addr;
    logf("RSDP Revision: %d\n", rsdp->Revision);
    if (rsdp->Revision < 2) {
        return handle_rsdp(rsdp);
    } else {
        return handle_xsdp(xsdp);
    }
}

