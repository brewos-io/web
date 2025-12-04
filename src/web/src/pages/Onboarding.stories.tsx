import type { Meta, StoryObj } from "@storybook/react";
import { StoryPageWrapper } from "@/components/storybook";
import {
  WelcomeStep,
  ScanStep,
  ManualStep,
  MachineNameStep,
  SuccessStep,
} from "@/components/onboarding";

// Wrapper component for stories
function OnboardingStoryWrapper({ children }: { children?: React.ReactNode }) {
  return <>{children}</>;
}

const meta = {
  title: "Pages/Onboarding/Onboarding",
  component: OnboardingStoryWrapper,
  tags: ["autodocs"],
  parameters: {
    layout: "fullscreen",
  },
} satisfies Meta<typeof OnboardingStoryWrapper>;

export default meta;
type Story = StoryObj<typeof meta>;

export const Welcome: Story = {
  render: () => (
    <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4">
      <StoryPageWrapper cardClassName="text-center w-full max-w-lg">
        <WelcomeStep />
      </StoryPageWrapper>
    </div>
  ),
};

export const ScanQRCode: Story = {
  render: () => (
    <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
      <StoryPageWrapper cardClassName="w-full max-w-lg">
        <ScanStep
          onScan={() => {}}
          onValidate={() => {}}
        />
      </StoryPageWrapper>
    </div>
  ),
};

export const ScanWithError: Story = {
  render: () => (
    <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
      <StoryPageWrapper cardClassName="w-full max-w-lg">
        <ScanStep
          error="Invalid QR code. Please try again."
          onScan={() => {}}
          onValidate={() => {}}
        />
      </StoryPageWrapper>
    </div>
  ),
};

export const ManualEntry: Story = {
  render: () => (
    <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
      <StoryPageWrapper cardClassName="w-full max-w-lg">
        <ManualStep
          claimCode="X6ST-AP3G"
          onClaimCodeChange={() => {}}
          onValidate={() => {}}
        />
      </StoryPageWrapper>
    </div>
  ),
};

export const ManualEntryWithError: Story = {
  render: () => (
    <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
      <StoryPageWrapper cardClassName="w-full max-w-lg">
        <ManualStep
          claimCode="X6ST-AP3G"
          error="Invalid or expired code"
          onClaimCodeChange={() => {}}
          onValidate={() => {}}
        />
      </StoryPageWrapper>
    </div>
  ),
};

export const MachineName: Story = {
  render: () => (
    <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
      <StoryPageWrapper cardClassName="w-full max-w-lg">
        <MachineNameStep
          deviceName="Kitchen Espresso"
          onDeviceNameChange={() => {}}
          onContinue={() => {}}
        />
      </StoryPageWrapper>
    </div>
  ),
};

export const Success: Story = {
  render: () => (
    <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
      <StoryPageWrapper cardClassName="text-center py-12 w-full max-w-lg">
        <SuccessStep deviceName="Kitchen Espresso" />
      </StoryPageWrapper>
    </div>
  ),
};

export const AllSteps: Story = {
  render: () => (
    <div className="space-y-8 p-4 bg-coffee-900">
      <div>
        <h3 className="text-cream-200 text-lg font-semibold mb-4 text-center">
          Step 1: Welcome
        </h3>
        <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4">
          <StoryPageWrapper cardClassName="text-center w-full max-w-lg">
            <WelcomeStep />
          </StoryPageWrapper>
        </div>
      </div>
      <div>
        <h3 className="text-cream-200 text-lg font-semibold mb-4 text-center">
          Step 2: Scan QR Code
        </h3>
        <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
          <StoryPageWrapper cardClassName="w-full max-w-lg">
            <ScanStep
              onScan={() => {}}
              onValidate={() => {}}
            />
          </StoryPageWrapper>
        </div>
      </div>
      <div>
        <h3 className="text-cream-200 text-lg font-semibold mb-4 text-center">
          Step 2 (Alt): Manual Entry
        </h3>
        <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
          <StoryPageWrapper cardClassName="w-full max-w-lg">
            <ManualStep
              claimCode="X6ST-AP3G"
              onClaimCodeChange={() => {}}
              onValidate={() => {}}
            />
          </StoryPageWrapper>
        </div>
      </div>
      <div>
        <h3 className="text-cream-200 text-lg font-semibold mb-4 text-center">
          Step 3: Name Your Machine
        </h3>
        <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
          <StoryPageWrapper cardClassName="w-full max-w-lg">
            <MachineNameStep
              deviceName="Kitchen Espresso"
              onDeviceNameChange={() => {}}
              onContinue={() => {}}
            />
          </StoryPageWrapper>
        </div>
      </div>
      <div>
        <h3 className="text-cream-200 text-lg font-semibold mb-4 text-center">
          Step 4: Success
        </h3>
        <div className="min-h-screen bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 flex items-center justify-center p-4 pt-16">
          <StoryPageWrapper cardClassName="text-center py-12 w-full max-w-lg">
            <SuccessStep deviceName="Kitchen Espresso" />
          </StoryPageWrapper>
        </div>
      </div>
    </div>
  ),
  parameters: {
    layout: "padded",
  },
};

