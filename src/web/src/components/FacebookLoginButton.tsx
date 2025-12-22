import { useEffect, useCallback, useState } from "react";
import { FACEBOOK_APP_ID } from "@/lib/auth";

declare global {
  interface Window {
    FB: {
      init: (params: {
        appId: string;
        cookie: boolean;
        xfbml: boolean;
        version: string;
      }) => void;
      login: (
        callback: (response: FacebookLoginResponse) => void,
        options: { scope: string }
      ) => void;
      getLoginStatus: (
        callback: (response: FacebookLoginResponse) => void
      ) => void;
    };
    fbAsyncInit: () => void;
  }
}

interface FacebookLoginResponse {
  status: string;
  authResponse?: {
    accessToken: string;
    userID: string;
  };
}

interface FacebookLoginButtonProps {
  onSuccess: (accessToken: string) => void;
  onError?: () => void;
  theme?: "light" | "dark";
}

export function FacebookLoginButton({
  onSuccess,
  onError,
  theme = "light",
}: FacebookLoginButtonProps) {
  const [sdkLoaded, setSdkLoaded] = useState(false);
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    // Don't load SDK if not configured
    if (!FACEBOOK_APP_ID) return;

    // Load Facebook SDK
    if (!document.getElementById("facebook-jssdk")) {
      window.fbAsyncInit = () => {
        window.FB.init({
          appId: FACEBOOK_APP_ID,
          cookie: true,
          xfbml: true,
          version: "v21.0",
        });
        setSdkLoaded(true);
      };

      const script = document.createElement("script");
      script.id = "facebook-jssdk";
      script.src = "https://connect.facebook.net/en_US/sdk.js";
      script.async = true;
      script.defer = true;
      document.body.appendChild(script);
    } else if (window.FB) {
      setSdkLoaded(true);
    }
  }, []);

  const handleClick = useCallback(() => {
    if (!sdkLoaded || !window.FB) {
      console.error("[Facebook] SDK not loaded yet");
      onError?.();
      return;
    }

    setLoading(true);
    window.FB.login(
      (response) => {
        setLoading(false);
        if (
          response.status === "connected" &&
          response.authResponse?.accessToken
        ) {
          onSuccess(response.authResponse.accessToken);
        } else {
          console.error("[Facebook] Login failed:", response.status);
          onError?.();
        }
      },
      { scope: "email,public_profile" }
    );
  }, [sdkLoaded, onSuccess, onError]);

  if (!FACEBOOK_APP_ID) return null;

  const isDark = theme === "dark";

  return (
    <button
      onClick={handleClick}
      disabled={!sdkLoaded || loading}
      className={`flex items-center justify-center gap-3 w-[280px] h-10 rounded-full font-medium text-sm transition-all duration-200 ${
        isDark
          ? "bg-[#1877F2] text-white hover:bg-[#166FE5] disabled:bg-[#1877F2]/70"
          : "bg-white text-[#1877F2] border border-gray-300 hover:bg-gray-50 disabled:opacity-70"
      } ${loading ? "cursor-wait" : "cursor-pointer"} disabled:cursor-not-allowed`}
    >
      <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 24 24">
        <path d="M24 12.073c0-6.627-5.373-12-12-12s-12 5.373-12 12c0 5.99 4.388 10.954 10.125 11.854v-8.385H7.078v-3.47h3.047V9.43c0-3.007 1.792-4.669 4.533-4.669 1.312 0 2.686.235 2.686.235v2.953H15.83c-1.491 0-1.956.925-1.956 1.874v2.25h3.328l-.532 3.47h-2.796v8.385C19.612 23.027 24 18.062 24 12.073z" />
      </svg>
      {loading ? "Connecting..." : "Continue with Facebook"}
    </button>
  );
}

