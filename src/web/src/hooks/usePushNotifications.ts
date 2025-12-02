import { useEffect, useState, useCallback } from 'react';
import {
  registerServiceWorker,
  subscribeToPush,
  unsubscribeFromPush,
  getPushSubscription,
  registerPushSubscription,
  unregisterPushSubscription,
} from '@/lib/push-notifications';
import { useAppStore } from '@/lib/mode';
import { isDemoMode } from '@/lib/demo-mode';

export interface PushNotificationState {
  isSupported: boolean;
  isRegistered: boolean;
  isSubscribed: boolean;
  permission: NotificationPermission;
  registration: ServiceWorkerRegistration | null;
  subscription: PushSubscription | null;
}

/**
 * Hook to manage push notifications
 */
export function usePushNotifications() {
  const isDemo = isDemoMode();
  
  const [state, setState] = useState<PushNotificationState>({
    isSupported: isDemo ? true : false,
    isRegistered: isDemo ? true : false,
    isSubscribed: isDemo ? true : false, // Enabled by default in demo
    permission: isDemo ? 'granted' : 'default',
    registration: null,
    subscription: null,
  });

  const { user, getSelectedDevice } = useAppStore();

  // Check support and initialize
  useEffect(() => {
    // Demo mode: simulate supported state
    if (isDemo) {
      setState((prev) => ({
        ...prev,
        isSupported: true,
        isRegistered: true,
        permission: 'granted',
      }));
      return;
    }

    const isSupported =
      'serviceWorker' in navigator && 'PushManager' in window && 'Notification' in window;

    setState((prev) => ({
      ...prev,
      isSupported,
      permission: 'Notification' in window ? Notification.permission : 'denied',
    }));

    if (!isSupported) {
      return;
    }

    // Register service worker and check subscription
    const init = async () => {
      const registration = await registerServiceWorker();
      if (!registration) {
        return;
      }

      const subscription = await getPushSubscription(registration);

      setState((prev) => ({
        ...prev,
        isRegistered: true,
        registration,
        isSubscribed: subscription !== null,
        subscription: subscription || null,
      }));
    };

    init();
  }, [isDemo]);

  // Subscribe to push notifications
  const subscribe = useCallback(async (): Promise<boolean> => {
    // Demo mode: simulate successful subscription
    if (isDemo) {
      await new Promise(r => setTimeout(r, 500)); // Simulate delay
      setState((prev) => ({
        ...prev,
        isSubscribed: true,
      }));
      return true;
    }

    if (!state.registration) {
      console.error('[PWA] Service worker not registered');
      return false;
    }

    const subscription = await subscribeToPush(state.registration);
    if (!subscription) {
      return false;
    }

    // Register with server
    const selectedDevice = getSelectedDevice();
    const success = await registerPushSubscription(
      subscription,
      user?.id,
      selectedDevice?.id
    );

    if (success) {
      setState((prev) => ({
        ...prev,
        isSubscribed: true,
        subscription,
      }));
    }

    return success;
  }, [isDemo, state.registration, user, getSelectedDevice]);

  // Unsubscribe from push notifications
  const unsubscribe = useCallback(async (): Promise<boolean> => {
    // Demo mode: simulate successful unsubscription
    if (isDemo) {
      await new Promise(r => setTimeout(r, 300)); // Simulate delay
      setState((prev) => ({
        ...prev,
        isSubscribed: false,
      }));
      return true;
    }

    if (!state.registration || !state.subscription) {
      return false;
    }

    // Unregister from server first
    await unregisterPushSubscription(state.subscription);

    // Then unsubscribe locally
    const success = await unsubscribeFromPush(state.registration);

    if (success) {
      setState((prev) => ({
        ...prev,
        isSubscribed: false,
        subscription: null,
      }));
    }

    return success;
  }, [isDemo, state.registration, state.subscription]);

  // Update subscription when device changes (skip in demo mode)
  useEffect(() => {
    if (isDemo) return;
    
    if (state.isSubscribed && state.subscription && user) {
      const selectedDevice = getSelectedDevice();
      registerPushSubscription(state.subscription, user.id, selectedDevice?.id).catch(
        (error) => {
          console.error('[PWA] Failed to update push subscription:', error);
        }
      );
    }
  }, [isDemo, state.isSubscribed, state.subscription, user, getSelectedDevice]);

  return {
    ...state,
    subscribe,
    unsubscribe,
  };
}

