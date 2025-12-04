import { Button } from "@/components/Button";
import { Input } from "@/components/Input";
import { ArrowRight, Coffee } from "lucide-react";

interface MachineNameStepProps {
  deviceName?: string;
  onDeviceNameChange?: (name: string) => void;
  onBack?: () => void;
  onContinue?: () => void;
  loading?: boolean;
}

export function MachineNameStep({
  deviceName,
  onDeviceNameChange,
  onBack,
  onContinue,
  loading = false,
}: MachineNameStepProps) {
  return (
    <div className="animate-in fade-in slide-in-from-bottom-4 duration-500">
      {/* Header */}
      <div className="text-center mb-8">
        <div className="w-16 h-16 bg-accent/10 rounded-2xl flex items-center justify-center mx-auto mb-4">
          <Coffee className="w-8 h-8 text-accent" />
        </div>
        <h2 className="text-3xl font-bold text-theme mb-2">Name Your Machine</h2>
        <p className="text-theme-muted text-base">
          Give your machine a friendly name to easily identify it
        </p>
      </div>

      {/* Form */}
      <div className="space-y-6">
        <Input
          label="Machine Name"
          placeholder="e.g., Kitchen Espresso"
          value={deviceName}
          onChange={(e) => onDeviceNameChange?.(e.target.value)}
          hint="This name will appear in your machine list"
          autoFocus
        />

        {/* Action buttons */}
        <div className="flex gap-3 pt-2">
          <Button variant="secondary" className="flex-1" onClick={onBack}>
            Back
          </Button>
          <Button
            className="flex-1 font-semibold"
            onClick={onContinue}
            loading={loading}
          >
            {loading ? "Adding..." : "Add Machine"}
            <ArrowRight className="w-4 h-4" />
          </Button>
        </div>
      </div>
    </div>
  );
}

