/**
 * Machine Features Helper
 *
 * Provides utility functions to determine what features and capabilities
 * are available for each machine type.
 */

import type { MachineType } from "./types";

// =============================================================================
// Feature Sets
// =============================================================================

export interface MachineFeatures {
  // Boiler configuration
  hasSeparateBoilers: boolean; // Two independently controlled boilers
  hasBrewBoiler: boolean; // Active brew boiler with heater
  hasSteamBoiler: boolean; // Active steam boiler with heater
  isHeatExchanger: boolean; // Passive heat exchange for brew water

  // Temperature control
  hasDualTemperatureControl: boolean; // Can set brew and steam temps independently
  needsModeSwitching: boolean; // Single boiler: switches between brew/steam mode
  hasGroupTempSensor: boolean; // HX machines: monitors group head temp

  // Power modes
  supportsPowerModes: boolean; // Can select between Brew Only / Brew & Steam

  // UI considerations
  showBrewGauge: boolean;
  showSteamGauge: boolean;
  brewGaugeLabel: string;
  steamGaugeLabel: string;
}

// =============================================================================
// Feature Definitions by Machine Type
// =============================================================================

const DUAL_BOILER_FEATURES: MachineFeatures = {
  hasSeparateBoilers: true,
  hasBrewBoiler: true,
  hasSteamBoiler: true,
  isHeatExchanger: false,
  hasDualTemperatureControl: true,
  needsModeSwitching: false,
  hasGroupTempSensor: false,
  supportsPowerModes: true,
  showBrewGauge: true,
  showSteamGauge: true,
  brewGaugeLabel: "Brew Boiler",
  steamGaugeLabel: "Steam Boiler",
};

const SINGLE_BOILER_FEATURES: MachineFeatures = {
  hasSeparateBoilers: false,
  hasBrewBoiler: true,
  hasSteamBoiler: false,
  isHeatExchanger: false,
  hasDualTemperatureControl: false,
  needsModeSwitching: true,
  hasGroupTempSensor: false,
  supportsPowerModes: false,
  showBrewGauge: true,
  showSteamGauge: false,
  brewGaugeLabel: "Boiler",
  steamGaugeLabel: "",
};

const HEAT_EXCHANGER_FEATURES: MachineFeatures = {
  hasSeparateBoilers: false,
  hasBrewBoiler: false,
  hasSteamBoiler: true,
  isHeatExchanger: true,
  hasDualTemperatureControl: false,
  needsModeSwitching: false,
  hasGroupTempSensor: true,
  supportsPowerModes: false,
  showBrewGauge: false,
  showSteamGauge: true,
  brewGaugeLabel: "",
  steamGaugeLabel: "Steam Boiler",
};

const UNKNOWN_FEATURES: MachineFeatures = {
  // Default to dual boiler behavior for unknown types
  hasSeparateBoilers: true,
  hasBrewBoiler: true,
  hasSteamBoiler: true,
  isHeatExchanger: false,
  hasDualTemperatureControl: true,
  needsModeSwitching: false,
  hasGroupTempSensor: false,
  supportsPowerModes: true,
  showBrewGauge: true,
  showSteamGauge: true,
  brewGaugeLabel: "Brew Boiler",
  steamGaugeLabel: "Steam Boiler",
};

// =============================================================================
// API Functions
// =============================================================================

/**
 * Get all features for a machine type
 */
export function getMachineFeatures(
  machineType: MachineType | undefined
): MachineFeatures {
  switch (machineType) {
    case "dual_boiler":
      return DUAL_BOILER_FEATURES;
    case "single_boiler":
      return SINGLE_BOILER_FEATURES;
    case "heat_exchanger":
      return HEAT_EXCHANGER_FEATURES;
    default:
      return UNKNOWN_FEATURES;
  }
}

/**
 * Check if machine type is dual boiler
 */
export function isDualBoiler(machineType: MachineType | undefined): boolean {
  return machineType === "dual_boiler";
}

/**
 * Check if machine type is single boiler
 */
export function isSingleBoiler(machineType: MachineType | undefined): boolean {
  return machineType === "single_boiler";
}

/**
 * Check if machine type is heat exchanger
 */
export function isHeatExchanger(machineType: MachineType | undefined): boolean {
  return machineType === "heat_exchanger";
}

/**
 * Check if machine supports power mode selection (Brew Only / Brew & Steam)
 */
export function supportsPowerModes(
  machineType: MachineType | undefined
): boolean {
  return getMachineFeatures(machineType).supportsPowerModes;
}

/**
 * Check if machine needs mode switching (single boiler only)
 */
export function needsModeSwitching(
  machineType: MachineType | undefined
): boolean {
  return getMachineFeatures(machineType).needsModeSwitching;
}

/**
 * Get human-readable machine type label
 */
export function getMachineTypeLabel(
  machineType: MachineType | undefined
): string {
  switch (machineType) {
    case "dual_boiler":
      return "Dual Boiler";
    case "single_boiler":
      return "Single Boiler";
    case "heat_exchanger":
      return "Heat Exchanger";
    default:
      return "Unknown";
  }
}

/**
 * Get machine type description
 */
export function getMachineTypeDescription(
  machineType: MachineType | undefined
): string {
  switch (machineType) {
    case "dual_boiler":
      return "Two independent boilers for simultaneous brewing and steaming";
    case "single_boiler":
      return "One boiler that switches between brew and steam temperatures";
    case "heat_exchanger":
      return "Steam boiler with passive heat exchange for brew water";
    default:
      return "Machine type not configured";
  }
}

/**
 * Get eco mode description based on machine type
 */
export function getEcoModeDescription(
  machineType: MachineType | undefined
): string {
  switch (machineType) {
    case "dual_boiler":
      return "Reduce power consumption by lowering both boiler temperatures when idle.";
    case "single_boiler":
      return "Reduce power consumption by lowering the boiler temperature when idle.";
    case "heat_exchanger":
      return "Reduce power consumption by lowering the steam boiler temperature when idle.";
    default:
      return "Reduce power consumption when idle by lowering boiler temperatures.";
  }
}

/**
 * Get temperature setting labels based on machine type
 */
export function getTemperatureLabels(machineType: MachineType | undefined): {
  brew: string;
  steam: string;
  brewHint: string;
  steamHint: string;
} {
  const features = getMachineFeatures(machineType);

  if (features.needsModeSwitching) {
    // Single boiler
    return {
      brew: "Brew Temperature",
      steam: "Steam Temperature",
      brewHint: "Target temperature for brewing espresso",
      steamHint: "Target temperature when steaming milk",
    };
  }

  if (features.isHeatExchanger) {
    // Heat exchanger
    return {
      brew: "Group Temperature",
      steam: "Boiler Temperature",
      brewHint: "Monitored via group head sensor",
      steamHint: "Controls HX brew water temperature",
    };
  }

  // Dual boiler or unknown
  return {
    brew: "Brew Boiler",
    steam: "Steam Boiler",
    brewHint: "Recommended: 91-96°C for espresso",
    steamHint: "For milk frothing",
  };
}
