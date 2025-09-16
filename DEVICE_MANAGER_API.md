# DeviceManager API Documentation

## Overview

The `DeviceManager` class provides a clean, modular interface for MIDI device connection, detection, and configuration in the MidiPlay application. This replaces the previously inline device management code from `play.cpp` with a well-structured, extensible architecture.

## Files

- **`device_manager.hpp`** - Header file with class declarations and interfaces
- **`device_manager.cpp`** - Implementation file with device management logic
- **`device_constants.hpp`** - Device-specific constants (shared with existing device classes)

## Class: `MidiPlay::DeviceManager`

### Public Methods

#### `DeviceInfo connectAndDetectDevice(cxxmidi::output::Default& outport)`

Handles the complete device connection process with timeout handling.

**Parameters:**
- `outport` - Reference to the MIDI output port

**Returns:**
- `DeviceInfo` struct containing device type, port name, and port index

**Throws:**
- `std::runtime_error` if device connection times out

**Example:**
```cpp
MidiPlay::DeviceManager deviceManager;
try {
    MidiPlay::DeviceInfo deviceInfo = deviceManager.connectAndDetectDevice(outport);
    std::cout << "Connected to: " << deviceInfo.portName << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Connection failed: " << e.what() << std::endl;
}
```

#### `void createAndConfigureDevice(DeviceType type, cxxmidi::output::Default& outport)`

Factory method that creates the appropriate device instance and configures it with default settings.

**Parameters:**
- `type` - The detected device type (from DeviceInfo)
- `outport` - Reference to the connected MIDI output port

**Throws:**
- `std::invalid_argument` for unsupported device types

**Example:**
```cpp
deviceManager.createAndConfigureDevice(deviceInfo.type, outport);
```

#### `static std::string getDeviceTypeName(DeviceType type)`

Returns a human-readable name for the device type.

**Parameters:**
- `type` - The device type enumeration

**Returns:**
- String description of the device type

**Example:**
```cpp
std::string deviceName = MidiPlay::DeviceManager::getDeviceTypeName(DeviceType::CASIO_CTX3000);
// Returns: "Casio CTX-3000 series"
```

#### `void loadDevicePresets(const std::string& configPath)`

**Note:** This method is prepared for future YAML configuration support but not yet implemented.

**Parameters:**
- `configPath` - Path to the YAML configuration file

## Enumerations

### `DeviceType`

Enumeration of supported MIDI device types:

- `CASIO_CTX3000` - Casio CTX-3000 series keyboards
- `YAMAHA_PSR_EW425` - Yamaha PSR-EW425 series keyboards  
- `ALLEN_PROTEGE` - Allen Protege organs
- `UNKNOWN` - Unknown or unsupported device

## Structures

### `DeviceInfo`

Information about a detected MIDI device:

```cpp
struct DeviceInfo {
    DeviceType type;        // Detected device type
    std::string portName;   // MIDI port name as reported by system
    int portIndex;          // Zero-based port index for connection
};
```

## Usage Pattern

The recommended usage pattern follows this sequence:

```cpp
#include "device_manager.hpp"

// Create DeviceManager instance
MidiPlay::DeviceManager deviceManager;

try {
    // Step 1: Connect and detect device
    MidiPlay::DeviceInfo deviceInfo = deviceManager.connectAndDetectDevice(outport);
    
    // Step 2: Create and configure device
    deviceManager.createAndConfigureDevice(deviceInfo.type, outport);
    
    // Step 3: Display connection information
    std::cout << "Connected to: " << MidiPlay::DeviceManager::getDeviceTypeName(deviceInfo.type) 
              << " (" << deviceInfo.portName << ")" << std::endl;
              
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    exit(MidiPlay::EXIT_DEVICE_NOT_FOUND);
}
```

## Device Detection Logic

The device detection is based on MIDI port name string matching:

| Port Name Pattern | Device Type | Device Class |
|-------------------|-------------|--------------|
| `"CASIO USB"` | `CASIO_CTX3000` | `ctx3000` |
| `"Digital Keyboard"` | `YAMAHA_PSR_EW425` | `psr_ew425` |
| All others | `ALLEN_PROTEGE` | `protege` |

## Connection Timeout Configuration

Connection timeout behavior is controlled by constants from `device_constants.hpp`:

- `MidiPlay::Device::CONNECTION_TIMEOUT` (300) - Maximum retry iterations
- `MidiPlay::Device::POLL_SLEEP_SECONDS` (2) - Sleep duration between retries
- `MidiPlay::Device::MIN_PORT_COUNT` (2) - Minimum ports required for connection
- `MidiPlay::Device::OUTPUT_PORT_INDEX` (1) - Output port index to use

## Error Handling

The DeviceManager uses exception-based error handling:

- **Connection Timeout:** `std::runtime_error` with descriptive message
- **Invalid Device Type:** `std::invalid_argument` for unsupported devices
- **Null Pointers:** `std::invalid_argument` for null device pointers

## Future YAML Configuration Support

The architecture is designed to support YAML configuration files in the future:

```yaml
devices:
  casio_ctx3000:
    detection_string: "CASIO USB"
    channels:
      1: { bank: 32, program: 19 }  # Pipe Organ
      2: { bank: 32, program: 19 }  # Pipe Organ  
      3: { bank: 36, program: 48 }  # Brass/Strings
  yamaha_psr_ew425:
    detection_string: "Digital Keyboard"
    channels:
      1: { bank: 113, program: 20 }  # Chapel Organ
      2: { bank: 113, program: 20 }  # Chapel Organ
      3: { bank: 112, program: 4 }   # Strings
```

## Integration with Existing Code

The DeviceManager integrates seamlessly with existing device classes:

- Uses existing `ctx3000`, `psr_ew425`, and `protege` classes
- Leverages `device_constants.hpp` for device-specific constants
- Maintains backward compatibility with original device behavior
- Follows the same memory management patterns (raw pointers with `new`)

## Benefits

1. **Modularity** - Device logic separated from main application flow
2. **Extensibility** - Easy to add new device types
3. **Maintainability** - Clear separation of concerns
4. **Testability** - Isolated device management functionality
5. **Configuration-Ready** - Prepared for external configuration files
6. **Error Handling** - Comprehensive exception-based error management
7. **Documentation** - Well-documented API with clear usage patterns

## Migration from Legacy Code

The DeviceManager replaces 45 lines of inline device management code from `play.cpp` (lines 432-476) with a clean, reusable interface. The original device detection and configuration logic has been preserved exactly, ensuring no functional changes.

**Before (Legacy):**
```cpp
// Inline timeout loop, device detection, and instantiation
for (int i = 0; true; i++) { /* ... */ }
if (portName.find("CASIO USB") == 0) { /* ... */ }
// etc.
```

**After (DeviceManager):**
```cpp
MidiPlay::DeviceManager deviceManager;
MidiPlay::DeviceInfo info = deviceManager.connectAndDetectDevice(outport);
deviceManager.createAndConfigureDevice(info.type, outport);
```

This transformation significantly improves code maintainability while preserving all original functionality.