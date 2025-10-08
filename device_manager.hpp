#pragma once

#include <cxxmidi/output/default.hpp>
#include <string>
#include <memory>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <optional>
#include <yaml-cpp/yaml.h>

#include "device_constants.hpp"

// Forward declaration
class Options;

namespace MidiPlay {

    /**
     * @brief Device key constants for YAML configuration
     */
    namespace DeviceKeys {
        constexpr const char* CASIO_CTX3000 = "casio_ctx3000";
        constexpr const char* YAMAHA_PSR_EW425 = "yamaha_psr_ew425";
        constexpr const char* ALLEN_PROTEGE = "allen_protege";
    }

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
         * @brief Constructor with Options dependency injection
         * @param options Reference to Options object for configuration and output control
         */
        DeviceManager(const Options& options);
        
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
         * @return String description of the device type (from YAML if loaded)
         */
        std::string getDeviceTypeName(DeviceType type);

        /**
         * @brief Load device configuration presets from YAML file
         *
         * Loads device configurations from YAML file. Searches standard locations
         * if no path provided: ~/.config/midiplay/midi_devices.yaml,
         * /etc/midiplay/midi_devices.yaml, ./midi_devices.yaml
         *
         * @param configPath Optional path to specific YAML configuration file
         * @throws std::runtime_error if YAML file not found or parsing fails
         */
        void loadDevicePresets(const std::string& configPath = "");

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

        // YAML configuration data structures
        struct ChannelConfig {
            std::uint8_t bank_msb = 0;
            std::uint8_t bank_lsb = 0;
            std::uint8_t program = 0;
            std::string description;
        };

        struct DeviceConfig {
            std::string name;
            std::string description;
            std::vector<std::string> detection_strings;
            std::map<int, ChannelConfig> channels;
        };

        struct ConnectionConfig {
            int timeout_iterations = MidiPlay::Device::CONNECTION_TIMEOUT;
            int poll_sleep_seconds = MidiPlay::Device::POLL_SLEEP_SECONDS;
            std::size_t min_port_count = MidiPlay::Device::MIN_PORT_COUNT;
            int output_port_index = MidiPlay::Device::OUTPUT_PORT_INDEX;
        };

        struct YamlConfig {
            std::string version;
            ConnectionConfig connection;
            std::map<std::string, DeviceConfig> devices;
        };

        // Options reference for configuration access
        const Options& options_;
        
        // YAML configuration state (using std::optional for clear semantics)
        std::optional<YamlConfig> yamlConfig_;

        // YAML parsing and file discovery methods
        std::string findConfigFile(const std::string& specifiedPath = "");
        void parseYamlFile(const std::string& filePath);
        void parseYamlContent(const YAML::Node& config);
        DeviceType detectDeviceTypeFromYaml(const std::string& portName);
        void configureDeviceFromYaml(const std::string& deviceKey, cxxmidi::output::Default& outport);
        
        // Helper methods for device key conversion
        std::string deviceTypeToKey(DeviceType type) const;
        DeviceType deviceKeyToType(const std::string& key) const;
    };

} // namespace MidiPlay