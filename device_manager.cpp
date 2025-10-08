#include "device_manager.hpp"
#include "constants.hpp"
#include "options.hpp"
#include "i18n.hpp"

#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <pwd.h>
#include <sys/types.h>

using cxxmidi::Event;
using cxxmidi::Message;

namespace MidiPlay {

    // Helper method implementations
    std::string DeviceManager::deviceTypeToKey(DeviceType type) const {
        switch (type) {
            case DeviceType::CASIO_CTX3000:
                return DeviceKeys::CASIO_CTX3000;
            case DeviceType::YAMAHA_PSR_EW425:
                return DeviceKeys::YAMAHA_PSR_EW425;
            case DeviceType::ALLEN_PROTEGE:
                return DeviceKeys::ALLEN_PROTEGE;
            case DeviceType::UNKNOWN:
            default:
                throw std::invalid_argument(_("Cannot convert unknown device type to key"));
        }
    }

    DeviceType DeviceManager::deviceKeyToType(const std::string& key) const {
        if (key == DeviceKeys::CASIO_CTX3000) {
            return DeviceType::CASIO_CTX3000;
        } else if (key == DeviceKeys::YAMAHA_PSR_EW425) {
            return DeviceType::YAMAHA_PSR_EW425;
        } else if (key == DeviceKeys::ALLEN_PROTEGE) {
            return DeviceType::ALLEN_PROTEGE;
        }
        return DeviceType::UNKNOWN;
    }

    DeviceManager::DeviceManager(const Options& options)
        : options_(options)
        , yamlConfig_()  // Initialize as empty optional
    {
    }

    DeviceInfo DeviceManager::connectAndDetectDevice(cxxmidi::output::Default& outport) {
        // Wait for device connection with timeout
        if (!waitForDeviceConnection(outport)) {
            throw std::runtime_error(_("Device connection timeout. No device found. Connect a MIDI device and try again."));
        }

        // Use YAML configuration for port index if loaded
        int outputPortIndex = yamlConfig_.has_value() ? yamlConfig_->connection.output_port_index : MidiPlay::Device::OUTPUT_PORT_INDEX;
        
        // Open the MIDI output port
        outport.OpenPort(outputPortIndex);

        // Get port information and detect device type
        std::string portName = outport.GetPortName(outputPortIndex);
        DeviceType type = detectDeviceType(portName);

        return DeviceInfo(type, portName, outputPortIndex);
    }

    void DeviceManager::createAndConfigureDevice(DeviceType type, cxxmidi::output::Default& outport) {
        // YAML configuration is now mandatory
        if (!yamlConfig_.has_value()) {
            throw std::runtime_error(_("YAML configuration is required. No device configuration found. "
                                   "Please ensure midi_devices.yaml is available in a standard location:\n"
                                   "  ~/.config/midiplay/midi_devices.yaml (user-specific)\n"
                                   "  /etc/midiplay/midi_devices.yaml (system-wide)\n"
                                   "  ./midi_devices.yaml (local)"));
        }

        // Configure device using YAML configuration - use helper method
        std::string deviceKey = deviceTypeToKey(type);
        configureDeviceFromYaml(deviceKey, outport);
    }

    std::string DeviceManager::getDeviceTypeName(DeviceType type) {
        // If YAML is loaded, get device name from configuration
        if (yamlConfig_.has_value() && type != DeviceType::UNKNOWN) {
            std::string deviceKey = deviceTypeToKey(type);
            
            // Look up device name in YAML configuration
            auto deviceIt = yamlConfig_->devices.find(deviceKey);
            if (deviceIt != yamlConfig_->devices.end() && !deviceIt->second.name.empty()) {
                return deviceIt->second.name;
            }
        }
        
        // Fallback names (should rarely be used since YAML is mandatory)
        switch (type) {
            case DeviceType::CASIO_CTX3000:
                return _("Casio CTX-3000 series");
            case DeviceType::YAMAHA_PSR_EW425:
                return _("Yamaha PSR-EW425 series");
            case DeviceType::ALLEN_PROTEGE:
                return _("Allen Protege organ");
            case DeviceType::UNKNOWN:
            default:
                return _("Unknown device");
        }
    }

    void DeviceManager::loadDevicePresets(const std::string& configPath) {
        std::string yamlPath = findConfigFile(configPath);
        
        if (yamlPath.empty()) {
            throw std::runtime_error(_("YAML configuration file not found. Device configuration is mandatory. "
                                   "Please create midi_devices.yaml in one of these locations:\n"
                                   "  ~/.config/midiplay/midi_devices.yaml (user-specific)\n"
                                   "  /etc/midiplay/midi_devices.yaml (system-wide)\n"
                                   "  ./midi_devices.yaml (local)"));
        }

        parseYamlFile(yamlPath);
        // yamlConfig_ is now set by parseYamlContent
        
        if (options_.isVerbose()) {
            std::cout << _("Loaded device configuration from: ") << yamlPath << std::endl;
        }
    }

