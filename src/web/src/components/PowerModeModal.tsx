import { useState, useEffect, useMemo, useCallback } from "react";
import { X, Flame, Sparkles, Check, Info } from "lucide-react";
import { cn } from "@/lib/utils";
import { useStore } from "@/lib/store";
import { useCommand } from "@/lib/useCommand";
import { getMachineByBrandModel } from "@/lib/machines";
import {
  POWER_MODES,
  POWER_MODE_DEFINITIONS,
  getHeatingStrategyForPowerMode,
  getAutoSelectedStrategyDescription,
  type PowerMode,
} from "@/lib/powerValidation";

const STORAGE_KEY = "brewos-last-power-mode";

interface PowerModeModalProps {
  isOpen: boolean;
  onClose: () => void;
  onSelect: (strategy: number) => void;
  defaultMode?: PowerMode;
}

export function PowerModeModal({
  isOpen,
  onClose,
  onSelect,
  defaultMode,
}: PowerModeModalProps) {
  const { sendCommand } = useCommand();

  // Get power settings and machine info from store
  const voltage = useStore((s) => s.power.voltage) || 220;
  const maxCurrent = useStore((s) => s.power.maxCurrent) || 13;
  const machineBrand = useStore((s) => s.device.machineBrand);
  const machineModel = useStore((s) => s.device.machineModel);

  // Get machine definition by brand and model
  const machine = useMemo(() => {
    if (!machineBrand || !machineModel) return undefined;
    return getMachineByBrandModel(machineBrand, machineModel);
  }, [machineBrand, machineModel]);

  const powerConfig = useMemo(
    () => ({ voltage, maxCurrent }),
    [voltage, maxCurrent]
  );

  // Initialize with correct mode using function initializer
  // This runs synchronously on mount, so no flicker
  // Only show selection when machine is on (defaultMode provided)
  // When machine is off, no selection is shown
  const [selectedMode, setSelectedMode] = useState<PowerMode | null>(() => {
    // If machine is on, use the current active mode
    if (defaultMode !== undefined) {
      return defaultMode;
    }
    // If machine is off, no selection
    return null;
  });

  // Memoize handleConfirm to avoid recreating on every render
  const handleConfirm = useCallback(
    (mode: PowerMode) => {
      // Calculate the actual heating strategy based on power mode and config
      const strategy = getHeatingStrategyForPowerMode(
        mode,
        machine,
        powerConfig
      );

      // Save mode preference
      localStorage.setItem(STORAGE_KEY, mode);
      sendCommand("set_preferences", { lastPowerMode: mode });

      // Send the calculated strategy to the machine
      onSelect(strategy);
      onClose();
    },
    [machine, powerConfig, sendCommand, onSelect, onClose]
  );

  // Handle keyboard navigation
  useEffect(() => {
    if (!isOpen) return;

    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        onClose();
      } else if (
        e.key === "ArrowDown" ||
        e.key === "ArrowRight" ||
        e.key === "ArrowUp" ||
        e.key === "ArrowLeft"
      ) {
        e.preventDefault();
        // Toggle between the two modes, or start with first if none selected
        setSelectedMode((prev) => {
          if (prev === null) return POWER_MODES.BREW_ONLY;
          return prev === POWER_MODES.BREW_ONLY
            ? POWER_MODES.BREW_STEAM
            : POWER_MODES.BREW_ONLY;
        });
      } else if (e.key === "Enter") {
        e.preventDefault();
        // Only confirm if a mode is selected
        if (selectedMode !== null) {
          handleConfirm(selectedMode);
        }
      }
    };

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [isOpen, selectedMode, onClose, handleConfirm]);

  if (!isOpen) return null;

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center xs:p-4"
      onClick={onClose}
    >
      {/* Backdrop */}
      <div className="absolute inset-0 bg-black/50 xs:backdrop-blur-sm transition-opacity" />

      {/* Modal */}
      <div
        className="relative w-full h-full xs:h-auto xs:max-w-lg xs:max-h-[90vh] bg-theme-card rounded-none xs:rounded-2xl xs:shadow-2xl border-0 xs:border border-theme overflow-hidden transform transition-all flex flex-col"
        onClick={(e) => e.stopPropagation()}
      >
        {/* Header */}
        <div className="flex items-center justify-between p-4 sm:p-6 border-b border-theme flex-shrink-0">
          <div className="flex-1 min-w-0 pr-2">
            <h2 className="text-xl sm:text-2xl font-bold text-theme">
              Select Power Mode
            </h2>
            <p className="text-xs sm:text-sm text-theme-muted mt-1">
              Choose what you want to use
            </p>
          </div>
          <button
            onClick={onClose}
            className="p-2 rounded-lg hover:bg-theme-secondary text-theme-muted hover:text-theme transition-colors flex-shrink-0"
            aria-label="Close"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Mode Selection */}
        <div className="flex-1 overflow-y-auto p-4 sm:p-6">
          <div className="grid grid-cols-1 gap-3 sm:gap-4">
            {POWER_MODE_DEFINITIONS.map((modeDef) => {
              const isSelected = selectedMode === modeDef.value;
              const Icon =
                modeDef.value === POWER_MODES.BREW_ONLY ? Flame : Sparkles;

              // Get auto-selected strategy description for Brew & Steam
              const strategyInfo = getAutoSelectedStrategyDescription(
                modeDef.value,
                machine,
                powerConfig
              );

              return (
                <button
                  key={modeDef.value}
                  onClick={() => handleConfirm(modeDef.value)}
                  className={cn(
                    "relative p-4 sm:p-5 rounded-xl border-2 transition-all text-left group",
                    "hover:scale-[1.02] hover:shadow-lg cursor-pointer active:scale-[0.98]",
                    isSelected
                      ? "border-accent bg-accent/10 shadow-md"
                      : "border-theme bg-theme-secondary hover:border-theme-tertiary"
                  )}
                >
                  {/* Selection indicator */}
                  {isSelected && (
                    <div className="absolute top-2 right-2 sm:top-3 sm:right-3 w-5 h-5 sm:w-6 sm:h-6 rounded-full bg-accent flex items-center justify-center">
                      <Check className="w-3 h-3 sm:w-4 sm:h-4 text-white" />
                    </div>
                  )}

                  <div className="flex items-start gap-4">
                    {/* Icon */}
                    <div
                      className={cn(
                        "w-12 h-12 sm:w-14 sm:h-14 rounded-xl flex items-center justify-center flex-shrink-0 transition-colors",
                        isSelected
                          ? "bg-accent/20 text-accent"
                          : "bg-theme-tertiary text-theme-muted group-hover:text-accent"
                      )}
                    >
                      <Icon className="w-6 h-6 sm:w-7 sm:h-7" />
                    </div>

                    <div className="flex-1 min-w-0">
                      {/* Label */}
                      <h3
                        className={cn(
                          "text-lg sm:text-xl font-semibold mb-1",
                          isSelected ? "text-theme" : "text-theme-secondary"
                        )}
                      >
                        {modeDef.label}
                      </h3>

                      {/* Description */}
                      <p className="text-sm text-theme-muted mb-2">
                        {modeDef.description}
                      </p>

                      {/* Detail */}
                      <p className="text-xs text-theme-muted leading-relaxed">
                        {modeDef.detail}
                      </p>

                      {/* Strategy info for Brew & Steam */}
                      {strategyInfo && (
                        <div className="flex items-center gap-1.5 mt-2 text-xs text-theme-muted">
                          <Info className="w-3.5 h-3.5 flex-shrink-0" />
                          <span>{strategyInfo}</span>
                        </div>
                      )}
                    </div>
                  </div>
                </button>
              );
            })}
          </div>

          {/* Power settings info */}
          {machine && (
            <div className="mt-4 pt-4 border-t border-theme">
              <p className="text-xs text-theme-muted text-center">
                Power settings: {voltage}V / {maxCurrent}A max
                {machine.specs.brewPowerWatts &&
                  machine.specs.steamPowerWatts && (
                    <span className="block mt-1">
                      {machine.brand} {machine.model}:{" "}
                      {machine.specs.brewPowerWatts}W brew +{" "}
                      {machine.specs.steamPowerWatts}W steam
                    </span>
                  )}
              </p>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

// Re-export for backward compatibility - components can migrate gradually
export { POWER_MODE_DEFINITIONS as HEATING_STRATEGIES };
