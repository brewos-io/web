import {
  Coffee,
  Wifi,
  Scale as ScaleIcon,
  Cloud,
  Globe,
  Palette,
  Server,
  Info,
} from "lucide-react";

export type SettingsTab =
  | "machine"
  | "network"
  | "scale"
  | "cloud"
  | "regional"
  | "appearance"
  | "system"
  | "about";

export interface TabConfig {
  id: SettingsTab;
  label: string;
  icon: React.ComponentType<{ className?: string }>;
}

export const getSettingsTabs = (isCloud: boolean, isDemo: boolean = false): TabConfig[] => {
  const tabs: TabConfig[] = [
    { id: "machine", label: "Machine", icon: Coffee },
    { id: "scale", label: "Scale", icon: ScaleIcon },
    { id: "network", label: "Network", icon: Wifi },
  ];

  // Show Cloud tab in local mode or demo mode
  if (!isCloud || isDemo) {
    tabs.push({ id: "cloud", label: "Cloud", icon: Cloud });
  }

  tabs.push(
    { id: "appearance", label: "Theme", icon: Palette },
    { id: "regional", label: "Regional", icon: Globe },
    { id: "system", label: "System", icon: Server },
    { id: "about", label: "About", icon: Info }
  );

  return tabs;
};
