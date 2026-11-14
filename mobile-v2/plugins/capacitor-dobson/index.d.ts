export interface CatalogObject {
  name: string;
  commonName: string;
  type: string;
  ra: number;
  dec: number;
  mag: number;
}

export interface DobsonNativePlugin {
  setNightSystemBars(options: { enabled: boolean }): Promise<void>;
  connect(options: { ssid: string; password?: string }): Promise<{ ssid: string }>;
  disconnect(options: { ssid: string }): Promise<void>;
  getWifiState(): Promise<{ connected: boolean; ssid: string }>;
  getEncoderStatus(): Promise<{ rawAZ: number; rawALT: number; ticksAZ: number; ticksALT: number }>;
  startSensors(options: { intervalMs?: number }): Promise<void>;
  stopSensors(): Promise<void>;
  addListener(
    eventName: "orientation",
    listener: (event: { azimuth: number; altitude: number; cameraAzimuth: number; cameraAltitude: number; pitch: number; roll: number; timestamp: number; headingAvailable: boolean }) => void
  ): Promise<{ remove(): Promise<void> }>;
  addListener(
    eventName: "sensorError",
    listener: (event: { message: string }) => void
  ): Promise<{ remove(): Promise<void> }>;
  addListener(
    eventName: "wifiState",
    listener: (event: { connected: boolean; ssid: string }) => void
  ): Promise<{ remove(): Promise<void> }>;
  initializeCatalog(options: { revision: string; catalogJson: string }): Promise<void>;
  getCatalog(): Promise<{ objects: CatalogObject[] }>;
  searchCatalog(options: { query: string; limit?: number }): Promise<{ objects: CatalogObject[] }>;
  saveAstrometryApiKey(options: { apiKey: string }): Promise<void>;
  getAstrometryApiKey(): Promise<{ apiKey: string }>;
  clearAstrometryApiKey(): Promise<void>;
  saveAstroPhoto(options: { base64: string; filename: string }): Promise<void>;
}
