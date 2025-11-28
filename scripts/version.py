#!/usr/bin/env python3
"""
ECM Version Management Script

Manages firmware and protocol versions from a single source of truth.
Updates version definitions in all firmware projects.

Usage:
    python scripts/version.py                    # Show current version
    python scripts/version.py --bump patch      # Bump patch version
    python scripts/version.py --bump minor      # Bump minor version
    python scripts/version.py --bump major      # Bump major version
    python scripts/version.py --set 1.2.3       # Set specific version
    python scripts/version.py --protocol 2     # Set protocol version
"""

import re
import sys
import os
from pathlib import Path

# Project root (parent of scripts/)
PROJECT_ROOT = Path(__file__).parent.parent
VERSION_FILE = PROJECT_ROOT / "VERSION"

# Files to update
PICO_CONFIG = PROJECT_ROOT / "src/pico/include/config.h"
ESP32_CONFIG = PROJECT_ROOT / "src/esp32/include/config.h"
PROTOCOL_DEFS = PROJECT_ROOT / "src/shared/protocol_defs.h"


def read_version_file():
    """Read version from VERSION file."""
    if not VERSION_FILE.exists():
        raise FileNotFoundError(f"VERSION file not found at {VERSION_FILE}")
    
    firmware_version = None
    protocol_version = None
    
    with open(VERSION_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('FIRMWARE_VERSION='):
                firmware_version = line.split('=', 1)[1].strip()
            elif line.startswith('PROTOCOL_VERSION='):
                protocol_version = line.split('=', 1)[1].strip()
    
    if not firmware_version or not protocol_version:
        raise ValueError("VERSION file missing FIRMWARE_VERSION or PROTOCOL_VERSION")
    
    return firmware_version, int(protocol_version)


def write_version_file(firmware_version, protocol_version):
    """Write version to VERSION file."""
    content = f"""# ECM Firmware Version
# Format: MAJOR.MINOR.PATCH
# Follow semantic versioning: https://semver.org/
#
# MAJOR: Breaking changes (protocol version changes, incompatible API)
# MINOR: New features (backward compatible)
# PATCH: Bug fixes (backward compatible)
#
# Protocol version is tracked separately in src/shared/protocol_defs.h
# Increment ECM_PROTOCOL_VERSION for breaking protocol changes

FIRMWARE_VERSION={firmware_version}
PROTOCOL_VERSION={protocol_version}
"""
    with open(VERSION_FILE, 'w') as f:
        f.write(content)


def parse_version(version_str):
    """Parse version string into (major, minor, patch)."""
    match = re.match(r'^(\d+)\.(\d+)\.(\d+)$', version_str)
    if not match:
        raise ValueError(f"Invalid version format: {version_str}. Expected MAJOR.MINOR.PATCH")
    return tuple(int(x) for x in match.groups())


def format_version(major, minor, patch):
    """Format version tuple into string."""
    return f"{major}.{minor}.{patch}"


def bump_version(version_str, bump_type):
    """Bump version by type (major, minor, patch)."""
    major, minor, patch = parse_version(version_str)
    
    if bump_type == 'major':
        return format_version(major + 1, 0, 0)
    elif bump_type == 'minor':
        return format_version(major, minor + 1, 0)
    elif bump_type == 'patch':
        return format_version(major, minor, patch + 1)
    else:
        raise ValueError(f"Invalid bump type: {bump_type}. Use major, minor, or patch")


def update_pico_config(version_str):
    """Update Pico config.h with version."""
    major, minor, patch = parse_version(version_str)
    
    with open(PICO_CONFIG, 'r') as f:
        content = f.read()
    
    # Replace version defines (order matters - do PATCH before MINOR to avoid conflicts)
    content = re.sub(
        r'#define FIRMWARE_VERSION_MAJOR\s+\d+',
        f'#define FIRMWARE_VERSION_MAJOR      {major}',
        content
    )
    content = re.sub(
        r'#define FIRMWARE_VERSION_PATCH\s+\d+',
        f'#define FIRMWARE_VERSION_PATCH      {patch}',
        content
    )
    content = re.sub(
        r'#define FIRMWARE_VERSION_MINOR\s+\d+',
        f'#define FIRMWARE_VERSION_MINOR      {minor}',
        content
    )
    
    with open(PICO_CONFIG, 'w') as f:
        f.write(content)
    
    print(f"✓ Updated {PICO_CONFIG}")


def update_esp32_config(version_str):
    """Update ESP32 config.h with version."""
    major, minor, patch = parse_version(version_str)
    
    with open(ESP32_CONFIG, 'r') as f:
        content = f.read()
    
    # Replace version defines
    content = re.sub(
        r'#define ESP32_VERSION_MAJOR\s+\d+',
        f'#define ESP32_VERSION_MAJOR     {major}',
        content
    )
    content = re.sub(
        r'#define ESP32_VERSION_MINOR\s+\d+',
        f'#define ESP32_VERSION_MINOR     {minor}',
        content
    )
    content = re.sub(
        r'#define ESP32_VERSION_PATCH\s+\d+',
        f'#define ESP32_VERSION_PATCH     {patch}',
        content
    )
    content = re.sub(
        r'#define ESP32_VERSION\s+"[^"]+"',
        f'#define ESP32_VERSION           "{version_str}"',
        content
    )
    
    with open(ESP32_CONFIG, 'w') as f:
        f.write(content)
    
    print(f"✓ Updated {ESP32_CONFIG}")


def update_protocol_defs(protocol_version):
    """Update protocol_defs.h with protocol version."""
    with open(PROTOCOL_DEFS, 'r') as f:
        content = f.read()
    
    content = re.sub(
        r'#define ECM_PROTOCOL_VERSION\s+\d+',
        f'#define ECM_PROTOCOL_VERSION    {protocol_version}',
        content
    )
    
    with open(PROTOCOL_DEFS, 'w') as f:
        f.write(content)
    
    print(f"✓ Updated {PROTOCOL_DEFS}")


def update_all_versions(firmware_version, protocol_version):
    """Update all version files."""
    print(f"Updating versions: firmware={firmware_version}, protocol={protocol_version}")
    print()
    
    update_pico_config(firmware_version)
    update_esp32_config(firmware_version)
    update_protocol_defs(protocol_version)
    write_version_file(firmware_version, protocol_version)
    
    print()
    print(f"✓ All versions updated successfully!")


def main():
    import argparse
    
    parser = argparse.ArgumentParser(
        description='ECM Version Management',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument('--bump', choices=['major', 'minor', 'patch'],
                       help='Bump version (major, minor, or patch)')
    parser.add_argument('--set', metavar='VERSION',
                       help='Set specific version (e.g., 1.2.3)')
    parser.add_argument('--protocol', type=int,
                       help='Set protocol version')
    parser.add_argument('--show', action='store_true',
                       help='Show current versions (default)')
    
    args = parser.parse_args()
    
    # Read current versions
    try:
        firmware_version, protocol_version = read_version_file()
    except FileNotFoundError:
        print(f"Error: VERSION file not found. Creating default...")
        write_version_file("0.1.0", 1)
        firmware_version, protocol_version = read_version_file()
    
    # Handle commands
    if args.set:
        try:
            parse_version(args.set)  # Validate format
            firmware_version = args.set
        except ValueError as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    
    if args.bump:
        firmware_version = bump_version(firmware_version, args.bump)
        print(f"Bumped {args.bump} version to {firmware_version}")
    
    if args.protocol is not None:
        protocol_version = args.protocol
        print(f"Setting protocol version to {protocol_version}")
    
    # Update files if version changed
    if args.set or args.bump or args.protocol is not None:
        update_all_versions(firmware_version, protocol_version)
    else:
        # Just show current versions
        print(f"Firmware Version: {firmware_version}")
        print(f"Protocol Version: {protocol_version}")
        major, minor, patch = parse_version(firmware_version)
        print()
        print("Current version definitions:")
        print(f"  Pico:     {major}.{minor}.{patch}")
        print(f"  ESP32:    {major}.{minor}.{patch}")
        print(f"  Protocol: {protocol_version}")


if __name__ == '__main__':
    main()

