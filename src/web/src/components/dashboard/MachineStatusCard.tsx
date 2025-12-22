import { useState, useEffect, useRef, useMemo, useCallback, memo } from "react";
import { Card } from "@/components/Card";
import { Badge } from "@/components/Badge";
import {
  Power,
  Zap,
  Leaf,
  Coffee,
  Thermometer,
  Flame,
  Sparkles,
  ChevronDown,
} from "lucide-react";
import { cn, getMachineStateLabel, getMachineStateColor } from "@/lib/utils";
import { useStore } from "@/lib/store";
import { convertFromCelsius, getUnitSymbol } from "@/lib/temperature";
import type { HeatingStrategy, MachineType } from "@/lib/types";
import { getPowerModeFromStrategy, POWER_MODES } from "@/lib/powerValidation";
import {
  getMachineFeatures,
  isDualBoiler as checkIsDualBoiler,
  getMachineTypeLabel,
} from "@/lib/machineFeatures";

// User-facing power mode labels (not internal heating strategies)
const POWER_MODE_LABELS: Record<string, { label: string; icon: typeof Flame }> =
  {
    brew_only: { label: "Brew Only", icon: Flame },
    brew_steam: { label: "Brew & Steam", icon: Sparkles },
  };

// Extracted outside component to prevent re-creation on each render
interface PowerButtonProps {
  icon: typeof Power;
  label: string;
  description: string;
  isActive: boolean;
  onClick: () => void;
}

const PowerButton = memo(function PowerButton({
  icon: Icon,
  label,
  description,
  isActive,
  onClick,
}: PowerButtonProps) {
  return (
    <button
      onClick={onClick}
      className={cn(
        "group flex-1 px-5 py-4 rounded-xl font-semibold transition-colors duration-200",
        "flex items-center justify-center gap-3",
        isActive
          ? "nav-active"
          : "bg-theme-secondary text-theme-secondary hover:bg-theme-tertiary"
      )}
    >
      <Icon className="w-5 h-5" />
      <div className="flex flex-col items-start">
        <span className="text-sm">{label}</span>
        <span
          className={cn(
            "text-[10px] font-normal",
            isActive ? "opacity-70" : "text-theme-muted"
          )}
        >
          {description}
        </span>
      </div>
    </button>
  );
});

interface MachineStatusCardProps {
  mode: string;
  state: string;
  machineType?: MachineType | undefined;
  heatingStrategy?: HeatingStrategy | null;
  onSetMode: (mode: string) => void;
  onQuickOn: () => void;
  onOpenStrategyModal: () => void;
}

