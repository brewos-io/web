import { useEffect } from "react";
import { useNavigate, useLocation } from "react-router-dom";
import { GoogleLogin, CredentialResponse } from "@react-oauth/google";
import { LogoIcon } from "@/components/Logo";
import { useAuth } from "@/lib/auth";
import { isGoogleAuthConfigured } from "@/lib/auth";
import { useThemeStore } from "@/lib/themeStore";
import { ChevronRight } from "lucide-react";
import { LoginHero, LoginForm } from "@/components/login";

export function Login() {
  const navigate = useNavigate();
  const location = useLocation();
  const { user, handleGoogleLogin } = useAuth();
  const { theme } = useThemeStore();
  const isDark = theme.isDark;

  const error = (location.state as { error?: string })?.error;

  useEffect(() => {
    if (user) {
      navigate("/machines");
    }
  }, [user, navigate]);

  if (!isGoogleAuthConfigured) {
    return (
      <div className="full-page bg-gradient-to-br from-coffee-800 to-coffee-900 flex items-center justify-center p-4">
        <div className="w-full max-w-md text-center p-8 rounded-3xl bg-white/5 backdrop-blur-xl border border-white/10 login-card-enter">
          <div className="flex justify-center mb-6">
            <LogoIcon size="xl" />
          </div>
          <h1 className="text-2xl font-bold text-white mb-3">
            Connect Locally
          </h1>
          <p className="text-white/60 mb-6">
            Cloud features are not configured. Connect directly to your device.
          </p>
          <a
            href="http://brewos.local"
            className="inline-flex items-center gap-2 px-6 py-3 rounded-xl bg-accent text-white font-semibold hover:bg-accent-light transition-all duration-300 shadow-lg shadow-accent/30"
          >
            Go to brewos.local
            <ChevronRight className="w-4 h-4" />
          </a>
        </div>
      </div>
    );
  }

  return (
    <div className="full-page flex overflow-hidden">
      {/* Left Panel - Hero/Brand Section (hidden on mobile) */}
      <LoginHero animated />

      {/* Right Panel - Login Form */}
      <div className="w-full lg:w-1/2 xl:w-[45%] flex items-center justify-center bg-theme p-6 sm:p-8 relative overflow-hidden">
        {/* Subtle decorative elements */}
        <div className="absolute top-0 right-0 w-64 h-64 bg-gradient-to-bl from-accent/5 to-transparent rounded-full blur-3xl" />
        <div className="absolute bottom-0 left-0 w-96 h-96 bg-gradient-to-tr from-accent/5 to-transparent rounded-full blur-3xl" />

        <LoginForm
          error={error}
          animated
          googleButton={
            <GoogleLogin
              onSuccess={(response: CredentialResponse) => {
                if (response.credential) {
                  handleGoogleLogin(response.credential);
                }
              }}
              onError={() => {
                console.error("Google login failed");
              }}
              theme={isDark ? "filled_black" : "outline"}
              size="large"
              width="320"
              text="continue_with"
              shape="pill"
            />
          }
        />
      </div>
    </div>
  );
}
