import { useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { getSupabase, isSupabaseConfigured } from '@/lib/supabase';
import { Loader2 } from 'lucide-react';

export function AuthCallback() {
  const navigate = useNavigate();

  useEffect(() => {
    // Handle the OAuth callback
    const handleCallback = async () => {
      if (!isSupabaseConfigured) {
        navigate('/');
        return;
      }
      
      const { error } = await getSupabase().auth.getSession();
      
      if (error) {
        console.error('Auth callback error:', error);
        navigate('/login');
        return;
      }

      // Check for pairing redirect
      const params = new URLSearchParams(window.location.search);
      const deviceId = params.get('device');
      const token = params.get('token');

      if (deviceId && token) {
        // Redirect to pair page with device info
        navigate(`/pair?id=${deviceId}&token=${token}`);
      } else {
        // Normal login - go to devices
        navigate('/devices');
      }
    };

    handleCallback();
  }, [navigate]);

  return (
    <div className="min-h-screen bg-cream-100 flex items-center justify-center">
      <div className="text-center">
        <Loader2 className="w-8 h-8 animate-spin text-accent mx-auto mb-4" />
        <p className="text-coffee-500">Signing in...</p>
      </div>
    </div>
  );
}

