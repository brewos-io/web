import { useState, useEffect } from "react";
import { RefreshCw, X } from "lucide-react";
import { setUpdateCallback, activateNewServiceWorker } from "@/lib/push-notifications";

export function UpdateNotification() {
  const [updateAvailable, setUpdateAvailable] = useState(false);
  const [dismissed, setDismissed] = useState(false);

  useEffect(() => {
    // Register callback for when new version is available
    setUpdateCallback(() => {
      setUpdateAvailable(true);
      setDismissed(false);
    });
  }, []);

  const handleUpdate = () => {
    // Tell SW to skip waiting and activate
    activateNewServiceWorker();
    // Page will auto-reload via controllerchange event
  };

  const handleDismiss = () => {
    setDismissed(true);
  };

  if (!updateAvailable || dismissed) {
    return null;
  }

  return (
    <div className="fixed bottom-4 left-4 right-4 md:left-auto md:right-4 md:w-96 z-50 animate-slide-up">
      <div className="bg-accent text-white rounded-xl shadow-2xl p-4 flex items-center gap-3">
        <div className="flex-shrink-0 w-10 h-10 bg-white/20 rounded-lg flex items-center justify-center">
          <RefreshCw className="w-5 h-5" />
        </div>
        <div className="flex-1 min-w-0">
          <p className="font-semibold text-sm">Update Available</p>
          <p className="text-xs text-white/80">A new version of BrewOS is ready</p>
        </div>
        <button
          onClick={handleUpdate}
          className="flex-shrink-0 px-4 py-2 bg-white text-accent font-semibold text-sm rounded-lg hover:bg-white/90 transition-colors"
        >
          Update
        </button>
        <button
          onClick={handleDismiss}
          className="flex-shrink-0 p-1.5 hover:bg-white/20 rounded-lg transition-colors"
          aria-label="Dismiss"
        >
          <X className="w-4 h-4" />
        </button>
      </div>
    </div>
  );
}

