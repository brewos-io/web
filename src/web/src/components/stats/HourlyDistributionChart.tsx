import { useMemo, useState } from "react";

export interface HourlyData {
  hour: number;
  count: number;
}

export interface HourlyDistributionChartProps {
  data: HourlyData[];
  height?: number;
  emptyMessage?: string;
}

export function HourlyDistributionChart({
  data,
  height = 120,
  emptyMessage = "No data yet",
}: HourlyDistributionChartProps) {
  const [activeHour, setActiveHour] = useState<number | null>(null);
  
  const chartData = useMemo(() => {
    // Ensure we have 24 hours of data
    const fullDay: HourlyData[] = [];
    for (let h = 0; h < 24; h++) {
      const found = data.find((d) => d.hour === h);
      fullDay.push({ hour: h, count: found?.count ?? 0 });
    }
    
    // Use a reasonable max that makes bars visible
    const actualMax = Math.max(...fullDay.map((d) => d.count), 1);
    // Don't let maxCount be too high relative to actual values - cap at 2x peak for visibility
    const maxCount = Math.max(actualMax, 1);
    const peakHour = fullDay.reduce((prev, curr) =>
      curr.count > prev.count ? curr : prev
    );
    
    return { hours: fullDay, maxCount, peakHour };
  }, [data]);

  const totalShots = data.reduce((sum, d) => sum + d.count, 0);
  
  if (totalShots === 0) {
    return (
      <div
        className="flex items-center justify-center text-theme-muted"
        style={{ height }}
      >
        <p>{emptyMessage}</p>
      </div>
    );
  }

  const formatHour = (hour: number) => {
    if (hour === 0) return "12am";
    if (hour === 12) return "12pm";
    return hour < 12 ? `${hour}am` : `${hour - 12}pm`;
  };
  
  const formatHourLong = (hour: number) => {
    const start = formatHour(hour);
    const end = formatHour((hour + 1) % 24);
    return `${start} – ${end}`;
  };

  // Get color based on time of day - using theme-compatible colors
  const getBarStyle = (hour: number, isPeak: boolean, isActive: boolean) => {
    // Peak hour gets special color
    if (isPeak) {
      return {
        background: '#a855f7', // Purple for peak
        opacity: isActive ? 1 : 0.85,
      };
    }
    
    // Time of day based colors using the accent as base
    const baseOpacity = isActive ? 1 : 0.7;
    
    if (hour >= 5 && hour < 12) {
      // Morning - warm amber
      return { background: '#f59e0b', opacity: baseOpacity };
    } else if (hour >= 12 && hour < 17) {
      // Afternoon - orange  
      return { background: '#f97316', opacity: baseOpacity };
    } else if (hour >= 17 && hour < 21) {
      // Evening - accent color
      return { background: 'var(--accent)', opacity: baseOpacity };
    } else {
      // Night - cooler indigo
      return { background: '#6366f1', opacity: baseOpacity };
    }
  };

  return (
    <div className="w-full" style={{ height }}>
      {/* Peak hour indicator */}
      <div className="flex justify-between items-center mb-2 px-1">
        <span className="text-xs text-theme-muted">
          Peak: {formatHour(chartData.peakHour.hour)}
        </span>
        <span className="text-xs font-medium" style={{ color: '#a855f7' }}>
          {chartData.peakHour.count} shots
        </span>
      </div>
      
      {/* Bar chart */}
      <div className="flex items-end justify-between gap-px relative" style={{ height: height - 52 }}>
        {chartData.hours.map((item) => {
          const heightPercent = (item.count / chartData.maxCount) * 100;
          const isPeak = item.hour === chartData.peakHour.hour && item.count > 0;
          const isActive = activeHour === item.hour;
          const barStyle = getBarStyle(item.hour, isPeak, isActive);
          
          return (
            <div
              key={item.hour}
              className="flex-1 h-full flex flex-col items-center justify-end group relative"
              onMouseEnter={() => setActiveHour(item.hour)}
              onMouseLeave={() => setActiveHour(null)}
            >
              {/* Tooltip */}
              <div 
                className={`
                  absolute -top-2 left-1/2 -translate-x-1/2 -translate-y-full
                  rounded-lg shadow-2xl px-3 py-2 text-xs z-20 whitespace-nowrap
                  transition-all duration-150
                  ${isActive ? "opacity-100 scale-100" : "opacity-0 scale-95 pointer-events-none"}
                `}
                style={{
                  backgroundColor: 'rgba(30, 25, 20, 0.98)',
                  border: '1px solid rgba(255, 255, 255, 0.1)',
                  boxShadow: '0 8px 32px rgba(0, 0, 0, 0.4)',
                }}
              >
                <div className="font-semibold" style={{ color: '#f5f5f4' }}>{formatHourLong(item.hour)}</div>
                <div className="flex items-center gap-2 mt-1.5">
                  <span 
                    className="font-bold"
                    style={{ color: isPeak ? '#a855f7' : 'var(--accent)' }}
                  >
                    {item.count} shots
                  </span>
                  {isPeak && (
                    <span 
                      className="text-[10px] px-1.5 py-0.5 rounded font-medium"
                      style={{ background: 'rgba(168, 85, 247, 0.2)', color: '#a855f7' }}
                    >
                      Peak
                    </span>
                  )}
                </div>
              </div>
              
              {/* Bar */}
              <div
                className="w-full rounded-t transition-all duration-200 cursor-pointer"
                style={{
                  height: `${Math.max(heightPercent, item.count > 0 ? 15 : 3)}%`,
                  minHeight: item.count > 0 ? '6px' : '2px',
                  background: barStyle.background,
                  opacity: item.count === 0 ? 0.15 : barStyle.opacity,
                  transform: isActive ? 'scaleX(1.3) scaleY(1.05)' : 'scaleX(1)',
                }}
              />
            </div>
          );
        })}
      </div>
      
      {/* Time labels */}
      <div className="flex justify-between mt-1 text-[10px] text-theme-muted">
        <span>12am</span>
        <span>6am</span>
        <span>12pm</span>
        <span>6pm</span>
        <span>12am</span>
      </div>
    </div>
  );
}
