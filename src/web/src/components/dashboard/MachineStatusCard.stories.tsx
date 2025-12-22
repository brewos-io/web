import type { Meta, StoryObj } from "@storybook/react";
import { MachineStatusCard } from "./MachineStatusCard";
import type { HeatingStrategy, MachineType } from "@/lib/types";

// We need to wrap the component since it uses store
// For storybook, we'll create a mock version
const MockMachineStatusCard = (props: {
  mode: string;
  state: string;
  machineType?: MachineType;
  heatingStrategy?: HeatingStrategy | null;
}) => {
  return (
    <MachineStatusCard
      {...props}
      onSetMode={(mode) => console.log("Set mode:", mode)}
      onQuickOn={() => console.log("Quick on")}
      onOpenStrategyModal={() => console.log("Open strategy modal")}
    />
  );
};

const meta: Meta<typeof MockMachineStatusCard> = {
  title: "Dashboard/MachineStatusCard",
  component: MockMachineStatusCard,
  tags: ["autodocs"],
  argTypes: {
    mode: {
      control: "select",
      options: ["standby", "on", "eco"],
    },
    state: {
      control: "select",
      options: [
        "standby",
        "idle",
        "heating",
        "ready",
        "brewing",
        "fault",
        "safe",
        "eco",
      ],
    },
    machineType: {
      control: "select",
      options: ["dual_boiler", "single_boiler", "heat_exchanger", ""],
      description: "Machine type affects available features",
    },
    heatingStrategy: {
      control: "select",
      options: [null, 0, 1, 2, 3],
      description:
        "0: Brew Only, 1: Sequential, 2: Parallel, 3: Smart",
    },
  },
  decorators: [
    (Story) => (
      <div className="max-w-2xl">
        <Story />
      </div>
    ),
  ],
};

export default meta;
type Story = StoryObj<typeof MockMachineStatusCard>;

// =============================================================================
// DUAL BOILER STORIES
// =============================================================================

export const DualBoiler_Standby: Story = {
  name: "Dual Boiler / Standby",
  args: {
    mode: "standby",
    state: "standby",
    machineType: "dual_boiler",
    heatingStrategy: null,
  },
};

export const DualBoiler_Heating_BrewOnly: Story = {
  name: "Dual Boiler / Heating (Brew Only)",
  args: {
    mode: "on",
    state: "heating",
    machineType: "dual_boiler",
    heatingStrategy: 0, // Brew Only
  },
};

export const DualBoiler_Heating_Sequential: Story = {
  name: "Dual Boiler / Heating (Sequential)",
  args: {
    mode: "on",
    state: "heating",
    machineType: "dual_boiler",
    heatingStrategy: 1, // Sequential
  },
};

export const DualBoiler_Heating_Parallel: Story = {
  name: "Dual Boiler / Heating (Parallel)",
  args: {
    mode: "on",
    state: "heating",
    machineType: "dual_boiler",
    heatingStrategy: 2, // Parallel
  },
};

export const DualBoiler_Heating_SmartStagger: Story = {
  name: "Dual Boiler / Heating (Smart Stagger)",
  args: {
    mode: "on",
    state: "heating",
    machineType: "dual_boiler",
    heatingStrategy: 3, // Smart Stagger
  },
};

export const DualBoiler_Ready_BrewOnly: Story = {
  name: "Dual Boiler / Ready (Brew Only)",
  args: {
    mode: "on",
    state: "ready",
    machineType: "dual_boiler",
    heatingStrategy: 0,
  },
};

export const DualBoiler_Ready_BrewSteam: Story = {
  name: "Dual Boiler / Ready (Brew & Steam)",
  args: {
    mode: "on",
    state: "ready",
    machineType: "dual_boiler",
    heatingStrategy: 2,
  },
};

export const DualBoiler_Brewing: Story = {
  name: "Dual Boiler / Brewing",
  args: {
    mode: "on",
    state: "brewing",
    machineType: "dual_boiler",
    heatingStrategy: 3,
  },
};

export const DualBoiler_EcoMode: Story = {
  name: "Dual Boiler / Eco Mode",
  args: {
    mode: "eco",
    state: "eco",
    machineType: "dual_boiler",
    heatingStrategy: 1,
  },
};

