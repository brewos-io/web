import { Logo } from "@/components/Logo";
import { Coffee, Gauge, Clock, Wifi } from "lucide-react";

// Feature highlight card
function FeatureCard({
  icon: Icon,
  title,
  description,
  animationClass,
}: {
  icon: React.ElementType;
  title: string;
  description: string;
  animationClass?: string;
}) {
  return (
    <div
      className={`flex items-start gap-4 ${animationClass ? `opacity-0 ${animationClass}` : ""}`}
    >
      <div className="flex-shrink-0 w-10 h-10 rounded-xl bg-white/10 backdrop-blur-sm flex items-center justify-center">
        <Icon className="w-5 h-5 text-accent-light" />
      </div>
      <div>
        <h3 className="text-white font-semibold text-sm mb-1">{title}</h3>
        <p className="text-white/60 text-xs leading-relaxed">{description}</p>
      </div>
    </div>
  );
}

// Floating orbs for ambient background animation
function FloatingOrb({
  size,
  x,
  y,
  animationClass,
}: {
  size: number;
  x: number;
  y: number;
  animationClass?: string;
}) {
  return (
    <div
      className={`absolute rounded-full blur-3xl opacity-20 ${animationClass || ""}`}
      style={{
        width: size,
        height: size,
        left: `${x}%`,
        top: `${y}%`,
        background:
          "radial-gradient(circle, var(--accent-glow) 0%, transparent 70%)",
      }}
    />
  );
}

interface LoginHeroProps {
  /** Enable entrance animations */
  animated?: boolean;
}

export function LoginHero({ animated = true }: LoginHeroProps) {
  return (
    <div className="hidden lg:flex lg:w-1/2 xl:w-[55%] relative bg-gradient-to-br from-coffee-900 via-coffee-800 to-coffee-900 overflow-hidden">
      {/* Animated background orbs */}
      <FloatingOrb
        size={400}
        x={-10}
        y={20}
        animationClass={animated ? "login-orb" : undefined}
      />
      <FloatingOrb
        size={300}
        x={60}
        y={60}
        animationClass={animated ? "login-orb-slow" : undefined}
      />
      <FloatingOrb
        size={250}
        x={30}
        y={-5}
        animationClass={animated ? "login-orb-slower" : undefined}
      />

      {/* Subtle grid pattern overlay */}
      <div
        className="absolute inset-0 opacity-[0.03]"
        style={{
          backgroundImage: `
            linear-gradient(rgba(255,255,255,0.1) 1px, transparent 1px),
            linear-gradient(90deg, rgba(255,255,255,0.1) 1px, transparent 1px)
          `,
          backgroundSize: "60px 60px",
        }}
      />

      {/* Content */}
      <div className="relative z-10 flex flex-col justify-between p-12 xl:p-16 w-full">
        {/* Top - Logo */}
        <div className={animated ? "login-hero-content" : ""}>
          <Logo size="lg" forceLight />
        </div>

        {/* Center - Hero messaging */}
        <div
          className={`flex-1 flex flex-col justify-center max-w-lg ${animated ? "login-hero-content" : ""}`}
        >
          <h1 className="text-4xl xl:text-5xl font-bold text-white mb-6 leading-tight">
            Craft the perfect
            <span className="block text-accent-light">espresso experience</span>
          </h1>
          <p className="text-white/60 text-lg mb-10 leading-relaxed">
            Take complete control of your espresso machine. Monitor temperature,
            pressure, and timing—from anywhere in the world.
          </p>

          {/* Feature highlights */}
          <div className="space-y-5">
            <FeatureCard
              icon={Gauge}
              title="Precision Temperature Control"
              description="PID-controlled heating with 0.1°C accuracy for consistent shots"
              animationClass={animated ? "login-feature-1" : undefined}
            />
            <FeatureCard
              icon={Clock}
              title="Smart Scheduling"
              description="Wake up to a pre-heated machine, ready when you are"
              animationClass={animated ? "login-feature-2" : undefined}
            />
            <FeatureCard
              icon={Wifi}
              title="Remote Monitoring"
              description="Keep an eye on your machine from anywhere via cloud"
              animationClass={animated ? "login-feature-3" : undefined}
            />
          </div>
        </div>

        {/* Bottom - Social proof / tagline */}
        <div className={animated ? "opacity-0 login-tagline" : ""}>
          <div className="flex items-center gap-3 text-white/40 text-sm">
            <Coffee className="w-4 h-4" />
            <span>Trusted by home baristas worldwide</span>
          </div>
        </div>
      </div>

      {/* Decorative coffee cup silhouette */}
      <div className="absolute right-0 bottom-0 w-80 h-80 xl:w-96 xl:h-96 opacity-5">
        <svg
          viewBox="0 0 200 200"
          fill="currentColor"
          className="text-white w-full h-full"
        >
          <path d="M40 140 Q40 180 80 180 L120 180 Q160 180 160 140 L160 80 L180 80 Q200 80 200 100 L200 120 Q200 140 180 140 L160 140 L160 140 Q160 180 120 180 L80 180 Q40 180 40 140 Z M40 60 L160 60 L160 80 L40 80 Z" />
        </svg>
      </div>
    </div>
  );
}

