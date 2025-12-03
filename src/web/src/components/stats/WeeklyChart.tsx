import { useState } from "react";

export interface WeeklyData {
  day: string;
  shots: number;
}

export interface WeeklyChartProps {
  data: WeeklyData[];
  emptyMessage?: string;
}

export function WeeklyChart({
  data,
  emptyMessage = "No data available yet",
}: WeeklyChartProps) {
  const [activeIndex, setActiveIndex] = useState<number | null>(null);
  const maxShots =
    data.length > 0 ? Math.max(...data.map((d) => d.shots), 1) : 5;
  const totalShots = data.reduce((sum, d) => sum + d.shots, 0);
  const avgShots = data.length > 0 ? totalShots / data.length : 0;

  if (data.length === 0) {
    return (
      <div className="flex items-center justify-center h-full text-theme-muted">
        <p>{emptyMessage}</p>
      </div>
    );
  }

  return (
    <div className="flex items-end justify-between h-full gap-2 pt-4">
      {data.map((item, index) => {
        const isActive = activeIndex === index;
        const isAboveAvg = item.shots > avgShots;
        const heightPercent = Math.max((item.shots / maxShots) * 100, 4);

        return (
          <div
            key={item.day}
            className="flex-1 flex flex-col items-center relative group"
            onMouseEnter={() => setActiveIndex(index)}
            onMouseLeave={() => setActiveIndex(null)}
          >
            {/* Tooltip */}
            <div
              className={`
                absolute -top-2 left-1/2 -translate-x-1/2 -translate-y-full
                rounded-lg shadow-2xl px-3 py-2 text-xs z-10 whitespace-nowrap
                transition-all duration-150
                ${
                  isActive
                    ? "opacity-100 scale-100"
                    : "opacity-0 scale-95 pointer-events-none"
                }
              `}
              style={{
                backgroundColor: "rgba(30, 25, 20, 0.98)",
                border: "1px solid rgba(255, 255, 255, 0.1)",
                boxShadow: "0 8px 32px rgba(0, 0, 0, 0.4)",
              }}
            >
              <div className="font-semibold" style={{ color: "#f5f5f4" }}>
                {item.day}
              </div>
              <div className="flex items-center gap-2 mt-1.5">
                <span className="font-bold" style={{ color: "var(--accent)" }}>
                  {item.shots} shots
                </span>
                {isAboveAvg && (
                  <span
                    className="text-[10px] font-medium"
                    style={{ color: "var(--accent)" }}
                  >
                    ↑ above avg
                  </span>
                )}
              </div>
            </div>

            <div className="w-full flex flex-col items-center justify-end h-32">
              {/* Shot count label */}
              <span
                className={`
                  text-xs font-semibold mb-1 transition-all duration-150
                  ${isActive ? "text-accent scale-110" : "text-theme-secondary"}
                `}
              >
                {item.shots > 0 ? item.shots : ""}
              </span>

              {/* Bar */}
              <div
                className={`
                  w-full max-w-12 rounded-t-lg transition-all duration-300 cursor-pointer
                  ${isActive ? "scale-105 shadow-lg" : ""}
                `}
                style={{
                  height: `${heightPercent}%`,
                  opacity: item.shots > 0 ? 1 : 0.2,
                  background: isActive
                    ? "linear-gradient(to top, var(--accent), var(--accent-light))"
                    : "linear-gradient(to top, var(--accent), var(--accent-light))",
                  boxShadow: isActive ? "0 0 15px var(--accent-glow)" : "none",
                  filter: isActive ? "brightness(1.1)" : "brightness(0.9)",
                }}
              />
            </div>

            {/* Day label */}
            <span
              className={`
                text-xs mt-2 font-medium transition-all duration-150
                ${isActive ? "text-accent" : "text-theme-muted"}
              `}
            >
              {item.day}
            </span>
          </div>
        );
      })}
    </div>
  );
}
