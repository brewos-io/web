// Connection types
export type ConnectionState = 
  | 'disconnected' 
  | 'connecting' 
  | 'connected' 
  | 'reconnecting' 
  | 'error';

export type ConnectionMode = 'local' | 'cloud';

export interface ConnectionConfig {
  mode: ConnectionMode;
  endpoint?: string;
  cloudUrl?: string;
  authToken?: string;
  deviceId?: string;
}

// Message types from ESP32
export type MessageType = 
  | 'status'
  | 'esp_status'
  | 'pico_status'
  | 'scale_status'
  | 'stats'
  | 'event'
  | 'log'
  | 'error'
  | 'scan_result'
  | 'scan_complete'
  | 'device_info';

export interface WebSocketMessage {
  type: MessageType;
  [key: string]: unknown;
}

// Machine state
export type MachineState = 
  | 'unknown'
  | 'init'
  | 'idle'
  | 'heating'
  | 'ready'
  | 'brewing'
  | 'steaming'
  | 'cooldown'
  | 'fault';

export type MachineMode = 'standby' | 'on' | 'eco';

export interface MachineStatus {
  state: MachineState;
  mode: MachineMode;
  isHeating: boolean;
  isBrewing: boolean;
}

// Temperature
export interface BoilerTemp {
  current: number;
  setpoint: number;
  max: number;
}

export interface Temperatures {
  brew: BoilerTemp;
  steam: BoilerTemp;
}

// Power
export interface PowerStatus {
  current: number;
  voltage: number;
  todayKwh: number;
  totalKwh: number;
}

// Water
export type WaterLevel = 'ok' | 'low' | 'empty';

export interface WaterStatus {
  tankLevel: WaterLevel;
  dripTrayFull: boolean;
}

// Scale
export interface ScaleStatus {
  connected: boolean;
  name: string;
  type: string;
  weight: number;
  flowRate: number;
  stable: boolean;
  battery: number;
}

export interface ScaleScanResult {
  address: string;
  name: string;
  rssi: number;
  type?: string;
}

// Brew-by-weight
export interface BBWSettings {
  enabled: boolean;
  targetWeight: number;
  doseWeight: number;
  stopOffset: number;
  autoTare: boolean;
}

// Shot
export interface ShotStatus {
  active: boolean;
  startTime: number;
  duration: number;
  weight: number;
  flowRate: number;
}

// WiFi
export interface WiFiStatus {
  connected: boolean;
  ssid: string;
  ip: string;
  rssi: number;
  apMode: boolean;
}

// MQTT
export interface MQTTConfig {
  enabled: boolean;
  broker: string;
  port: number;
  username: string;
  password: string;
  topic: string;
  discovery: boolean;
}

export interface MQTTStatus {
  enabled: boolean;
  connected: boolean;
  broker: string;
  topic: string;
}

// Cloud
export interface CloudConfig {
  enabled: boolean;
  serverUrl: string;
  connected: boolean;
  deviceId: string;
}

// Device info
export interface DeviceInfo {
  deviceId: string;
  deviceName: string;
  machineBrand: string;
  machineModel: string;
  machineType: string;  // Computed: dual_boiler, single_boiler, etc.
  firmwareVersion: string;
}

export interface ESP32Info {
  version: string;
  freeHeap: number;
  uptime: number;
}

export interface PicoInfo {
  connected: boolean;
  version: string;
  uptime: number;
}

// Statistics (matches ESP32 state_types.h)
export interface Statistics {
  // Lifetime stats
  totalShots: number;
  totalSteamCycles: number;
  totalKwh: number;
  totalOnTimeMinutes: number;
  
  // Daily stats (reset at midnight)
  shotsToday: number;
  kwhToday: number;
  onTimeToday: number;
  
  // Maintenance counters
  shotsSinceDescale: number;
  shotsSinceGroupClean: number;
  shotsSinceBackflush: number;
  lastDescaleTimestamp: number;
  lastGroupCleanTimestamp: number;
  lastBackflushTimestamp: number;
  
  // Pico statistics (from protocol)
  avgBrewTimeMs: number;
  minBrewTimeMs: number;
  maxBrewTimeMs: number;
  dailyCount: number;
  weeklyCount: number;
  monthlyCount: number;
  
  // Session
  sessionStartTimestamp: number;
  sessionShots: number;
}

// Alerts & Logs
export interface Alert {
  id: number;
  time: string;
  level: 'info' | 'warning' | 'error';
  message: string;
  dismissed: boolean;
}

export interface LogEntry {
  id: number;
  time: string;
  level: string;
  message: string;
}

// Cloud device (from API)
export interface CloudDevice {
  id: string;
  name: string;
  isOnline: boolean;
  lastSeen: string | null;
  firmwareVersion: string | null;
  machineType: string | null;
  claimedAt: string;
}

// User preferences (stored in localStorage)
export type FirstDayOfWeek = 'sunday' | 'monday';

export interface UserPreferences {
  firstDayOfWeek: FirstDayOfWeek;
  use24HourTime: boolean;
}
