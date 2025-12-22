import type { Meta, StoryObj } from "@storybook/react";
import { TemperatureGauges } from "./TemperatureGauges";

const meta: Meta<typeof TemperatureGauges> = {
  title: "Dashboard/TemperatureGauges",
  component: TemperatureGauges,
  tags: ["autodocs"],
  argTypes: {
    machineType: {
      control: "select",
      options: ["dual_boiler", "single_boiler", "heat_exchanger", undefined],
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
type Story = StoryObj<typeof TemperatureGauges>;

// =============================================================================
// DUAL BOILER STORIES
// =============================================================================

export const DualBoiler_Ready: Story = {
  name: "Dual Boiler / Ready",
  args: {
    machineType: "dual_boiler",
    brewTemp: { current: 93.5, setpoint: 93.5, max: 105 },
    steamTemp: { current: 145, setpoint: 145, max: 160 },
    groupTemp: 88,
  },
};

export const DualBoiler_Heating: Story = {
  name: "Dual Boiler / Heating",
  args: {
    machineType: "dual_boiler",
    brewTemp: { current: 65, setpoint: 93.5, max: 105 },
    steamTemp: { current: 80, setpoint: 145, max: 160 },
    groupTemp: 45,
  },
};

export const DualBoiler_BrewOnly: Story = {
  name: "Dual Boiler / Brew Only Mode",
  args: {
    machineType: "dual_boiler",
    brewTemp: { current: 93.5, setpoint: 93.5, max: 105 },
    steamTemp: { current: 35, setpoint: 0, max: 160 },
    groupTemp: 88,
  },
};

export const DualBoiler_Cold: Story = {
  name: "Dual Boiler / Cold",
  args: {
    machineType: "dual_boiler",
    brewTemp: { current: 25, setpoint: 93.5, max: 105 },
    steamTemp: { current: 25, setpoint: 145, max: 160 },
    groupTemp: 22,
  },
};

// =============================================================================
// SINGLE BOILER STORIES
// =============================================================================

export const SingleBoiler_BrewMode_Ready: Story = {
  name: "Single Boiler / Brew Mode Ready",
  args: {
    machineType: "single_boiler",
    brewTemp: { current: 93, setpoint: 93, max: 105 },
    steamTemp: { current: 0, setpoint: 0, max: 160 },
    groupTemp: 0,
  },
};

export const SingleBoiler_BrewMode_Heating: Story = {
  name: "Single Boiler / Brew Mode Heating",
  args: {
    machineType: "single_boiler",
    brewTemp: { current: 55, setpoint: 93, max: 105 },
    steamTemp: { current: 0, setpoint: 0, max: 160 },
    groupTemp: 0,
  },
};

export const SingleBoiler_SteamMode_Ready: Story = {
  name: "Single Boiler / Steam Mode Ready",
  args: {
    machineType: "single_boiler",
    brewTemp: { current: 140, setpoint: 140, max: 160 },
    steamTemp: { current: 0, setpoint: 0, max: 160 },
    groupTemp: 0,
  },
};

export const SingleBoiler_SteamMode_Heating: Story = {
  name: "Single Boiler / Steam Mode Heating",
  args: {
    machineType: "single_boiler",
    brewTemp: { current: 110, setpoint: 140, max: 160 },
    steamTemp: { current: 0, setpoint: 0, max: 160 },
    groupTemp: 0,
  },
};

export const SingleBoiler_Cold: Story = {
  name: "Single Boiler / Cold",
  args: {
    machineType: "single_boiler",
    brewTemp: { current: 22, setpoint: 93, max: 105 },
    steamTemp: { current: 0, setpoint: 0, max: 160 },
    groupTemp: 0,
  },
};

// =============================================================================
// HEAT EXCHANGER STORIES
// =============================================================================

export const HeatExchanger_Ready: Story = {
  name: "Heat Exchanger / Ready",
  args: {
    machineType: "heat_exchanger",
    brewTemp: { current: 0, setpoint: 0, max: 105 },
    steamTemp: { current: 125, setpoint: 125, max: 160 },
    groupTemp: 93,
  },
};

export const HeatExchanger_Heating: Story = {
  name: "Heat Exchanger / Heating",
  args: {
    machineType: "heat_exchanger",
    brewTemp: { current: 0, setpoint: 0, max: 105 },
    steamTemp: { current: 80, setpoint: 125, max: 160 },
    groupTemp: 55,
  },
};

export const HeatExchanger_GroupHot: Story = {
  name: "Heat Exchanger / Group Too Hot (Flush Needed)",
  args: {
    machineType: "heat_exchanger",
    brewTemp: { current: 0, setpoint: 0, max: 105 },
    steamTemp: { current: 125, setpoint: 125, max: 160 },
    groupTemp: 98,
  },
};

export const HeatExchanger_Cold: Story = {
  name: "Heat Exchanger / Cold",
  args: {
    machineType: "heat_exchanger",
    brewTemp: { current: 0, setpoint: 0, max: 105 },
    steamTemp: { current: 25, setpoint: 125, max: 160 },
    groupTemp: 22,
  },
};

// =============================================================================
// UNKNOWN MACHINE TYPE STORIES
// =============================================================================

export const Unknown_Ready: Story = {
  name: "Unknown / Ready (Shows Both)",
  args: {
    machineType: undefined,
    brewTemp: { current: 93, setpoint: 93, max: 105 },
    steamTemp: { current: 145, setpoint: 145, max: 160 },
    groupTemp: 88,
  },
};

export const Unknown_Heating: Story = {
  name: "Unknown / Heating",
  args: {
    machineType: undefined,
    brewTemp: { current: 50, setpoint: 93, max: 105 },
    steamTemp: { current: 60, setpoint: 145, max: 160 },
    groupTemp: 40,
  },
};

