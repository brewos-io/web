import { useMemo, useState } from "react";
import type { PowerSample } from "@/lib/types";

export interface PowerChartProps {
  data: PowerSample[];
  height?: number;
  showLabels?: boolean;
  emptyMessage?: string;
}

interface TooltipData {
  x: number;
  y: number;
  time: string;
  avgWatts: number;
  maxWatts: number;
  kwh: number;
}

export function PowerChart({
  data,
  height = 200,
  showLabels = true,
  emptyMessage = "No power data available",
}: PowerChartProps) {
  const [tooltip, setTooltip] = useState<TooltipData | null>(null);
  const [containerRef, setContainerRef] = useState<HTMLDivElement | null>(null);

  // Aggregate data into hourly buckets for cleaner visualization
  const chartData = useMemo(() => {
    if (data.length === 0)
      return { buckets: [], maxWatts: 1000, peakWatts: 1000, totalKwh: 0 };

    // Group by hour
    const hourlyMap = new Map<
      number,
      { samples: PowerSample[]; hourStart: number }
    >();

    data.forEach((sample) => {
      const date = new Date(sample.timestamp * 1000);
      const hourStart =
        new Date(
          date.getFullYear(),
          date.getMonth(),
          date.getDate(),
          date.getHours()
        ).getTime() / 1000;

      if (!hourlyMap.has(hourStart)) {
        hourlyMap.set(hourStart, { samples: [], hourStart });
      }
      hourlyMap.get(hourStart)!.samples.push(sample);
    });

    // Convert to buckets with aggregated values
    const buckets = Array.from(hourlyMap.values())
      .sort((a, b) => a.hourStart - b.hourStart)
      .map(({ samples, hourStart }) => ({
        timestamp: hourStart,
        avgWatts:
          samples.reduce((sum, s) => sum + s.avgWatts, 0) / samples.length,
        maxWatts: Math.max(...samples.map((s) => s.maxWatts)),
        kwhConsumed: samples.reduce((sum, s) => sum + s.kwhConsumed, 0),
      }));

    // Scale Y-axis based on average watts only, so bars are prominent
    // Add 15% headroom above the max average
    const maxAvgWatts = Math.max(...buckets.map((b) => b.avgWatts), 50);
    const peakWatts = Math.max(...buckets.map((b) => b.maxWatts), 100);
    const scaleMax = maxAvgWatts * 1.15;
    const totalKwh = buckets.reduce((sum, b) => sum + b.kwhConsumed, 0);

    return { buckets, maxWatts: scaleMax, peakWatts, totalKwh };
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

  const formatTime = (timestamp: number) => {
    const date = new Date(timestamp * 1000);
    return date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
  };

  const formatDateTime = (timestamp: number) => {
    const date = new Date(timestamp * 1000);
    const now = new Date();
    const isToday = date.toDateString() === now.toDateString();
    const yesterday = new Date(now);
    yesterday.setDate(yesterday.getDate() - 1);
    const isYesterday = date.toDateString() === yesterday.toDateString();

    const time = date.toLocaleTimeString([], {
      hour: "2-digit",
      minute: "2-digit",
    });

    if (isToday) return time;
    if (isYesterday) return `Yesterday ${time}`;
    return (
      date.toLocaleDateString([], { month: "short", day: "numeric" }) +
      ` ${time}`
    );
  };

  const handleBarHover = (
    bucket: (typeof chartData.buckets)[0],
    index: number,
    event: React.MouseEvent
  ) => {
    if (!containerRef) return;

    const rect = containerRef.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;

    setTooltip({
      x,
      y,
      time: formatDateTime(bucket.timestamp),
      avgWatts: bucket.avgWatts,
      maxWatts: bucket.maxWatts,
      kwh: bucket.kwhConsumed,
    });
  };

  const barWidth = Math.max(100 / chartData.buckets.length - 1, 2);
  const barGap = Math.min(1, 100 / chartData.buckets.length / 4);

  return (
    <div className="w-full" style={{ height }} ref={setContainerRef}>
      {/* Chart header with summary */}
      {showLabels && (
        <div className="flex justify-between items-center mb-2 px-1">
          <span className="text-xs text-theme-muted">
            Peak: {Math.round(chartData.peakWatts)}W
          </span>
          <span className="text-xs font-medium text-accent">
            Total: {chartData.totalKwh.toFixed(2)} kWh
          </span>
        </div>
      )}

      {/* SVG Chart */}
      <div
        className="relative w-full"
        style={{ height: height - (showLabels ? 48 : 16) }}
        onMouseLeave={() => setTooltip(null)}
      >
        <svg
          viewBox="0 0 100 100"
          preserveAspectRatio="none"
          className="w-full h-full"
        >
          <defs>
            <linearGradient
              id="powerBarGradient"
              x1="0%"
              y1="0%"
              x2="0%"
              y2="100%"
            >
              <stop offset="0%" stopColor="var(--accent)" stopOpacity="0.9" />
              <stop offset="100%" stopColor="var(--accent)" stopOpacity="0.4" />
            </linearGradient>
            <linearGradient
              id="powerBarGradientHover"
              x1="0%"
              y1="0%"
              x2="0%"
              y2="100%"
            >
              <stop offset="0%" stopColor="var(--accent)" stopOpacity="1" />
              <stop offset="100%" stopColor="var(--accent)" stopOpacity="0.7" />
            </linearGradient>
          </defs>

          {/* Horizontal grid lines */}
          {[0, 25, 50, 75, 100].map((y) => (
            <line
              key={y}
              x1="0"
              y1={y}
              x2="100"
              y2={y}
              stroke="currentColor"
              strokeOpacity={y === 100 ? "0.2" : "0.08"}
              strokeWidth={y === 100 ? "0.5" : "0.3"}
            />
          ))}

          {/* Bars - showing average power consumption */}
          {chartData.buckets.map((bucket, index) => {
            const barHeight = (bucket.avgWatts / chartData.maxWatts) * 100;
            const x = (index / chartData.buckets.length) * 100 + barGap;

            return (
              <g
                key={index}
                className="cursor-pointer group"
                onMouseEnter={(e) => handleBarHover(bucket, index, e)}
                onMouseMove={(e) => handleBarHover(bucket, index, e)}
              >
                {/* Average bar */}
                <rect
                  x={x}
                  y={100 - barHeight}
                  width={barWidth}
                  height={Math.max(barHeight, 0.5)}
                  fill="url(#powerBarGradient)"
                  rx="0.5"
                  className="transition-all duration-150 group-hover:fill-[url(#powerBarGradientHover)]"
                />
              </g>
            );
          })}
        </svg>

        {/* Y-axis labels */}
        {showLabels && (
          <div className="absolute left-0 top-0 h-full flex flex-col justify-between text-[10px] text-theme-muted pr-2 pointer-events-none">
            <span>{Math.round(chartData.maxWatts)}W</span>
            <span>{Math.round(chartData.maxWatts / 2)}W</span>
            <span>0W</span>
          </div>
        )}

        {/* Tooltip */}
        {tooltip && (
          <div
            className="absolute z-50 pointer-events-none"
            style={{
              left: Math.min(
                tooltip.x,
                (containerRef?.offsetWidth || 300) - 160
              ),
              top: Math.max(tooltip.y - 90, 0),
            }}
          >
            <div
              className="bg-theme-base rounded-lg shadow-2xl px-3 py-2.5 text-xs min-w-[140px] backdrop-blur-sm"
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
                {tooltip.time}
              </div>
              <div className="space-y-1.5" style={{ color: "#a8a29e" }}>
                <div className="flex justify-between gap-4">
                  <span>Average:</span>
                  <span className="font-medium" style={{ color: "#f5f5f4" }}>
                    {Math.round(tooltip.avgWatts)}W
                  </span>
                </div>
                <div className="flex justify-between gap-4">
                  <span>Peak:</span>
                  <span
                    className="font-bold"
                    style={{ color: "var(--accent)" }}
                  >
                    {Math.round(tooltip.maxWatts)}W
                  </span>
                </div>
                <div className="flex justify-between gap-4">
                  <span>Energy:</span>
                  <span className="font-medium" style={{ color: "#f5f5f4" }}>
                    {(tooltip.kwh * 1000).toFixed(1)} Wh
                  </span>
                </div>
              </div>
            </div>
          </div>
        )}
      </div>

      {/* X-axis labels */}
      {showLabels && chartData.buckets.length > 0 && (
        <div className="flex justify-between mt-2 px-1 text-[10px] text-theme-muted">
          <div className="text-left">
            <div>{formatDateTime(chartData.buckets[0].timestamp)}</div>
          </div>
          {chartData.buckets.length > 4 && (
            <div className="text-center">
              <div>
                {formatDateTime(
                  chartData.buckets[Math.floor(chartData.buckets.length / 2)]
                    .timestamp
                )}
              </div>
            </div>
          )}
          <div className="text-right">
            <div>
              {formatDateTime(
                chartData.buckets[chartData.buckets.length - 1].timestamp
              )}
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
