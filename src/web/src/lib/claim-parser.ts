/**
 * Parse claim code from various formats
 * Supports:
 * - URL format: ?id=DEVICE_ID&token=TOKEN
 * - Legacy format: DEVICE_ID:TOKEN
 * - Short code format: X6ST-AP3G
 */
export interface ParsedClaimCode {
  deviceId?: string;
  token?: string;
  manualCode?: string;
}

export function parseClaimCode(code: string): ParsedClaimCode {
  if (code.includes("?")) {
    // URL format
    try {
      const url = new URL(code);
      return {
        deviceId: url.searchParams.get("id") || "",
        token: url.searchParams.get("token") || "",
      };
    } catch {
      // Try parsing as query string
      const params = new URLSearchParams(code.split("?")[1]);
      return {
        deviceId: params.get("id") || "",
        token: params.get("token") || "",
      };
    }
  } else if (code.includes(":")) {
    // Legacy format: DEVICE_ID:TOKEN
    const parts = code.split(":");
    return {
      deviceId: parts[0],
      token: parts[1] || "",
    };
  } else if (/^[A-Z0-9]{4}-[A-Z0-9]{4}$/i.test(code.trim())) {
    // Short manual code format: X6ST-AP3G
    return {
      manualCode: code.trim().toUpperCase(),
    };
  }

  return {};
}

