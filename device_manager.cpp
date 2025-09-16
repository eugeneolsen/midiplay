#include "device_manager.hpp"
#include "constants.hpp"
#include "ctx3000.hpp"
#include "psr-ew425.hpp"
#include "protege.hpp"

#include <stdexcept>
#include <iostream>
#include <unistd.h>

namespace MidiPlay {

    DeviceInfo DeviceManager::connectAndDetectDevice(cxxmidi::output::Default& outport) {
        // Wait for device connection with timeout
        if (!waitForDeviceConnection(outport)) {
            throw std::runtime_error("Device connection timeout. No device found. Connect a MIDI device and try again.");
        }

        // Open the MIDI output port
        outport.OpenPort(MidiPlay::Device::OUTPUT_PORT_INDEX);

        // Get port information and detect device type
        std::string portName = outport.GetPortName(MidiPlay::Device::OUTPUT_PORT_INDEX);
        DeviceType type = detectDeviceType(portName);

        return DeviceInfo(type, portName, MidiPlay::Device::OUTPUT_PORT_INDEX);
    }

    void DeviceManager::createAndConfigureDevice(DeviceType type, cxxmidi::output::Default& outport) {
        // Create device instance and configure it with default settings
        // This matches the original pattern from play.cpp (using raw pointers with new)
        
        switch (type) {
            case DeviceType::CASIO_CTX3000: {
                // This is a Casio USB MIDI device.
                ctx3000* ctx = new ctx3000(outport);
                ctx->SetDefaults();
                break;
            }
            case DeviceType::YAMAHA_PSR_EW425: {
                // Yamaha PSR-EW425 series keyboard
                psr_ew425* psr = new psr_ew425(outport);
                psr->SetDefaults();
                break;
            }
            case DeviceType::ALLEN_PROTEGE: {
                // Allen Protege organ or other unrecognized device
                protege* p = new protege(outport);
                p->SetDefaults();
                break;
            }
            case DeviceType::UNKNOWN:
            default:
                throw std::invalid_argument("Cannot create device for unknown or unsupported device type");
        }
    }

    std::string DeviceManager::getDeviceTypeName(DeviceType type) {
        switch (type) {
            case DeviceType::CASIO_CTX3000:
                return "Casio CTX-3000 series";
            case DeviceType::YAMAHA_PSR_EW425:
                return "Yamaha PSR-EW425 series";
            case DeviceType::ALLEN_PROTEGE:
                return "Allen Protege organ";
            case DeviceType::UNKNOWN:
            default:
                return "Unknown device";
        }
    }

    void DeviceManager::loadDevicePresets(const std::string& configPath) {
        // TODO: Future implementation for YAML configuration loading
        // This method will:
        // 1. Parse YAML configuration file
        // 2. Load device-specific presets (bank/program combinations)
        // 3. Allow custom device detection strings
        // 4. Enable user-configurable device setups without code changes
        
        std::cout << "Note: YAML configuration loading is not yet implemented." << std::endl;
        std::cout << "Configuration file path: " << configPath << std::endl;
    }

    DeviceType DeviceManager::detectDeviceType(const std::string& portName) {
        // Device detection logic extracted from play.cpp lines 459-476
        
        if (portName.find("CASIO USB") == 0) {
            // Casio USB MIDI device
            return DeviceType::CASIO_CTX3000;
        }
        else if (portName.find("Digital Keyboard") == 0) {
            // Yamaha PSR-EW425 series keyboard
            return DeviceType::YAMAHA_PSR_EW425;
        }
        else {
            // Default to Allen Protege organ for unrecognized devices
            // This maintains backward compatibility with the original logic
            // TODO: In future, could check for "USB MIDI Interface" specifically
            return DeviceType::ALLEN_PROTEGE;
        }
    }

    bool DeviceManager::waitForDeviceConnection(cxxmidi::output::Default& outport) {
        // Device connection logic extracted from play.cpp lines 432-453
        
        for (int i = 0; i <= MidiPlay::Device::CONNECTION_TIMEOUT; i++) {
            // Check if we've reached the timeout limit
            if (i > MidiPlay::Device::CONNECTION_TIMEOUT) {
                std::cout << "Device connection timeout. No device found. Connect a MIDI device and try again.\n" 
                         << std::endl;
                return false;  // Timeout reached
            }

            // Check current port count
            size_t portCount = outport.GetPortCount();

#if defined(DEBUG)
            // Debug output: show all available ports
            std::cout << "Available MIDI ports:" << std::endl;
            for (size_t j = 0; j < portCount; j++) {
                std::cout << j << ": " << outport.GetPortName(j) << std::endl;
            }
            std::cout << std::endl;
#endif

            // Check if we have sufficient ports for device connection
            if (portCount >= MidiPlay::Device::MIN_PORT_COUNT) {
                break;  // We have a device. Exit the polling loop.
            }
            else {
                // No device connected yet, continue polling
                std::cout << "No device connected. Connect a device." << std::endl;
                sleep(MidiPlay::Device::POLL_SLEEP_SECONDS);
                
                // Refresh port count for next iteration
                // Note: GetPortCount() will re-scan for available ports
            }
        }

        return true;  // Device connection successful
    }

} // namespace MidiPlay