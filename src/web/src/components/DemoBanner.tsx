import { Play, X } from 'lucide-react';
import { useThemeStore } from '@/lib/themeStore';

interface DemoBannerProps {
  onExit?: () => void;
}

export function DemoBanner({ onExit }: DemoBannerProps) {
  const { theme } = useThemeStore();
  const isDark = theme.isDark;

  const handleExit = () => {
    if (onExit) {
      onExit();
    } else {
      // Remove demo param and reload
      const url = new URL(window.location.href);
      url.searchParams.delete('demo');
      window.location.href = url.pathname;
    }
  };

  return (
    <div 
      className={`
        border-b px-4 py-2.5
        ${isDark 
          ? 'bg-gradient-to-r from-violet-500/20 via-purple-500/15 to-violet-500/20 border-violet-400/30' 
          : 'bg-gradient-to-r from-violet-600/15 via-purple-600/10 to-violet-600/15 border-violet-500/40'
        }
      `}
    >
      <div className="max-w-6xl mx-auto flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div 
            className={`
              flex items-center gap-2 px-2.5 py-1 rounded-full
              ${isDark 
                ? 'bg-violet-500/25 border border-violet-400/30' 
                : 'bg-violet-600/20 border border-violet-500/30'
              }
            `}
          >
            <Play 
              className={`w-3.5 h-3.5 ${isDark ? 'text-violet-300 fill-violet-300' : 'text-violet-600 fill-violet-600'}`} 
            />
            <span 
              className={`text-xs font-bold uppercase tracking-wide ${isDark ? 'text-violet-200' : 'text-violet-700'}`}
            >
              Demo Mode
            </span>
          </div>
          <span 
            className={`text-sm hidden sm:inline ${isDark ? 'text-violet-300/90' : 'text-violet-600/90'}`}
          >
            Explore BrewOS with simulated machine data
          </span>
        </div>
        <button
          onClick={handleExit}
          className={`
            flex items-center gap-1.5 text-sm transition-colors group
            ${isDark 
              ? 'text-violet-300/80 hover:text-violet-100' 
              : 'text-violet-600/80 hover:text-violet-800'
            }
          `}
        >
          <span className="hidden sm:inline font-medium">Exit Demo</span>
          <X className="w-4 h-4 group-hover:rotate-90 transition-transform" />
        </button>
      </div>
    </div>
  );
}

