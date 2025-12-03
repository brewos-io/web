import { useMemo, useState } from "react";
import type { DailySummary } from "@/lib/types";

export interface BrewTrendsChartProps {
  data: DailySummary[];
  height?: number;
  showLabels?: boolean;
  emptyMessage?: string;
}

interface TooltipData {
  x: number;
  date: string;
  shots: number;
  avgBrewTime: number;
  totalKwh: number;
  onTimeMinutes: number;
}

export function BrewTrendsChart({
  data,
  height = 200,
  showLabels = true,
  emptyMessage = "No trend data available",
}: BrewTrendsChartProps) {
  const [tooltip, setTooltip] = useState<TooltipData | null>(null);
  const [activeIndex, setActiveIndex] = useState<number | null>(null);
  const [containerRef, setContainerRef] = useState<HTMLDivElement | null>(null);

  const chartData = useMemo(() => {
    if (data.length === 0)
      return { days: [], maxShots: 10, avgShots: 0, totalShots: 0 };

    const days = data.slice(-30); // Last 30 days
    const maxShots = Math.max(...days.map((d) => d.shotCount), 1);
    const totalShots = days.reduce((sum, d) => sum + d.shotCount, 0);
    const avgShots = totalShots / days.length;

    return { days, maxShots, avgShots, totalShots };
  }, [data]);

  if (data.length === 0) {
    return (
      <div
        className="flex items-center justify-center text-theme-muted"
        style={{ height }}
      >
        <p>{emptyMessage}</p>
      </div>
    );
  }

  const formatDate = (timestamp: number) => {
    const date = new Date(timestamp * 1000);
    return date.toLocaleDateString([], { month: "short", day: "numeric" });
  };

  const formatDateLong = (timestamp: number) => {
    const date = new Date(timestamp * 1000);
    return date.toLocaleDateString([], {
      weekday: "short",
      month: "short",
      day: "numeric",
    });
  };

  const formatOnTime = (minutes: number) => {
    const hours = Math.floor(minutes / 60);
    const mins = minutes % 60;
    if (hours === 0) return `${mins}m`;
    return `${hours}h ${mins}m`;
  };

  const handleBarHover = (
    day: DailySummary,
    index: number,
    event: React.MouseEvent
  ) => {
    if (!containerRef) return;

    const rect = containerRef.getBoundingClientRect();
    const x = event.clientX - rect.left;

    setActiveIndex(index);
    setTooltip({
      x,
      date: formatDateLong(day.date),
      shots: day.shotCount,
      avgBrewTime: day.avgBrewTimeMs / 1000,
      totalKwh: day.totalKwh,
      onTimeMinutes: day.onTimeMinutes,
    });
  };

  // Get date range info for x-axis
  const firstDate = chartData.days[0]?.date;
  const lastDate = chartData.days[chartData.days.length - 1]?.date;
  const midDate = chartData.days[Math.floor(chartData.days.length / 2)]?.date;

  return (
    <div className="w-full" style={{ height }} ref={setContainerRef}>
      {/* Chart header */}
      {showLabels && (
        <div className="flex justify-between items-center mb-2 px-1">
          <span className="text-xs text-theme-muted">
            Avg: {chartData.avgShots.toFixed(1)} shots/day
          </span>
          <span
            className="text-xs font-medium"
            style={{ color: "var(--accent)" }}
          >
            Total: {chartData.totalShots} shots
          </span>
        </div>
      )}

      {/* Chart area */}
      <div
        className="relative w-full"
        style={{ height: height - (showLabels ? 56 : 16) }}
        onMouseLeave={() => {
          setTooltip(null);
          setActiveIndex(null);
        }}
      >
        {/* Y-axis labels */}
        {showLabels && (
          <div className="absolute left-0 top-0 h-full flex flex-col justify-between text-[10px] text-theme-muted pr-2 pointer-events-none w-8">
            <span>{chartData.maxShots}</span>
            <span>{Math.round(chartData.maxShots / 2)}</span>
            <span>0</span>
          </div>
        )}

        {/* Bars container */}
        <div className="absolute left-8 right-0 top-0 bottom-0">
          {/* Grid lines */}
          <div className="absolute inset-0 flex flex-col justify-between pointer-events-none">
            {[0, 1, 2].map((i) => (
              <div
                key={i}
                className="border-t border-theme-border/20"
                style={{ borderStyle: i === 2 ? "solid" : "dashed" }}
              />
            ))}
          </div>

          {/* Average line */}
          <div
            className="absolute left-0 right-0 border-t-2 border-dashed pointer-events-none z-10"
            style={{
              top: `${100 - (chartData.avgShots / chartData.maxShots) * 100}%`,
              borderColor: "var(--accent)",
              opacity: 0.4,
            }}
          />

          {/* Bars */}
          <div className="absolute inset-0 flex items-end gap-[1px]">
            {chartData.days.map((day, index) => {
              const heightPercent = (day.shotCount / chartData.maxShots) * 100;
              const isActive = activeIndex === index;
              const isAboveAvg = day.shotCount > chartData.avgShots;

              return (
                <div
                  key={index}
                  className="flex-1 h-full flex items-end cursor-pointer min-w-0"
                  onMouseEnter={(e) => handleBarHover(day, index, e)}
                  onMouseMove={(e) => handleBarHover(day, index, e)}
                >
                  <div
                    className="w-full rounded-t transition-all duration-150"
                    style={{
                      height: `${Math.max(
                        heightPercent,
                        day.shotCount > 0 ? 5 : 1
                      )}%`,
                      transform: isActive ? "scaleX(1.2)" : "scaleX(1)",
                      background: "var(--accent)",
                      opacity: isActive ? 1 : isAboveAvg ? 0.9 : 0.7,
                      boxShadow: isActive
                        ? "0 0 12px var(--accent-glow)"
                        : "none",
                    }}
                  />
                </div>
              );
            })}
          </div>
        </div>

        {/* Tooltip */}
        {tooltip && (
          <div
            className="absolute z-50 pointer-events-none"
            style={{
              left: Math.min(
                Math.max(tooltip.x - 75, 40),
                (containerRef?.offsetWidth || 300) - 170
              ),
              top: 8,
            }}
          >
            <div
              className="rounded-lg shadow-2xl px-3 py-2.5 text-xs min-w-[150px]"
              style={{
                backgroundColor: "rgba(30, 25, 20, 0.98)",
                border: "1px solid rgba(255, 255, 255, 0.1)",
                boxShadow: "0 8px 32px rgba(0, 0, 0, 0.4)",
              }}
            >
              <div
                className="font-semibold mb-2 pb-1.5"
                style={{
                  color: "#f5f5f4",
                  borderBottom: "1px solid rgba(255, 255, 255, 0.1)",
                }}
              >
                {tooltip.date}
              </div>
              <div className="space-y-1.5" style={{ color: "#a8a29e" }}>
                <div className="flex justify-between gap-4">
                  <span>Shots:</span>
                  <span
                    className="font-bold"
                    style={{ color: "var(--accent)" }}
                  >
                    {tooltip.shots}
                  </span>
                </div>
                {tooltip.avgBrewTime > 0 && (
                  <div className="flex justify-between gap-4">
                    <span>Avg time:</span>
                    <span className="font-medium" style={{ color: "#f5f5f4" }}>
                      {tooltip.avgBrewTime.toFixed(1)}s
                    </span>
                  </div>
                )}
                {tooltip.totalKwh > 0 && (
                  <div className="flex justify-between gap-4">
                    <span>Energy:</span>
                    <span className="font-medium" style={{ color: "#f5f5f4" }}>
                      {tooltip.totalKwh.toFixed(2)} kWh
                    </span>
                  </div>
                )}
                {tooltip.onTimeMinutes > 0 && (
                  <div className="flex justify-between gap-4">
                    <span>On time:</span>
                    <span className="font-medium" style={{ color: "#f5f5f4" }}>
                      {formatOnTime(tooltip.onTimeMinutes)}
                    </span>
                  </div>
                )}
              </div>
            </div>
          </div>
        )}
      </div>

      {/* X-axis labels */}
      {showLabels && chartData.days.length > 0 && (
        <div className="flex justify-between mt-2 pl-8 text-[10px] text-theme-muted">
          <span>{formatDate(firstDate)}</span>
          {chartData.days.length > 7 && <span>{formatDate(midDate)}</span>}
          <span>{formatDate(lastDate)}</span>
        </div>
      )}
    </div>
  );
}
