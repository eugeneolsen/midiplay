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

#### `bool loadDevicePresets(const std::string& configPath = "")`

Loads device configurations from YAML file with automatic file discovery.

**Parameters:**
- `configPath` - Optional path to specific YAML configuration file. If empty, searches standard locations.

**Returns:**
- `true` if YAML configuration loaded successfully
- `false` if no YAML found or parsing failed (falls back to hardcoded defaults)

**File Search Locations (in priority order):**
1. `~/.config/midiplay/midi_devices.yaml` (user-specific)
2. `/etc/midiplay/midi_devices.yaml` (system-wide)
3. `./midi_devices.yaml` (local/development)

**YAML Configuration Structure:**
```yaml
version: "1.0"

# Optional connection parameter overrides
connection:
  timeout_iterations: 300      # Connection timeout (default: 300)
  poll_sleep_seconds: 2        # Sleep between retries (default: 2)
  min_port_count: 2           # Minimum ports required (default: 2)
  output_port_index: 1        # MIDI output port index (default: 1)

# Device configurations
devices:
  casio_ctx3000:
    name: "Casio CTX-3000 Series"
    description: "Casio USB MIDI keyboards with organ sounds"
    detection_strings:
      - "CASIO USB"
    channels:
      1:
        bank_msb: 32
        bank_lsb: 0
        program: 19
        description: "Pipe Organ"
      # ... additional channels
        
  yamaha_psr_ew425:
    name: "Yamaha PSR-EW425 Series"
    detection_strings:
      - "Digital Keyboard"
    channels:
      1:
        bank_msb: 0
        bank_lsb: 113
        program: 20
        description: "Chapel Organ"
      # ... additional channels
      
  allen_protege:
    name: "Allen Protege Organ"
    detection_strings: []       # Empty = fallback device
    channels: {}                # No channel configuration
```

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

## Usage Patterns

### Basic Usage (Hardcoded Defaults)

```cpp
#include "device_manager.hpp"

// Create DeviceManager instance
MidiPlay::DeviceManager deviceManager;

try {
    // Connect and configure device using hardcoded defaults
    MidiPlay::DeviceInfo deviceInfo = deviceManager.connectAndDetectDevice(outport);
    deviceManager.createAndConfigureDevice(deviceInfo.type, outport);
    
    std::cout << "Connected to: " << MidiPlay::DeviceManager::getDeviceTypeName(deviceInfo.type)
              << " (" << deviceInfo.portName << ")" << std::endl;
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    exit(MidiPlay::EXIT_DEVICE_NOT_FOUND);
}
```

### YAML Configuration Usage (Recommended)

```cpp
#include "device_manager.hpp"

// Create DeviceManager instance
MidiPlay::DeviceManager deviceManager;

// Load YAML configuration (searches standard locations automatically)
bool yamlLoaded = deviceManager.loadDevicePresets();
if (yamlLoaded) {
    std::cout << "Using YAML device configuration" << std::endl;
} else {
    std::cout << "Using built-in device defaults" << std::endl;
}

try {
    // Device management now uses YAML config when available
    MidiPlay::DeviceInfo deviceInfo = deviceManager.connectAndDetectDevice(outport);
    deviceManager.createAndConfigureDevice(deviceInfo.type, outport);
    
    std::cout << "Connected to: " << MidiPlay::DeviceManager::getDeviceTypeName(deviceInfo.type)
              << " (" << deviceInfo.portName << ")" << std::endl;
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    exit(MidiPlay::EXIT_DEVICE_NOT_FOUND);
}
```

### Custom YAML Path Usage

```cpp
// Load YAML from specific location
bool yamlLoaded = deviceManager.loadDevicePresets("/path/to/custom/midi_devices.yaml");
if (!yamlLoaded) {
    std::cerr << "Failed to load custom YAML configuration" << std::endl;
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

## YAML Configuration Features

### Automatic Configuration Discovery

The DeviceManager automatically searches for YAML configuration files in standard Debian locations:

1. **User Configuration**: `~/.config/midiplay/midi_devices.yaml`
   - User-specific device customizations
   - Overrides system-wide settings
   
2. **System Configuration**: `/etc/midiplay/midi_devices.yaml`
   - System-wide device configurations
   - Managed by system administrators
   
3. **Local Development**: `./midi_devices.yaml`
   - Local testing and development
   - Version-controlled project configurations

### YAML-Driven Device Detection

Device detection uses YAML configuration when available:

**YAML Approach:**
```yaml
devices:
  casio_ctx3000:
    detection_strings:
      - "CASIO USB"
      - "CTX-3000"    # Multiple detection patterns supported
```

**Fallback to Hardcoded:**
- If no YAML found, uses original hardcoded detection strings
- Maintains backward compatibility with existing deployments

### YAML-Driven Device Configuration

**Channel Configuration via YAML:**
```yaml
devices:
  custom_device:
    name: "Custom MIDI Keyboard"
    detection_strings:
      - "Custom Device Name"
    channels:
      1:
        bank_msb: 10
        bank_lsb: 20
        program: 5
        description: "Custom Sound 1"
      2:
        bank_msb: 15
        bank_lsb: 25
        program: 10
        description: "Custom Sound 2"
```

**Runtime Output:**
```
Channel 1: Custom Sound 1 (Bank 10:20, Program 5)
Channel 2: Custom Sound 2 (Bank 15:25, Program 10)
```

### Connection Parameter Override

YAML can override connection behavior:

```yaml
connection:
  timeout_iterations: 600      # Double the default timeout
  poll_sleep_seconds: 1        # Faster polling
  min_port_count: 3           # Require more ports
  output_port_index: 2        # Use different output port
```

### Volume Control Philosophy

**Note**: YAML configuration intentionally omits volume settings. Volume control is left to the organist via physical volume pedals, maintaining the traditional organ playing experience.

### Adding New Device Types

To add support for a new device via YAML:

1. **Add device section to YAML:**
```yaml
devices:
  new_device_type:
    name: "New Device Brand Model"
    description: "Device description"
    detection_strings:
      - "Device USB Name"
    channels:
      1:
        bank_msb: 0
        bank_lsb: 0
        program: 1
        description: "Default Sound"
```

2. **No code changes required** - DeviceManager automatically detects and configures new devices from YAML

### Error Handling and Validation

**YAML Parsing Errors:**
- Comprehensive error reporting via yaml-cpp library
- Graceful fallback to hardcoded defaults on any YAML error
- Detailed error messages for troubleshooting

**Example Error Output:**
```
YAML parsing error: invalid key "channels" at line 15
Failed to parse YAML configuration. Using built-in defaults.
```

### Dependencies

**Required Library:**
- `yaml-cpp` - Modern C++ YAML parser library
- Install: `sudo apt install libyaml-cpp-dev` (Debian/Ubuntu)
- Link: Add `-lyaml-cpp` to compiler args

**Header Include:**
```cpp
#include <yaml-cpp/yaml.h>  // Required for YAML functionality
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