# USB Boot Enhancements Plan

This document outlines the steps and current changes required to evolve the educational USB stack into a minimally boot-capable USB mass storage implementation so ICS-OS can boot directly from a USB thumb drive (BIOS/legacy mode first, then optionally UEFI).

## Current State Summary
- `usb.c` provides simulated controller detection and a fake mass storage device if none are found.
- Real UHCI/OHCI/EHCI register programming is not implemented (only a soft reset for UHCI).
- Control transfers, enumeration (SET_ADDRESS, GET_DESCRIPTOR sequence), configuration, and endpoint parsing are stubbed/simulated.
- Bulk-Only Transport SCSI wrapper (CBW/CSW) functions exist, but bulk transfer is simulated and does not set up TD/QH chains.
- Block device registration provides a logical device for higher layers, usable for experimentation but not for actual hardware boot.

## Requirements For Real USB Boot
1. Early USB initialization before filesystem mount (in `kernel32.c` after PCI init, before mounting root FS).
2. Legacy BIOS boot path (MBR + stage2) still loads kernel; kernel must then mount the SAME USB device as root (consistency of drive geometry, block size).
3. Real device enumeration:
   - Reset port, debounce, check connect status & enable bit.
   - Issue standard requests: GET_DESCRIPTOR (8 bytes), SET_ADDRESS, full device descriptor, configuration descriptor tree walk, SET_CONFIGURATION.
   - Parse interface 0, class 0x08, subclass 0x06, protocol 0x50, locate bulk IN/OUT endpoints.
4. Implement minimal UHCI transaction layer:
   - Frame List (1KB aligned) with TD/QH structures.
   - TD preparation for SETUP (8 bytes), DATA (variable), STATUS stages.
   - Poll (or IRQ) for TD completion; simple timeout & retry.
5. Implement control transfer builder using UHCI TDs.
6. Implement bulk transfers: build TD chains for OUT (CBW/data) and IN (data/CSW) phases.
7. Error handling: detect STALL, clear feature HALT on endpoint, retry limited times.
8. SCSI layer: retain existing CBW/CSW logic; add REQUEST SENSE on failure.
9. Block device caching (optional) for performance.
10. Logging & diagnostics: keep circular buffer, add USB trace toggle.

## Incremental Implementation Plan
| Phase | Goal | Key Files | Notes |
|-------|------|-----------|-------|
| 1 | Refactor simulation vs real paths | usb.c | Introduce feature flag `USB_SIMULATION` (build-time). |
| 2 | Basic UHCI data structures | usb_uhci.c (new) | Define TD, QH structs, frame list allocation. |
| 3 | Control transfer engine | usb_uhci.c | Support standard requests (GET_DESCRIPTOR, SET_ADDRESS, SET_CONFIGURATION). |
| 4 | Enumeration & endpoint parse | usb_enum.c (new) | Walk config descriptor; detect mass storage. |
| 5 | Bulk-only transport (real) | usb_mass_storage.c | Replace simulated bulk with real TD chains. |
| 6 | Robust SCSI (sense, retries) | usb_mass_storage.c | Add REQUEST SENSE. |
| 7 | Boot integration | kernel32.c | Call usb_init earlier; mount USB as root if present. |

## Data Structures (UHCI)
```
struct uhci_td { // 16-byte aligned
  uint32_t link;      // Next TD/QH pointer
  uint32_t ctrl;      // Control/status bits
  uint32_t token;     // PID, device addr, endpoint, data toggle, length
  uint32_t buffer;    // Data buffer pointer
} __attribute__((packed, aligned(16)));

struct uhci_qh { // 16-byte aligned
  uint32_t head_link; // Terminate | QH/T
  uint32_t elem_link; // First TD
} __attribute__((packed, aligned(16)));
```

## Minimal Control Transfer Flow
1. Build SETUP TD (PID=SETUP, toggle=0) 8 bytes.
2. Optional DATA TD (IN/OUT, toggle=1).
3. STATUS TD (opposite direction, toggle=1) with length=0.
4. Link into QH; insert QH into frame list all frames (periodic reuse) or a single frame polled loop.
5. Poll status bits (Active, Stalled, Error).

## Risk & Simplifications
- Focus only on UHCI (most straightforward) first; skip OHCI/EHCI.
- Polling instead of interrupts to keep code small.
- Single device support initially (first detected mass storage) -> scale later.
- Ignore hubs (assume direct-connect). Add hub reset later if needed.

## Boot Considerations
- BIOS already loaded kernel from the USB stick: we need same drive to appear at LBA mapping identical to BIOS assumptions. Usually BIOS emulates the USB stick as INT13 device; once in protected mode we now drive USB directly.
- Potential mismatch: BIOS may have used CHS/LBA translation; our driver uses raw sector LBA. Typically fine for modern sticks.
- Provide fallback: If USB init fails, continue with RAM disk or existing root FS.

## Fallback / Simulation Path
- Retain current simulated device for teaching when `USB_SIMULATION=1` or no controller present.
- Expose a kernel command (`usb.mode sim|real`) to toggle.

## Testing Strategy
- Phase tests: 
  - Phase 2: Dump frame list addresses; confirm controller runs (USBSTS). 
  - Phase 3: Issue GET_DESCRIPTOR (8B) and validate first bytes (length=18, type=1). 
  - Phase 5: Read LBA0, verify partition signature 0x55AA.
- Provide `usb diag` shell command to run a scripted test sequence and log results.

## Next Steps (Action Items)
1. Add new source files scaffolding: `usb_uhci.c`, `usb_enum.c` with stubs.
2. Add build flags in `hardware/usb/Makefile`.
3. Refactor `usb_control_transfer` to dispatch to backend (sim vs uhci).
4. Add early call in kernel init sequence.

---
This is a living plan; update as implementation proceeds.
