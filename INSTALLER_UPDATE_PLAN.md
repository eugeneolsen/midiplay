# Installer Update Plan for DeviceManager + YAML Configuration

## Overview
The DeviceManager refactoring with YAML configuration support requires significant installer updates to handle new dependencies, configuration files, and system integration.

## Changes Summary
- **Version Bump**: 1.4.7 → 1.5.0 (breaking changes due to new dependencies)
- **New Dependency**: libyaml-cpp0.6 runtime library required
- **Configuration**: System-wide YAML configuration in /etc/midiplay/
- **Backwards Compatibility**: None - YAML configuration is mandatory

## Required Updates

### 1. DEBIAN/control - Add Dependencies
**File**: `midiplay-installer/debian-package/DEBIAN/control`
**Changes**:
```
Version: 1.4.7 → 1.5.0
Depends: libasound2 (>= 1.0.16), libc6 (>= 2.17), libgcc-s1 (>= 3.0), libstdc++6 (>= 5.2), libyaml-cpp0.6
```

### 2. Package Structure - Add Configuration Directory
**New Structure**:
```
debian-package/
├── usr/local/bin/play                    # Existing binary
└── etc/midiplay/                         # NEW: System config directory
    └── midi_devices.yaml                 # NEW: Default configuration
```

### 3. postinst Script Updates  
**File**: `midiplay-installer/debian-package/DEBIAN/postinst`
**Add After Line 24** (after symlink creation):
```bash
# Create system configuration directory
if [ ! -d /etc/midiplay ]; then
    mkdir -p /etc/midiplay
    chmod 755 /etc/midiplay
fi

# Install default YAML configuration if not present
if [ ! -f /etc/midiplay/midi_devices.yaml ]; then
    cp /etc/midiplay/midi_devices.yaml.dist /etc/midiplay/midi_devices.yaml
    chmod 644 /etc/midiplay/midi_devices.yaml
    echo "✅ Default MIDI device configuration installed to /etc/midiplay/midi_devices.yaml"
else
    echo "ℹ️  Existing MIDI device configuration preserved"
fi

echo "📁 Configuration directory: /etc/midiplay/"
echo "   System config: /etc/midiplay/midi_devices.yaml"
echo "   User config:   ~/.config/midiplay/midi_devices.yaml (optional override)"
```

### 4. install.sh Updates
**File**: `midiplay-installer/install.sh`
**Changes**:
- Line 16: `PACKAGE_VERSION="1.4.7"` → `PACKAGE_VERSION="1.5.0"`
- Line 124: Update version reference in summary
- Add dependency check for libyaml-cpp before installation

**New Dependency Check** (Add after line 62):
```bash
# Check if yaml-cpp is available
if ! dpkg -l | grep -q libyaml-cpp; then
    print_warning "libyaml-cpp0.6 not found. This will be installed as a dependency."
fi
```

### 5. create-installer-archive.sh Updates
**File**: `create-installer-archive.sh`
**Changes**:
- Line 12: `VERSION="1.4.7"` → `VERSION="1.5.0"`
- Update required files list to include YAML configuration

**Add to required_files array**:
```bash
"midiplay-installer/debian-package/etc/midiplay/midi_devices.yaml"
```

### 6. Installation Verification Updates
**File**: `midiplay-installer/install.sh`
**Update Installation Summary** (Line 122):
```bash
echo -e "${BLUE}Installation Summary:${NC}"
echo "• Binary installed to: /usr/local/bin/play"
echo "• Symlink created: /usr/local/bin/p -> /usr/local/bin/play"
echo "• System config: /etc/midiplay/midi_devices.yaml"
echo "• User config: ~/.config/midiplay/midi_devices.yaml (optional)"
echo "• Version: $(play --version 2>/dev/null || echo '1.5.0')"
```

### 7. Documentation Updates
**Files to Update**:
- `midiplay-installer/README.md` - Add configuration information
- `INSTALLER-SUMMARY.md` - Update for v1.5.0 changes

## Implementation Sequence

### Phase 1: Core Package Updates
1. ✅ **Update DEBIAN/control** - Version + dependencies
2. ✅ **Create package structure** - Add /etc/midiplay/ directory
3. ✅ **Copy YAML configuration** - Install midi_devices.yaml to package

### Phase 2: Script Updates  
4. ✅ **Update postinst script** - Configuration setup logic
5. ✅ **Update install.sh** - Version numbers and checks
6. ✅ **Update create-installer-archive.sh** - Version and file verification

### Phase 3: Testing & Documentation
7. ✅ **Test complete installation cycle** - Fresh install + upgrade
8. ✅ **Update documentation** - README and installation guides
9. ✅ **Verify uninstall process** - Configuration cleanup

## Risk Assessment

### High Risk
- **Dependency Availability**: libyaml-cpp0.6 may not be available on all systems
- **Breaking Change**: Existing installations won't work without YAML config
- **Permission Issues**: /etc/midiplay/ creation requires root access

### Mitigation Strategies
- **Dependency Check**: Verify libyaml-cpp availability before installation
- **Graceful Degradation**: Provide clear error messages for missing dependencies
- **Configuration Backup**: Preserve existing configurations during upgrades

## Testing Checklist

### Fresh Installation Test
- [ ] Clean system installation with dependencies
- [ ] YAML configuration file created in /etc/midiplay/
- [ ] Binary runs with system configuration
- [ ] User override with ~/.config/midiplay/midi_devices.yaml works

### Upgrade Test  
- [ ] Upgrade from v1.4.7 preserves existing functionality
- [ ] New YAML configuration installed without conflicts
- [ ] Binary compatibility maintained

### Uninstall Test
- [ ] Complete removal of binaries and configurations
- [ ] User configurations preserved or removed based on choice

## Success Criteria
1. ✅ **Zero Installation Failures** on supported systems
2. ✅ **Configuration Auto-Discovery** works in priority order
3. ✅ **Dependency Resolution** handled automatically by package manager
4. ✅ **Documentation Complete** for new configuration system
5. ✅ **Backwards Compatibility** - clear migration path documented

---

## Next Steps
1. **Switch to Code Mode** to implement these changes
2. **Create updated package structure** with YAML configuration
3. **Test installation process** on clean system
4. **Validate upgrade process** from existing installation

**Ready for Implementation**: All planning complete, detailed changes specified.