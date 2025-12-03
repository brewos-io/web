import { Button } from "@/components/Button";
import { formatDate } from "@/lib/date";

export interface MaintenanceCardProps {
  label: string;
  description?: string;
  shotsSince: number;
  lastTimestamp: number;
  threshold: number;
  warningThreshold?: number;
  icon?: React.ReactNode;
  onMark: () => void;
}

type MaintenanceStatus = "good" | "warning" | "overdue";

function getStatus(shots: number, threshold: number, warningThreshold?: number): MaintenanceStatus {
  const warning = warningThreshold ?? Math.floor(threshold * 0.8);
  if (shots >= threshold) return "overdue";
  if (shots >= warning) return "warning";
  return "good";
}

const statusConfig = {
  good: {
    gradient: "from-emerald-500/10 via-emerald-500/5 to-transparent",
    borderColor: "border-emerald-500/20",
    accentColor: "#10b981",
    textClass: "text-emerald-500",
    label: "OK",
    iconBg: "bg-emerald-500/15",
  },
  warning: {
    gradient: "from-amber-500/15 via-amber-500/5 to-transparent",
    borderColor: "border-amber-500/30",
    accentColor: "#f59e0b",
    textClass: "text-amber-500",
    label: "Due Soon",
    iconBg: "bg-amber-500/15",
  },
  overdue: {
    gradient: "from-red-500/15 via-red-500/5 to-transparent",
    borderColor: "border-red-500/30",
    accentColor: "#ef4444",
    textClass: "text-red-500",
    label: "Overdue",
    iconBg: "bg-red-500/15",
  },
};

export function MaintenanceCard({
  label,
  description,
  shotsSince,
  lastTimestamp,
  threshold,
  warningThreshold,
  icon,
  onMark,
}: MaintenanceCardProps) {
  const status = getStatus(shotsSince, threshold, warningThreshold);
  const config = statusConfig[status];
  const progress = Math.min((shotsSince / threshold) * 100, 100);
  const lastDate = lastTimestamp > 0 ? formatDate(lastTimestamp, { dateStyle: "short" }) : "Never";
  const remaining = Math.max(threshold - shotsSince, 0);

  return (
    <div 
      className={`relative overflow-hidden rounded-2xl border ${config.borderColor}`}
      style={{ backgroundColor: 'var(--color-bg-surface)' }}
    >
      {/* Gradient background */}
      <div className={`absolute inset-0 bg-gradient-to-br ${config.gradient} pointer-events-none`} />
      
      {/* Content */}
      <div className="relative p-5">
        {/* Header */}
        <div className="flex items-start justify-between mb-4">
          <div className="flex items-center gap-3">
            {icon && (
              <div className={`w-10 h-10 rounded-xl ${config.iconBg} flex items-center justify-center`}>
                <span style={{ color: config.accentColor }}>{icon}</span>
              </div>
            )}
            <div>
              <h3 className="font-semibold text-theme-primary">{label}</h3>
              {description && (
                <p className="text-xs text-theme-muted mt-0.5">{description}</p>
              )}
            </div>
          </div>
          
          {/* Status badge */}
          <div 
            className="flex items-center gap-1.5 px-2.5 py-1 rounded-full text-xs font-medium"
            style={{ 
              backgroundColor: `${config.accentColor}15`,
              color: config.accentColor,
            }}
          >
            <div 
              className="w-1.5 h-1.5 rounded-full animate-pulse"
              style={{ backgroundColor: config.accentColor }}
            />
            {config.label}
          </div>
        </div>

        {/* Stats row */}
        <div className="flex items-baseline gap-2 mb-1">
          <span 
            className="text-4xl font-bold tabular-nums"
            style={{ color: config.accentColor }}
          >
            {shotsSince}
          </span>
          <span className="text-sm text-theme-muted">/ {threshold} shots</span>
        </div>
        
        {/* Remaining text */}
        <p className="text-xs text-theme-muted mb-4">
          {remaining > 0 
            ? `${remaining} shots until maintenance`
            : `${shotsSince - threshold} shots overdue`
          }
        </p>

        {/* Progress bar */}
        <div 
          className="h-2 rounded-full mb-4 overflow-hidden"
          style={{ backgroundColor: 'var(--color-border)' }}
        >
          <div
            className="h-full rounded-full transition-all duration-500"
            style={{ 
              width: `${progress}%`,
              backgroundColor: config.accentColor,
              boxShadow: `0 0 8px ${config.accentColor}40`,
            }}
          />
        </div>

        {/* Footer */}
        <div className="flex items-center justify-between">
          <div className="text-xs text-theme-muted">
            <span className="opacity-70">Last:</span>{" "}
            <span className="font-medium text-theme-secondary">{lastDate}</span>
          </div>
          <Button
            size="sm"
            variant={status === "overdue" ? "primary" : "secondary"}
            onClick={onMark}
          >
            Mark Done
          </Button>
        </div>
      </div>
    </div>
  );
}