    std::string DeviceManager::findConfigFile(const std::string& specifiedPath) {
        // If specific path provided, use it
        if (!specifiedPath.empty()) {
            if (std::filesystem::exists(specifiedPath)) {
                return specifiedPath;
            }
            return "";
        }

        // Search standard locations in priority order
        std::vector<std::string> searchPaths;
        
        // 1. User-specific config: ~/.config/midiplay/midi_devices.yaml
        const char* homeDir = getenv("HOME");
        if (!homeDir) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) homeDir = pw->pw_dir;
        }
        if (homeDir) {
            searchPaths.push_back(std::string(homeDir) + "/.config/midiplay/midi_devices.yaml");
        }
        
        // 2. System-wide config: /etc/midiplay/midi_devices.yaml
        searchPaths.push_back("/etc/midiplay/midi_devices.yaml");
        
        // 3. Local development: ./midi_devices.yaml
        searchPaths.push_back("./midi_devices.yaml");

        // Find first existing file
        for (const auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                return path;
            }
        }

        return "";  // No config file found
    }

    void DeviceManager::parseYamlFile(const std::string& filePath) {
        try {
            YAML::Node config = YAML::LoadFile(filePath);
            parseYamlContent(config);
        } catch (const YAML::Exception& e) {
            throw std::runtime_error(_("YAML parsing error: ") + std::string(e.what()));
        } catch (const std::exception& e) {
            throw std::runtime_error(_("Error loading YAML file: ") + std::string(e.what()));
        }
    }

    void DeviceManager::parseYamlContent(const YAML::Node& config) {
        try {
            // Create a new YamlConfig to populate
            YamlConfig newConfig;
            
            // Parse version
            if (config["version"]) {
                newConfig.version = config["version"].as<std::string>();
            }

            // Parse connection settings
            if (config["connection"]) {
                const YAML::Node& conn = config["connection"];
                if (conn["timeout_iterations"]) {
                    newConfig.connection.timeout_iterations = conn["timeout_iterations"].as<int>();
                }
                if (conn["poll_sleep_seconds"]) {
                    newConfig.connection.poll_sleep_seconds = conn["poll_sleep_seconds"].as<int>();
                }
                if (conn["min_port_count"]) {
                    newConfig.connection.min_port_count = conn["min_port_count"].as<std::size_t>();
                }
                if (conn["output_port_index"]) {
                    newConfig.connection.output_port_index = conn["output_port_index"].as<int>();
                }
            }

            // Parse device configurations
            if (config["devices"]) {
                const YAML::Node& devices = config["devices"];
                for (const auto& devicePair : devices) {
                    std::string deviceKey = devicePair.first.as<std::string>();
                    const YAML::Node& deviceNode = devicePair.second;
                    
                    DeviceConfig deviceConfig;
                    
                    // Parse device properties
                    if (deviceNode["name"]) {
                        deviceConfig.name = deviceNode["name"].as<std::string>();
                    }
                    if (deviceNode["description"]) {
                        deviceConfig.description = deviceNode["description"].as<std::string>();
                    }
                    
                    // Parse detection strings
                    if (deviceNode["detection_strings"]) {
                        const YAML::Node& detectionStrings = deviceNode["detection_strings"];
                        for (const auto& detStr : detectionStrings) {
                            deviceConfig.detection_strings.push_back(detStr.as<std::string>());
                        }
                    }
                    
                    // Parse channels
                    if (deviceNode["channels"]) {
                        const YAML::Node& channels = deviceNode["channels"];
                        for (const auto& channelPair : channels) {
                            int channelNum = channelPair.first.as<int>();
                            const YAML::Node& channelNode = channelPair.second;
                            
                            ChannelConfig channelConfig;
                            if (channelNode["bank_msb"]) {
                                channelConfig.bank_msb = channelNode["bank_msb"].as<std::uint8_t>();
                            }
                            if (channelNode["bank_lsb"]) {
                                channelConfig.bank_lsb = channelNode["bank_lsb"].as<std::uint8_t>();
                            }
                            if (channelNode["program"]) {
                                channelConfig.program = channelNode["program"].as<std::uint8_t>();
                            }
                            if (channelNode["description"]) {
                                channelConfig.description = channelNode["description"].as<std::string>();
                            }
                            
                            deviceConfig.channels[channelNum] = channelConfig;
                        }
                    }
                    
                    newConfig.devices[deviceKey] = deviceConfig;
                }
            }
            
            // Assign the fully-populated config to the optional
            yamlConfig_ = std::move(newConfig);
        } catch (const YAML::Exception& e) {
            throw std::runtime_error(_("YAML content parsing error: ") + std::string(e.what()));
        } catch (const std::exception& e) {
            throw std::runtime_error(_("Error parsing YAML content: ") + std::string(e.what()));
        }
    }

    DeviceType DeviceManager::detectDeviceType(const std::string& portName) {
        // YAML configuration is now mandatory for device detection
        if (!yamlConfig_.has_value()) {
            throw std::runtime_error(_("YAML configuration is required for device detection. "
                                   "Cannot detect device type without configuration file."));
        }
        
        return detectDeviceTypeFromYaml(portName);
    }

    DeviceType DeviceManager::detectDeviceTypeFromYaml(const std::string& portName) {
        // Search through YAML device configurations for matching detection strings
        for (const auto& [deviceKey, deviceConfig] : yamlConfig_->devices) {
            for (const auto& detectionString : deviceConfig.detection_strings) {
                if (!detectionString.empty() && portName.find(detectionString) == 0) {
                    // Use helper method to map device key to DeviceType enum
                    return deviceKeyToType(deviceKey);
                }
            }
        }
        
        // If no match found, use fallback device (usually allen_protege)
        for (const auto& [deviceKey, deviceConfig] : yamlConfig_->devices) {
            if (deviceConfig.detection_strings.empty()) {
                if (deviceKey == DeviceKeys::ALLEN_PROTEGE) {
                    return DeviceType::ALLEN_PROTEGE;
                }
            }
        }
        
        // Final fallback to unknown
        return DeviceType::UNKNOWN;
    }

    void DeviceManager::configureDeviceFromYaml(const std::string& deviceKey, cxxmidi::output::Default& outport) {
        auto deviceIt = yamlConfig_->devices.find(deviceKey);
        if (deviceIt == yamlConfig_->devices.end()) {
            return; // Device configuration not found
        }

        const DeviceConfig& config = deviceIt->second;
        
        // Configure each channel according to YAML specification
        for (const auto& [channelNum, channelConfig] : config.channels) {
            // Convert 1-based channel numbers to MIDI channel messages
            std::uint8_t midiChannel = static_cast<std::uint8_t>(channelNum - 1);
            
            // Send Bank Select MSB
            if (channelConfig.bank_msb != 0) {
                Event bankMsbEvent(0, midiChannel | Message::kControlChange,
                                  0, channelConfig.bank_msb);
                outport.SendMessage(&bankMsbEvent);
            }
            
            // Send Bank Select LSB
            if (channelConfig.bank_lsb != 0) {
                Event bankLsbEvent(0, midiChannel | Message::kControlChange,
                                  32, channelConfig.bank_lsb);
                outport.SendMessage(&bankLsbEvent);
            }
            
            // Send Program Change
            Event programEvent(0, midiChannel | Message::kProgramChange,
                              channelConfig.program);
            outport.SendMessage(&programEvent);
            
            if (options_.isVerbose()) {
                std::cout << _("  Channel ") << channelNum << ": " << channelConfig.description
                          << _(" (Bank ") << static_cast<int>(channelConfig.bank_msb) << ":"
                          << static_cast<int>(channelConfig.bank_lsb) << _(", Program ")
                          << static_cast<int>(channelConfig.program) << ")" << std::endl;
            }
        }
    }

    bool DeviceManager::waitForDeviceConnection(cxxmidi::output::Default& outport) {
        // Use YAML configuration values if loaded, otherwise use defaults
        int timeoutLimit = yamlConfig_.has_value() ? yamlConfig_->connection.timeout_iterations : MidiPlay::Device::CONNECTION_TIMEOUT;
        int pollSleep = yamlConfig_.has_value() ? yamlConfig_->connection.poll_sleep_seconds : MidiPlay::Device::POLL_SLEEP_SECONDS;
        std::size_t minPortCount = yamlConfig_.has_value() ? yamlConfig_->connection.min_port_count : MidiPlay::Device::MIN_PORT_COUNT;
        
        for (int i = 0; i <= timeoutLimit; i++) {
            // Check if we've reached the timeout limit
            if (i > timeoutLimit) {
                std::cout << _("Device connection timeout. No device found. Connect a MIDI device and try again.\n")
                         << std::endl;
                return false;  // Timeout reached
            }

            // Check current port count
            size_t portCount = outport.GetPortCount();

#if defined(DEBUG)
            // Debug output: show all available ports
            std::cout << _("Available MIDI ports:") << std::endl;
            for (size_t j = 0; j < portCount; j++) {
                std::cout << j << ": " << outport.GetPortName(j) << std::endl;
            }
            std::cout << std::endl;
#endif

            // Check if we have sufficient ports for device connection
            if (portCount >= minPortCount) {
                break;  // We have a device. Exit the polling loop.
            }
            else {
                // No device connected yet, continue polling
                std::cout << _("No device connected. Connect a device.") << std::endl;
                sleep(pollSleep);
                
                // Refresh port count for next iteration
                // Note: GetPortCount() will re-scan for available ports
            }
        }

        return true;  // Device connection successful
    }

} // namespace MidiPlay