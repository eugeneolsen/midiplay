#pragma once

#include <cxxmidi/output/default.hpp>
#include <string>
#include <memory>
#include <iostream>

#include "device_constants.hpp"

// Forward declarations for device classes
class ctx3000;
class psr_ew425;
class protege;

namespace MidiPlay {

    /**
     * @brief Enumeration of supported MIDI device types
     */
    enum class DeviceType {
        CASIO_CTX3000,      ///< Casio CTX-3000 series keyboards
        YAMAHA_PSR_EW425,   ///< Yamaha PSR-EW425 series keyboards  
        ALLEN_PROTEGE,      ///< Allen Protege organs
        UNKNOWN             ///< Unknown or unsupported device
    };

    /**
     * @brief Information about a detected MIDI device
     */
    struct DeviceInfo {
        DeviceType type;        ///< Detected device type
        std::string portName;   ///< MIDI port name as reported by system
        int portIndex;          ///< Zero-based port index for connection
        
        DeviceInfo() : type(DeviceType::UNKNOWN), portIndex(-1) {}
        DeviceInfo(DeviceType t, const std::string& name, int index) 
            : type(t), portName(name), portIndex(index) {}
    };

    /**
     * @brief Base interface for all MIDI device implementations
     * 
     * This interface provides a common contract for all supported MIDI devices.
     * Existing device classes (ctx3000, psr_ew425, protege) will eventually
     * inherit from this interface to enable polymorphic usage.
     */
    class BaseDevice {
    public:
        virtual ~BaseDevice() = default;
        
        /**
         * @brief Configure device with default settings
         * 
         * This method should set up the device with appropriate
         * bank selections, program changes, and volume settings
         * for optimal hymn playback.
         */
        virtual void SetDefaults() = 0;
        
        /**
         * @brief Get the device type
         * @return DeviceType enumeration value
         */
        virtual DeviceType GetDeviceType() const = 0;
    };

    /**
     * @brief Manager class for MIDI device detection, connection, and creation
     * 
     * The DeviceManager encapsulates all device-related operations including:
     * - Device connection with timeout handling
     * - Device type detection based on port names
     * - Factory-based device creation
     * - Future YAML configuration support
     * 
     * This class extracts device management logic from the main application
     * to improve modularity and prepare for configuration-driven device setup.
     */
    class DeviceManager {
    public:
        /**
         * @brief Default constructor
         */
        DeviceManager() = default;
        
        /**
         * @brief Destructor
         */
        ~DeviceManager() = default;

        /**
         * @brief Connect to a MIDI device and detect its type
         * 
         * This method handles the complete device connection process:
         * 1. Wait for device connection with timeout
         * 2. Open the MIDI output port
         * 3. Detect device type based on port name
         * 
         * @param outport Reference to the MIDI output port
         * @return DeviceInfo containing device type and connection details
         * @throws std::runtime_error if device connection times out
         */
        DeviceInfo connectAndDetectDevice(cxxmidi::output::Default& outport);

        /**
         * @brief Create and configure a device instance based on detected type
         *
         * Factory method that instantiates the appropriate device class
         * based on the detected device type and immediately configures it
         * with default settings. This combines device creation and configuration
         * into a single step for simplicity.
         *
         * @param type The detected device type
         * @param outport Reference to the connected MIDI output port
         * @throws std::invalid_argument for unsupported device types
         */
        void createAndConfigureDevice(DeviceType type, cxxmidi::output::Default& outport);

        /**
         * @brief Get human-readable name for device type
         * @param type The device type
         * @return String description of the device type
         */
        static std::string getDeviceTypeName(DeviceType type);

        /**
         * @brief Load device configuration presets from YAML file
         * 
         * Future implementation will support loading device configurations
         * from external YAML files to enable customization without code changes.
         * 
         * @param configPath Path to the YAML configuration file
         * @note This method is planned for future implementation
         */
        void loadDevicePresets(const std::string& configPath);

    private:
        /**
         * @brief Detect device type based on MIDI port name
         * 
         * Analyzes the port name string to determine which type of
         * MIDI device is connected. Uses string matching against
         * known device identifier patterns.
         * 
         * @param portName The MIDI port name as reported by the system
         * @return DeviceType enumeration value
         */
        DeviceType detectDeviceType(const std::string& portName);

        /**
         * @brief Wait for MIDI device connection with timeout
         * 
         * Polls for device availability until either a device is found
         * or the connection timeout is reached. Provides user feedback
         * during the waiting process.
         * 
         * @param outport Reference to the MIDI output port
         * @return true if device connected successfully, false on timeout
         */
        bool waitForDeviceConnection(cxxmidi::output::Default& outport);

        // Device configuration structures for future YAML support
        struct DevicePreset {
            std::string detectionString;
            // Future: channel configurations, bank/program mappings
        };
    };

} // namespace MidiPlay