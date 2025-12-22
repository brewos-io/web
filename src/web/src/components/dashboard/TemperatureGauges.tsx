import { memo, useMemo } from "react";
import { Gauge } from "@/components/Gauge";
import { Flame, Wind } from "lucide-react";

interface TemperatureData {
  current: number;
  setpoint: number;
  max: number;
}

interface TemperatureGaugesProps {
  machineType: string | undefined;
  brewTemp: TemperatureData;
  steamTemp: TemperatureData;
  groupTemp: number;
}

export const TemperatureGauges = memo(function TemperatureGauges({
  machineType,
  brewTemp,
  steamTemp,
  groupTemp,
}: TemperatureGaugesProps) {
  // For single boiler: detect if in steam mode based on setpoint (>120°C = steam mode)
  const singleBoilerInSteamMode = useMemo(() => {
    if (machineType !== "single_boiler") return false;
    return brewTemp.setpoint > 120;
  }, [machineType, brewTemp.setpoint]);

  // For single boiler: use appropriate max and variant based on mode
  const singleBoilerConfig = useMemo(() => {
    if (singleBoilerInSteamMode) {
      return {
        max: steamTemp.max || 160,
        variant: "steam" as const,
        label: "Boiler (Steam Mode)",
        icon: <Wind className="w-5 h-5" />,
      };
    }
    return {
      max: brewTemp.max,
      variant: "default" as const,
      label: "Boiler (Brew Mode)",
      icon: <Flame className="w-5 h-5" />,
    };
  }, [singleBoilerInSteamMode, brewTemp.max, steamTemp.max]);

  return (
    <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
      {/* Dual Boiler: Brew + Steam */}
      {machineType === "dual_boiler" && (
        <>
          <Gauge
            value={brewTemp.current}
            max={brewTemp.max}
            setpoint={brewTemp.setpoint}
            label="Brew Boiler"
            icon={<Flame className="w-5 h-5" />}
            variant="default"
          />
          <Gauge
            value={steamTemp.current}
            max={steamTemp.max}
            setpoint={steamTemp.setpoint}
            label="Steam Boiler"
            icon={<Wind className="w-5 h-5" />}
            variant="steam"
          />
        </>
      )}

      {/* Single Boiler: One boiler gauge spanning full width */}
      {machineType === "single_boiler" && (
        <div className="md:col-span-2">
          <Gauge
            value={brewTemp.current}
            max={singleBoilerConfig.max}
            setpoint={brewTemp.setpoint}
            label={singleBoilerConfig.label}
            icon={singleBoilerConfig.icon}
            variant={singleBoilerConfig.variant}
          />
        </div>
      )}

      {/* Heat Exchanger: Steam Boiler + Group Head */}
      {machineType === "heat_exchanger" && (
        <>
          <Gauge
            value={steamTemp.current}
            max={steamTemp.max}
            setpoint={steamTemp.setpoint}
            label="Steam Boiler"
            icon={<Wind className="w-5 h-5" />}
            variant="steam"
          />
          <Gauge
            value={groupTemp}
            max={105}
            setpoint={93}
            label="Group Head"
            icon={<Flame className="w-5 h-5" />}
            variant="default"
          />
        </>
      )}

      {/* Unknown machine type - show both as fallback */}
      {!machineType && (
        <>
          <Gauge
            value={brewTemp.current}
            max={brewTemp.max}
            setpoint={brewTemp.setpoint}
            label="Brew Boiler"
            icon={<Flame className="w-5 h-5" />}
            variant="default"
          />
          <Gauge
            value={steamTemp.current}
            max={steamTemp.max}
            setpoint={steamTemp.setpoint}
            label="Steam Boiler"
            icon={<Wind className="w-5 h-5" />}
            variant="steam"
          />
        </>
      )}
    </div>
  );
});

