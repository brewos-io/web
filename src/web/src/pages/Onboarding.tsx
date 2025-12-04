import { useState } from "react";
import { useNavigate } from "react-router-dom";
import { Card } from "@/components/Card";
import {
  WelcomeStep,
  ScanStep,
  ManualStep,
  MachineNameStep,
  SuccessStep,
} from "@/components/onboarding";
import { useAppStore } from "@/lib/mode";
import { parseClaimCode } from "@/lib/claim-parser";

export function Onboarding() {
  const navigate = useNavigate();
  const { claimDevice, fetchDevices } = useAppStore();

  const [step, setStep] = useState<"welcome" | "scan" | "manual" | "name" | "success">(
    "welcome"
  );
  const [claimCode, setClaimCode] = useState("");
  const [deviceName, setDeviceName] = useState("");
  const [validating, setValidating] = useState(false);
  const [claiming, setClaiming] = useState(false);
  const [error, setError] = useState("");
  const [previousStep, setPreviousStep] = useState<"scan" | "manual">("scan");
  const [parsedCode, setParsedCode] = useState<{
    deviceId?: string;
    token?: string;
    manualCode?: string;
  } | null>(null);

  // Validate the code first (without claiming)
  const handleValidate = async () => {
    if (!claimCode) return;

    setValidating(true);
    setError("");

    try {
      const parsed = parseClaimCode(claimCode);

      if (parsed.manualCode || (parsed.deviceId && parsed.token)) {
        // Code format is valid, move to name step
        setParsedCode(parsed);
        if (step === "scan" || step === "manual") {
          setPreviousStep(step);
        }
        setStep("name");
      } else {
        setError("Invalid code format");
      }
    } catch {
      setError("Invalid code format");
    }

    setValidating(false);
  };

  // Complete the claim with machine name
  const handleClaim = async () => {
    if (!parsedCode) return;

    setClaiming(true);
    setError("");

    try {
      // Handle manual code format
      if (parsedCode.manualCode) {
        const success = await claimDevice(
          parsedCode.manualCode,
          "",
          deviceName || undefined
        );
        if (success) {
          setStep("success");
          await fetchDevices();
          setTimeout(() => navigate("/"), 2000);
        } else {
          setError("Invalid or expired code");
          setStep(previousStep);
        }
        setClaiming(false);
        return;
      }

      // Handle deviceId + token format
      if (parsedCode.deviceId && parsedCode.token) {
        const success = await claimDevice(
          parsedCode.deviceId,
          parsedCode.token,
          deviceName || undefined
        );

        if (success) {
          setStep("success");
          await fetchDevices();
          setTimeout(() => navigate("/"), 2000);
        } else {
          setError("Failed to add device. The code may have expired.");
          setStep(previousStep);
        }
      } else {
        setError("Invalid code format");
        setStep(previousStep);
      }
    } catch {
      setError("An error occurred");
      setStep(previousStep);
    }

    setClaiming(false);
  };

  return (
    <div
      className={`full-page-scroll bg-gradient-to-br from-coffee-800 via-coffee-900 to-coffee-950 p-4 transition-all duration-300 ${
        step === "welcome"
          ? "flex flex-col items-center justify-center"
          : "flex justify-center"
      }`}
    >
      {step === "welcome" ? (
        <div className="w-full max-w-lg">
          <Card className="text-center animate-in fade-in slide-in-from-bottom-4 duration-500">
            <WelcomeStep
              onScanClick={() => setStep("scan")}
              onManualClick={() => setStep("manual")}
            />
          </Card>
        </div>
      ) : (
        <div className={`w-full max-w-lg transition-all duration-300 pt-16`}>
          {step === "scan" && (
            <Card className="animate-in fade-in slide-in-from-right-4 duration-300">
              <ScanStep
                onScan={(result) => {
                  setClaimCode(result);
                  setError(""); // Clear any previous errors
                }}
                onScanError={(err) => {
                  setError(err || "Failed to scan QR code");
                }}
                error={error}
                onBack={() => {
                  setStep("welcome");
                  setError("");
                  setClaimCode("");
                }}
                onValidate={handleValidate}
                disabled={!claimCode || validating}
                loading={validating}
              />
            </Card>
          )}

          {step === "manual" && (
            <Card className="animate-in fade-in slide-in-from-right-4 duration-300">
              <ManualStep
                claimCode={claimCode}
                onClaimCodeChange={setClaimCode}
                error={error}
                onBack={() => {
                  setStep("welcome");
                  setError("");
                }}
                onValidate={handleValidate}
                disabled={!claimCode || validating}
                loading={validating}
              />
            </Card>
          )}

          {step === "name" && (
            <Card className="animate-in fade-in slide-in-from-right-4 duration-300">
              <MachineNameStep
                deviceName={deviceName}
                onDeviceNameChange={setDeviceName}
                onBack={() => {
                  setStep(previousStep);
                  setError("");
                }}
                onContinue={handleClaim}
                loading={claiming}
              />
            </Card>
          )}

          {step === "success" && (
            <Card className="animate-in fade-in zoom-in-95 duration-500">
              <SuccessStep deviceName={deviceName} />
            </Card>
          )}
        </div>
      )}
    </div>
  );
}