export const DualBoiler_SafeMode: Story = {
  name: "Dual Boiler / Safe Mode",
  args: {
    mode: "standby",
    state: "safe",
    machineType: "dual_boiler",
    heatingStrategy: null,
  },
};

export const DualBoiler_Fault: Story = {
  name: "Dual Boiler / Fault",
  args: {
    mode: "standby",
    state: "fault",
    machineType: "dual_boiler",
    heatingStrategy: null,
  },
};

export const DualBoiler_Offline: Story = {
  name: "Dual Boiler / Offline",
  args: {
    mode: "standby",
    state: "offline",
    machineType: "dual_boiler",
    heatingStrategy: null,
  },
};

// =============================================================================
// SINGLE BOILER STORIES
// =============================================================================

export const SingleBoiler_Standby: Story = {
  name: "Single Boiler / Standby",
  args: {
    mode: "standby",
    state: "standby",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

export const SingleBoiler_Heating: Story = {
  name: "Single Boiler / Heating",
  args: {
    mode: "on",
    state: "heating",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

export const SingleBoiler_Ready: Story = {
  name: "Single Boiler / Ready",
  args: {
    mode: "on",
    state: "ready",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

export const SingleBoiler_Brewing: Story = {
  name: "Single Boiler / Brewing",
  args: {
    mode: "on",
    state: "brewing",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

export const SingleBoiler_EcoMode: Story = {
  name: "Single Boiler / Eco Mode",
  args: {
    mode: "eco",
    state: "eco",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

export const SingleBoiler_SafeMode: Story = {
  name: "Single Boiler / Safe Mode",
  args: {
    mode: "standby",
    state: "safe",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

export const SingleBoiler_Fault: Story = {
  name: "Single Boiler / Fault",
  args: {
    mode: "standby",
    state: "fault",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

export const SingleBoiler_Offline: Story = {
  name: "Single Boiler / Offline",
  args: {
    mode: "standby",
    state: "offline",
    machineType: "single_boiler",
    heatingStrategy: null,
  },
};

// =============================================================================
// HEAT EXCHANGER STORIES
// =============================================================================

export const HeatExchanger_Standby: Story = {
  name: "Heat Exchanger / Standby",
  args: {
    mode: "standby",
    state: "standby",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

export const HeatExchanger_Heating: Story = {
  name: "Heat Exchanger / Heating",
  args: {
    mode: "on",
    state: "heating",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

export const HeatExchanger_Ready: Story = {
  name: "Heat Exchanger / Ready",
  args: {
    mode: "on",
    state: "ready",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

export const HeatExchanger_Brewing: Story = {
  name: "Heat Exchanger / Brewing",
  args: {
    mode: "on",
    state: "brewing",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

export const HeatExchanger_EcoMode: Story = {
  name: "Heat Exchanger / Eco Mode",
  args: {
    mode: "eco",
    state: "eco",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

export const HeatExchanger_SafeMode: Story = {
  name: "Heat Exchanger / Safe Mode",
  args: {
    mode: "standby",
    state: "safe",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

export const HeatExchanger_Fault: Story = {
  name: "Heat Exchanger / Fault",
  args: {
    mode: "standby",
    state: "fault",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

export const HeatExchanger_Offline: Story = {
  name: "Heat Exchanger / Offline",
  args: {
    mode: "standby",
    state: "offline",
    machineType: "heat_exchanger",
    heatingStrategy: null,
  },
};

// =============================================================================
// UNKNOWN / NOT CONFIGURED STORIES
// =============================================================================

export const Unknown_Standby: Story = {
  name: "Unknown Machine / Standby",
  args: {
    mode: "standby",
    state: "standby",
    machineType: "",
    heatingStrategy: null,
  },
};

export const Unknown_Ready: Story = {
  name: "Unknown Machine / Ready",
  args: {
    mode: "on",
    state: "ready",
    machineType: "",
    heatingStrategy: null,
  },
};

export const Unknown_Heating: Story = {
  name: "Unknown Machine / Heating",
  args: {
    mode: "on",
    state: "heating",
    machineType: undefined,
    heatingStrategy: null,
  },
};