export const MachineStatusCard = memo(function MachineStatusCard({
  mode,
  state,
  machineType,
  heatingStrategy,
  onSetMode,
  onQuickOn,
  onOpenStrategyModal,
}: MachineStatusCardProps) {
  // Get machine features for this type
  const features = getMachineFeatures(machineType);
  const isDualBoiler = checkIsDualBoiler(machineType);
  // Use specific selectors to avoid re-renders from unrelated store changes
  // Round temperature to 1 decimal to prevent flicker from tiny fluctuations
  const brewCurrent = useStore(
    (s) => Math.round(s.temps.brew.current * 10) / 10
  );
  const brewSetpoint = useStore((s) => s.temps.brew.setpoint);
  const steamCurrent = useStore(
    (s) => Math.round(s.temps.steam.current * 10) / 10
  );
  const steamSetpoint = useStore((s) => s.temps.steam.setpoint);
  const temperatureUnit = useStore((s) => s.preferences.temperatureUnit);

  // Memoize calculated values including combined heating progress for dual boilers
  const { heatingProgress, displayTemp, displaySetpoint, unitSymbol } =
    useMemo(() => {
      // Calculate individual boiler progress (capped at 100%)
      const brewProgress = Math.min(
        100,
        Math.max(0, (brewCurrent / brewSetpoint) * 100)
      );
      const steamProgress = Math.min(
        100,
        Math.max(0, (steamCurrent / steamSetpoint) * 100)
      );

      // Consider boiler "at target" when within 1% (handles natural fluctuation)
      const brewAtTarget = brewProgress >= 99;

      // Detect if steam was actually heated (above 50°C means it was heated at some point)
      const steamWasHeated = steamCurrent > 50;

      // Calculate combined progress based on machine type and heating strategy
      let progress: number;

      // When in standby (heatingStrategy is null), infer from actual temperatures
      // This ensures cooling animation reflects what was actually heated
      const effectiveStrategy = heatingStrategy ?? (steamWasHeated ? 2 : 0);

      if (!isDualBoiler || effectiveStrategy === 0) {
        // Single boiler, HX, or Brew Only strategy: only brew boiler matters
        progress = brewProgress;
      } else if (effectiveStrategy === 1) {
        // Sequential: brew heats first (0-50%), then steam (50-100%)
        // Use stable threshold to prevent jumps from temperature fluctuation
        if (!brewAtTarget) {
          progress = brewProgress * 0.5; // 0-50% while brew heats
        } else {
          progress = 50 + steamProgress * 0.5; // 50-100% while steam heats
        }
      } else {
        // Parallel (2) or Smart Stagger (3): both heat together
        // Machine is ready when BOTH reach target, so use the minimum
        progress = Math.min(brewProgress, steamProgress);
      }

      const dispTemp = convertFromCelsius(brewCurrent, temperatureUnit);
      const dispSetpoint = convertFromCelsius(brewSetpoint, temperatureUnit);
      const unit = getUnitSymbol(temperatureUnit);
      return {
        heatingProgress: progress,
        displayTemp: dispTemp,
        displaySetpoint: dispSetpoint,
        unitSymbol: unit,
      };
    }, [
      brewCurrent,
      brewSetpoint,
      steamCurrent,
      steamSetpoint,
      temperatureUnit,
      isDualBoiler,
      heatingStrategy,
    ]);

  return (
    <Card className="relative overflow-hidden">
      {/* Subtle accent line at top based on state */}
      <div
        className={cn(
          "absolute top-0 left-0 right-0 h-1 transition-colors duration-500",
          state === "offline" && "bg-slate-500",
          state === "ready" && "bg-emerald-500",
          state === "heating" && "bg-amber-500",
          state === "brewing" && "bg-accent",
          state === "safe" && "bg-red-500",
          state === "eco" && "bg-blue-400",
          // Cooling state: standby but still hot
          mode === "standby" && heatingProgress > 10 && "bg-blue-400",
          // Fully cooled or never heated
          mode === "standby" && heatingProgress <= 10 && "bg-theme-tertiary",
          state === "idle" && mode !== "standby" && "bg-theme-tertiary"
        )}
      />

      <div className="relative space-y-6">
        <div className="flex items-center justify-between gap-4 sm:gap-6">
          {/* Status Ring - round progress to prevent flicker from tiny changes */}
          <StatusRing
            mode={mode}
            state={state}
            heatingProgress={Math.round(heatingProgress)}
          />

          {/* Status Info */}
          <div className="flex-1 min-w-0">
            <div className="flex items-center gap-3 mb-2">
              <h2 className="text-xl sm:text-2xl font-bold text-theme">
                Machine Status
              </h2>
            </div>
            <div className="flex items-center gap-2 flex-wrap">
              <Badge
                className={cn(
                  getMachineStateColor(state),
                  "text-sm px-4 py-1.5"
                )}
              >
                {getMachineStateLabel(state)}
              </Badge>
              {/* Power mode - show for machines that support power modes when machine is on */}
              {features.supportsPowerModes &&
                mode !== "standby" &&
                heatingStrategy !== null &&
                heatingStrategy !== undefined && (
                  <span className="inline-flex items-center gap-1.5 text-xs text-theme-muted">
                    <span className="text-theme-tertiary">•</span>
                    {(() => {
                      const powerMode =
                        getPowerModeFromStrategy(heatingStrategy);
                      const modeInfo = POWER_MODE_LABELS[powerMode];
                      const Icon = modeInfo?.icon || Flame;
                      return (
                        <>
                          <Icon className="w-3 h-3" />
                          {modeInfo?.label || "Unknown"}
                        </>
                      );
                    })()}
                  </span>
                )}
              {/* Machine type indicator for non-dual-boiler machines when on */}
              {!features.supportsPowerModes &&
                mode !== "standby" &&
                machineType && (
                  <span className="inline-flex items-center gap-1.5 text-xs text-theme-muted">
                    <span className="text-theme-tertiary">•</span>
                    {getMachineTypeLabel(machineType)}
                  </span>
                )}
            </div>

            {/* Temperature display - always reserve space to prevent layout shift */}
            <div
              className={cn(
                "mt-4 flex items-center gap-2 text-theme-secondary h-7 transition-opacity duration-300",
                mode === "standby" ? "opacity-0" : "opacity-100"
              )}
            >
              <Thermometer className="w-4 h-4 text-accent flex-shrink-0" />
              <span className="text-lg font-semibold tabular-nums min-w-[4.5rem]">
                {displayTemp.toFixed(1)}
                {unitSymbol}
              </span>
              <span className="text-theme-muted text-sm flex-shrink-0">
                / {displaySetpoint.toFixed(0)}
                {unitSymbol}
              </span>
            </div>

            {/* Contextual helper text - fixed height to prevent layout shift */}
            <p className="mt-2 text-sm text-theme-muted min-h-[2.5rem]">
              {getStatusDescription(
                mode,
                state,
                heatingProgress,
                machineType,
                heatingStrategy
              )}
            </p>
          </div>
        </div>

        {/* Power Controls */}
        <PowerControls
          mode={mode}
          state={state}
          heatingStrategy={heatingStrategy}
          machineType={machineType}
          onSetMode={onSetMode}
          onQuickOn={onQuickOn}
          onOpenStrategyModal={onOpenStrategyModal}
        />
      </div>
    </Card>
  );
});

