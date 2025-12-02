import { useState, useEffect, createContext, useContext, useCallback, ReactNode } from 'react';
import { cn } from '@/lib/utils';
import { CheckCircle2, XCircle, AlertCircle, X, Loader2 } from 'lucide-react';

export type ToastType = 'success' | 'error' | 'warning' | 'loading';

interface Toast {
  id: number;
  type: ToastType;
  message: string;
  duration?: number;
}

interface ToastContextType {
  toast: (type: ToastType, message: string, duration?: number) => number;
  success: (message: string, duration?: number) => number;
  error: (message: string, duration?: number) => number;
  warning: (message: string, duration?: number) => number;
  loading: (message: string) => number;
  dismiss: (id: number) => void;
  update: (id: number, type: ToastType, message: string) => void;
}

const ToastContext = createContext<ToastContextType | null>(null);

let toastId = 0;

export function ToastProvider({ children }: { children: ReactNode }) {
  const [toasts, setToasts] = useState<Toast[]>([]);

  const dismiss = useCallback((id: number) => {
    setToasts((prev) => prev.filter((t) => t.id !== id));
  }, []);

  const toast = useCallback((type: ToastType, message: string, duration = 4000) => {
    const id = ++toastId;
    setToasts((prev) => [...prev, { id, type, message, duration }]);
    
    if (type !== 'loading' && duration > 0) {
      setTimeout(() => dismiss(id), duration);
    }
    
    return id;
  }, [dismiss]);

  const update = useCallback((id: number, type: ToastType, message: string) => {
    setToasts((prev) => 
      prev.map((t) => t.id === id ? { ...t, type, message } : t)
    );
    
    // Auto-dismiss after update (unless loading)
    if (type !== 'loading') {
      setTimeout(() => dismiss(id), 3000);
    }
  }, [dismiss]);

  const success = useCallback((message: string, duration?: number) => 
    toast('success', message, duration), [toast]);
  
  const error = useCallback((message: string, duration?: number) => 
    toast('error', message, duration ?? 6000), [toast]);
  
  const warning = useCallback((message: string, duration?: number) => 
    toast('warning', message, duration ?? 5000), [toast]);
  
  const loading = useCallback((message: string) => 
    toast('loading', message, 0), [toast]);

  return (
    <ToastContext.Provider value={{ toast, success, error, warning, loading, dismiss, update }}>
      {children}
      <ToastContainer toasts={toasts} onDismiss={dismiss} />
    </ToastContext.Provider>
  );
}

export function useToast() {
  const context = useContext(ToastContext);
  if (!context) {
    throw new Error('useToast must be used within ToastProvider');
  }
  return context;
}

function ToastContainer({ toasts, onDismiss }: { toasts: Toast[]; onDismiss: (id: number) => void }) {
  if (toasts.length === 0) return null;

  return (
    <div className="fixed bottom-4 right-4 z-[100] flex flex-col gap-2 max-w-sm w-full pointer-events-none">
      {toasts.map((toast) => (
        <ToastItem key={toast.id} toast={toast} onDismiss={() => onDismiss(toast.id)} />
      ))}
    </div>
  );
}

function ToastItem({ toast, onDismiss }: { toast: Toast; onDismiss: () => void }) {
  const [isVisible, setIsVisible] = useState(false);
  
  useEffect(() => {
    // Trigger enter animation
    requestAnimationFrame(() => setIsVisible(true));
  }, []);

  const icons = {
    success: <CheckCircle2 className="w-5 h-5 text-emerald-500" />,
    error: <XCircle className="w-5 h-5 text-red-500" />,
    warning: <AlertCircle className="w-5 h-5 text-amber-500" />,
    loading: <Loader2 className="w-5 h-5 text-accent animate-spin" />,
  };

  const bgColors = {
    success: 'bg-emerald-500/10 border-emerald-500/20',
    error: 'bg-red-500/10 border-red-500/20',
    warning: 'bg-amber-500/10 border-amber-500/20',
    loading: 'bg-accent/10 border-accent/20',
  };

  return (
    <div
      className={cn(
        'flex items-center gap-3 px-4 py-3 rounded-xl border backdrop-blur-sm shadow-lg pointer-events-auto',
        'transform transition-all duration-300 ease-out',
        bgColors[toast.type],
        isVisible ? 'translate-x-0 opacity-100' : 'translate-x-8 opacity-0'
      )}
    >
      {icons[toast.type]}
      <span className="flex-1 text-sm font-medium text-theme">{toast.message}</span>
      {toast.type !== 'loading' && (
        <button
          onClick={onDismiss}
          className="p-1 rounded-lg hover:bg-white/10 text-theme-muted transition-colors"
        >
          <X className="w-4 h-4" />
        </button>
      )}
    </div>
  );
}

