import { useMemo, useState } from "react";
import type { DailySummary } from "@/lib/types";

export interface EnergyTrendsChartProps {
  data: DailySummary[];
  height?: number;
  emptyMessage?: string;
}

interface TooltipData {
  x: number;
  date: string;
  kwh: number;
  shots: number;
  onTime: string;
}

export function EnergyTrendsChart({
  data,
  height = 200,
  emptyMessage = "Energy trends will appear over time",
}: EnergyTrendsChartProps) {
  const [tooltip, setTooltip] = useState<TooltipData | null>(null);
  const [activeIndex, setActiveIndex] = useState<number | null>(null);
  const [containerRef, setContainerRef] = useState<HTMLDivElement | null>(null);

  const chartData = useMemo(() => {
    if (data.length === 0) return { days: [], maxKwh: 1, totalKwh: 0, avgKwh: 0 };
    
    const days = data.slice(-30); // Last 30 days
    const maxKwh = Math.max(...days.map((d) => d.totalKwh), 0.1);
    const totalKwh = days.reduce((sum, d) => sum + d.totalKwh, 0);
    const avgKwh = totalKwh / days.length;
    
    return { days, maxKwh, totalKwh, avgKwh };
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
    return date.toLocaleDateString([], { weekday: "short", month: "short", day: "numeric" });
  };
  
  const formatOnTime = (minutes: number) => {
    const hours = Math.floor(minutes / 60);
    const mins = minutes % 60;
    if (hours === 0) return `${mins}m`;
    return `${hours}h ${mins}m`;
  };

  const handleBarHover = (day: DailySummary, index: number, event: React.MouseEvent) => {
    if (!containerRef) return;
    
    const rect = containerRef.getBoundingClientRect();
    const x = event.clientX - rect.left;
    
    setActiveIndex(index);
    setTooltip({
      x,
      date: formatDateLong(day.date),
      kwh: day.totalKwh,
      shots: day.shotCount,
      onTime: formatOnTime(day.onTimeMinutes),
    });
  };

  // Get date range info for x-axis
  const firstDate = chartData.days[0]?.date;
  const lastDate = chartData.days[chartData.days.length - 1]?.date;
  const midDate = chartData.days[Math.floor(chartData.days.length / 2)]?.date;

  return (
    <div className="w-full" style={{ height }} ref={setContainerRef}>
      {/* Chart header */}
      <div className="flex justify-between items-center mb-2 px-1">
        <span className="text-xs text-theme-muted">
          Avg: {chartData.avgKwh.toFixed(2)} kWh/day
        </span>
        <span className="text-xs font-medium text-amber-500">
          Total: {chartData.totalKwh.toFixed(1)} kWh
        </span>
      </div>
      
      {/* Chart area */}
      <div 
        className="relative w-full"
        style={{ height: height - 64 }}
        onMouseLeave={() => { setTooltip(null); setActiveIndex(null); }}
      >
        {/* Y-axis labels */}
        <div className="absolute left-0 top-0 h-full flex flex-col justify-between text-[10px] text-theme-muted pr-2 pointer-events-none w-12">
          <span>{chartData.maxKwh.toFixed(1)}</span>
          <span>{(chartData.maxKwh / 2).toFixed(1)}</span>
          <span>0 kWh</span>
        </div>
        
        {/* Bars container */}
        <div className="absolute left-12 right-0 top-0 bottom-0">
          {/* Grid lines */}
          <div className="absolute inset-0 flex flex-col justify-between pointer-events-none">
            {[0, 1, 2].map((i) => (
              <div 
                key={i} 
                className="border-t border-theme-border/30" 
                style={{ borderStyle: i === 2 ? 'solid' : 'dashed' }}
              />
            ))}
          </div>
          
          {/* Average line */}
          <div 
            className="absolute left-0 right-0 border-t-2 border-dashed border-amber-500/40 pointer-events-none"
            style={{ 
              top: `${100 - (chartData.avgKwh / chartData.maxKwh) * 100}%` 
            }}
          />
          
          {/* Bars */}
          <div className="absolute inset-0 flex items-end gap-[1px]">
            {chartData.days.map((day, index) => {
              const heightPercent = (day.totalKwh / chartData.maxKwh) * 100;
              const isActive = activeIndex === index;
              const isAboveAvg = day.totalKwh > chartData.avgKwh;
              
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
                      height: `${Math.max(heightPercent, day.totalKwh > 0 ? 5 : 1)}%`,
                      transform: isActive ? 'scaleX(1.15)' : 'scaleX(1)',
                      background: isActive 
                        ? 'linear-gradient(to top, #f59e0b, #fbbf24)' 
                        : isAboveAvg
                          ? 'linear-gradient(to top, #f59e0b, #fbbf24)'
                          : 'linear-gradient(to top, rgba(245, 158, 11, 0.8), rgba(251, 191, 36, 0.8))',
                      boxShadow: isActive ? '0 0 12px rgba(245, 158, 11, 0.3)' : 'none',
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
              left: Math.min(Math.max(tooltip.x - 75, 60), (containerRef?.offsetWidth || 300) - 170),
              top: 8,
            }}
          >
            <div 
              className="rounded-lg shadow-2xl px-3 py-2.5 text-xs min-w-[150px]"
              style={{
                backgroundColor: 'rgba(30, 25, 20, 0.98)',
                border: '1px solid rgba(255, 255, 255, 0.1)',
                boxShadow: '0 8px 32px rgba(0, 0, 0, 0.4)',
              }}
            >
              <div 
                className="font-semibold mb-2 pb-1.5"
                style={{ 
                  color: '#f5f5f4',
                  borderBottom: '1px solid rgba(255, 255, 255, 0.1)',
                }}
              >
                {tooltip.date}
              </div>
              <div className="space-y-1.5" style={{ color: '#a8a29e' }}>
                <div className="flex justify-between gap-4">
                  <span>Energy:</span>
                  <span className="font-bold" style={{ color: '#f59e0b' }}>{tooltip.kwh.toFixed(2)} kWh</span>
                </div>
                <div className="flex justify-between gap-4">
                  <span>Shots:</span>
                  <span className="font-medium" style={{ color: '#f5f5f4' }}>{tooltip.shots}</span>
                </div>
                <div className="flex justify-between gap-4">
                  <span>On time:</span>
                  <span className="font-medium" style={{ color: '#f5f5f4' }}>{tooltip.onTime}</span>
                </div>
              </div>
            </div>
          </div>
        )}
      </div>
      
      {/* X-axis labels */}
      <div className="flex justify-between mt-2 pl-12 text-[10px] text-theme-muted">
        <span>{formatDate(firstDate)}</span>
        {chartData.days.length > 7 && <span>{formatDate(midDate)}</span>}
        <span>{formatDate(lastDate)}</span>
      </div>
    </div>
  );
}