function getStatusDescription(
  mode: string,
  state: string,
  heatingProgress?: number,
  machineType?: MachineType,
  heatingStrategy?: HeatingStrategy | null
): string {
  const features = getMachineFeatures(machineType);

  // Handle offline state first - device not reachable
  if (state === "offline") {
    return "Machine is offline. Check power and network connection.";
  }
  if (mode === "standby") {
    // Check if still cooling down
    if (heatingProgress && heatingProgress > 10) {
      return `Cooling down... ${Math.round(heatingProgress)}% residual heat.`;
    }
    return "Machine is in standby mode. Turn on to start heating.";
  }
  if (mode === "eco") {
    if (features.needsModeSwitching) {
      return "Eco mode - boiler at reduced temperature to save power.";
    }
    return "Eco mode active. Lower temperature to save energy.";
  }
  if (state === "heating") {
    if (heatingProgress && heatingProgress > 0) {
      // Single boiler - simple message
      if (features.needsModeSwitching) {
        return `Heating boiler... ${Math.round(
          heatingProgress
        )}% to brew temperature.`;
      }
      // Heat exchanger - heating steam boiler
      if (features.isHeatExchanger) {
        return `Heating steam boiler... ${Math.round(heatingProgress)}%`;
      }
      // Dual boiler - more descriptive message when in Brew & Steam mode
      const isBrewSteamMode =
        features.supportsPowerModes && heatingStrategy !== 0;
      if (isBrewSteamMode) {
        // For sequential heating (strategy 1), show which boiler
        if (heatingStrategy === 1 && heatingProgress < 50) {
          return `Heating brew boiler... ${Math.round(heatingProgress * 2)}%`;
        } else if (heatingStrategy === 1 && heatingProgress >= 50) {
          return `Heating steam boiler... ${Math.round(
            (heatingProgress - 50) * 2
          )}%`;
        }
        return `Warming up both boilers... ${Math.round(heatingProgress)}%`;
      }
      return `Warming up... ${Math.round(
        heatingProgress
      )}% to target temperature.`;
    }
    return "Warming up... Your machine will be ready soon.";
  }
  if (state === "ready") {
    // Single boiler
    if (features.needsModeSwitching) {
      return "Boiler at brew temperature. Ready to brew!";
    }
    // Heat exchanger
    if (features.isHeatExchanger) {
      return "Steam boiler ready. Group head at brewing temperature!";
    }
    // Dual boiler - check if in Brew & Steam mode (any strategy except 0)
    const isBrewSteamMode =
      features.supportsPowerModes && heatingStrategy !== 0;
    if (isBrewSteamMode) {
      return "Both boilers at temperature. Ready to brew!";
    }
    return "Perfect temperature reached. Ready to brew!";
  }
  if (state === "brewing") return "Extraction in progress...";
  if (state === "fault") return "Fault detected. Check machine.";
  if (state === "safe") return "Safe mode - all outputs disabled.";
  if (state === "eco") return "Eco mode - reduced temperature to save power.";
  return "Monitoring your espresso machine.";
}

interface StatusRingProps {
  mode: string;
  state: string;
  heatingProgress: number;
}

