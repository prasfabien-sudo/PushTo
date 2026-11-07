export interface CatalogObject {
  name: string;
  commonName: string;
  type: string;
  ra: number;
  dec: number;
  mag: number;
}

export interface DobsonNativePlugin {
  connect(options: { ssid: string; password?: string }): Promise<{ ssid: string; baseUrl: string }>;
  disconnect(options: { ssid: string }): Promise<void>;
  startSensors(options: { intervalMs?: number }): Promise<void>;
  stopSensors(): Promise<void>;
  addListener(
    eventName: "orientation",
    listener: (event: { azimuth: number; altitude: number; pitch: number; roll: number; timestamp: number; headingAvailable: boolean }) => void
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
}