const StatusRing = memo(function StatusRing({
  mode,
  state,
  heatingProgress,
}: StatusRingProps) {
  const [displayProgress, setDisplayProgress] = useState(0);
  const lastUpdateRef = useRef(0);

  // Always use actual heating progress - this reflects real boiler temperature
  // Whether heating up, ready, or cooling down after turning off
  const targetProgress = heatingProgress;

  // Only update display progress when change is significant (>2%) or enough time passed
  useEffect(() => {
    const now = Date.now();
    const timeSinceLastUpdate = now - lastUpdateRef.current;
    const progressDiff = Math.abs(targetProgress - displayProgress);

    // Update if: significant change or enough time passed
    if (progressDiff > 2 || timeSinceLastUpdate > 2000) {
      lastUpdateRef.current = now;
      setDisplayProgress(targetProgress);
    }
  }, [targetProgress, displayProgress]);

  const isActive = mode !== "standby";
  const isReady = state === "ready";
  const isHeating = state === "heating";
  const isBrewing = state === "brewing";
  // Standby but still hot (cooling down)
  const isCooling = mode === "standby" && heatingProgress > 10;

  // Ring dimensions
  const size = 120;
  const strokeWidth = 8;
  const radius = (size - strokeWidth) / 2;
  const circumference = 2 * Math.PI * radius;
  const strokeDashoffset =
    circumference - (displayProgress / 100) * circumference;

  return (
    <div className="relative flex-shrink-0">
      {/* SVG Ring */}
      <svg width={size} height={size} className="transform -rotate-90">
        {/* Background ring */}
        <circle
          cx={size / 2}
          cy={size / 2}
          r={radius}
          fill="none"
          strokeWidth={strokeWidth}
          className="stroke-theme-tertiary"
        />

        {/* Progress ring */}
        <circle
          cx={size / 2}
          cy={size / 2}
          r={radius}
          fill="none"
          strokeWidth={strokeWidth}
          strokeLinecap="round"
          strokeDasharray={circumference}
          strokeDashoffset={strokeDashoffset}
          style={{ transition: "stroke-dashoffset 800ms ease-out" }}
          className={cn(
            isBrewing && "stroke-accent",
            isReady && !isBrewing && "stroke-emerald-500",
            isHeating && "stroke-amber-500",
            isCooling && "stroke-blue-400",
            !isActive && !isCooling && "stroke-theme-muted"
          )}
        />
      </svg>

      {/* Center icon */}
      <div className="absolute inset-0 flex items-center justify-center">
        <div
          className={cn(
            "w-16 h-16 rounded-full flex items-center justify-center",
            isBrewing && "bg-accent/15",
            isReady && !isBrewing && "bg-emerald-500/15",
            isHeating && "bg-amber-500/15",
            isCooling && "bg-blue-400/15",
            !isActive && !isCooling && "bg-theme-tertiary"
          )}
        >
          {isBrewing ? (
            <Coffee className="w-8 h-8 text-accent" />
          ) : (
            <Power
              className={cn(
                "w-8 h-8",
                isReady && "text-emerald-500",
                isHeating && "text-amber-500",
                isCooling && "text-blue-400",
                !isActive && !isCooling && "text-theme-muted"
              )}
            />
          )}
        </div>
      </div>
    </div>
  );
});

interface PowerControlsProps {
  mode: string;
  heatingStrategy?: HeatingStrategy | null;
  machineType?: MachineType;
  onSetMode: (mode: string) => void;
  onQuickOn: () => void;
  onOpenStrategyModal: () => void;
}

const PowerControls = memo(function PowerControls({
  mode,
  heatingStrategy,
  machineType,
  onSetMode,
  onQuickOn,
  onOpenStrategyModal,
  state,
}: PowerControlsProps & { state: string }) {
  const features = getMachineFeatures(machineType);
  const isOnActive = mode === "on";
  const isEcoActive = mode === "eco";

  // Can switch modes when machine is ready (not brewing, not in fault/safe state, not initializing)
  // This allows: On ↔ Eco, Standby → On, Standby → Eco, and changing power mode while running
  const canSwitchModes =
    state !== "brewing" &&
    state !== "fault" &&
    state !== "safe" &&
    state !== "offline" &&
    state !== "init" &&
    state !== "unknown";

  // Split "On" button for dual boiler
  const SplitOnButton = () => {
    // Get the power mode that will be used (from state or localStorage)
    const getEffectivePowerMode = () => {
      if (heatingStrategy !== null && heatingStrategy !== undefined) {
        return getPowerModeFromStrategy(heatingStrategy);
      }
      const stored = localStorage.getItem("brewos-last-power-mode");
      if (
        stored === POWER_MODES.BREW_ONLY ||
        stored === POWER_MODES.BREW_STEAM
      ) {
        return stored;
      }
      return POWER_MODES.BREW_STEAM; // Default to Brew & Steam
    };

    const effectiveMode = getEffectivePowerMode();
    const modeInfo = POWER_MODE_LABELS[effectiveMode];
    const ModeIcon = modeInfo?.icon || Sparkles;
    const modeLabel = modeInfo?.label || "Brew & Steam";

    return (
      <div
        role="group"
        aria-label="Power control"
        className="flex-1 flex rounded-xl overflow-hidden"
      >
        {/* Main button - 70% - Turn On with power mode */}
        <button
          onClick={onQuickOn}
          disabled={!canSwitchModes}
          className={cn(
            "flex-[7] px-4 py-4 font-semibold transition-colors duration-200",
            "flex items-center justify-center gap-3",
            !canSwitchModes && "opacity-50 cursor-not-allowed",
            isOnActive
              ? "nav-active rounded-l-xl"
              : "bg-theme-secondary text-theme-secondary hover:bg-theme-tertiary"
          )}
          title={
            !canSwitchModes
              ? "Cannot change mode while machine is not ready"
              : undefined
          }
        >
          <ModeIcon className="w-5 h-5" />
          <div className="flex flex-col items-start">
            <span className="text-sm">On</span>
            <span
              className={cn(
                "text-[10px] font-normal",
                isOnActive ? "opacity-70" : "text-theme-muted"
              )}
            >
              {modeLabel}
            </span>
          </div>
        </button>

        {/* Divider */}
        <div
          className={cn(
            "w-px",
            isOnActive ? "bg-white/20" : "bg-theme-tertiary"
          )}
        />

        {/* Secondary button - 30% - Change power mode (enabled when machine is running) */}
        <button
          onClick={onOpenStrategyModal}
          disabled={!canSwitchModes}
          className={cn(
            "flex-[3] px-3 py-4 transition-colors duration-200",
            "flex items-center justify-center",
            !canSwitchModes && "opacity-50 cursor-not-allowed",
            isOnActive
              ? "nav-active rounded-r-xl"
              : "bg-theme-secondary text-theme-secondary hover:bg-theme-tertiary"
          )}
          title={
            !canSwitchModes
              ? "Cannot change mode while machine is not ready"
              : "Change power mode"
          }
        >
          <ChevronDown className="w-5 h-5" />
        </button>
      </div>
    );
  };

  // Memoize callbacks to prevent unnecessary re-renders
  const handleStandby = useCallback(() => onSetMode("standby"), [onSetMode]);
  const handleEco = useCallback(() => onSetMode("eco"), [onSetMode]);

  return (
    <div className="border-t border-theme pt-5">
      <div className="flex flex-col sm:flex-row gap-3">
        {/* Standby button */}
        <PowerButton
          icon={Power}
          label="Standby"
          description="Power off"
          isActive={mode === "standby"}
          onClick={handleStandby}
        />

        {/* On button - split for machines that support power modes, simple for others */}
        {features.supportsPowerModes ? (
          <SplitOnButton />
        ) : (
          <button
            onClick={onQuickOn}
            disabled={!canSwitchModes}
            className={cn(
              "flex-1 px-5 py-4 rounded-xl font-semibold transition-colors duration-200",
              "flex items-center justify-center gap-3",
              !canSwitchModes && "opacity-50 cursor-not-allowed",
              isOnActive
                ? "nav-active"
                : "bg-theme-secondary text-theme-secondary hover:bg-theme-tertiary"
            )}
            title={
              !canSwitchModes
                ? "Cannot change mode while machine is not ready"
                : undefined
            }
          >
            <Zap className="w-5 h-5" />
            <div className="flex flex-col items-start">
              <span className="text-sm">On</span>
              <span
                className={cn(
                  "text-[10px] font-normal",
                  isOnActive ? "opacity-70" : "text-theme-muted"
                )}
              >
                Full power
              </span>
            </div>
          </button>
        )}

        {/* Eco button */}
        <button
          onClick={handleEco}
          disabled={!canSwitchModes}
          className={cn(
            "flex-1 px-5 py-4 rounded-xl font-semibold transition-colors duration-200",
            "flex items-center justify-center gap-3",
            !canSwitchModes && "opacity-50 cursor-not-allowed",
            isEcoActive
              ? "nav-active"
              : "bg-theme-secondary text-theme-secondary hover:bg-theme-tertiary"
          )}
          title={
            !canSwitchModes
              ? "Cannot change mode while machine is not ready"
              : undefined
          }
        >
          <Leaf className="w-5 h-5" />
          <div className="flex flex-col items-start">
            <span className="text-sm">Eco</span>
            <span
              className={cn(
                "text-[10px] font-normal",
                isEcoActive ? "opacity-70" : "text-theme-muted"
              )}
            >
              Energy saving
            </span>
          </div>
        </button>
      </div>
    </div>
  );
});
