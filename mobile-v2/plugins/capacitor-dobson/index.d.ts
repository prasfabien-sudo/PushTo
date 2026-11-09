export interface CatalogObject {
  name: string;
  commonName: string;
  type: string;
  ra: number;
  dec: number;
  mag: number;
}

export interface DobsonNativePlugin {
  connect(options: { ssid: string; password?: string }): Promise<{ ssid: string }>;
  disconnect(options: { ssid: string }): Promise<void>;
  getEncoderStatus(): Promise<{ rawAZ: number; rawALT: number; ticksAZ: number; ticksALT: number }>;
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
  saveAstroPhoto(options: { base64: string; filename: string }): Promise<void>;
  zwoListCameras(): Promise<{ cameras: ZwoCameraInfo[] }>;
  zwoStatus(): Promise<ZwoStatus>;
  zwoOpen(options: { index: number; exposureMs?: number; gain?: number }): Promise<ZwoStatus>;
  zwoSetControls(options: { exposureMs?: number; gain?: number }): Promise<ZwoStatus>;
  zwoGetFrame(): Promise<{ dataUrl: string; width: number; height: number; timestamp: number }>;
  zwoClose(): Promise<ZwoStatus>;
  zwoGetLog(): Promise<{ content: string }>;
  zwoClearLog(): Promise<void>;
}

export interface ZwoCameraInfo {
  index: number;
  cameraId: number;
  name: string;
  maxWidth: number;
  maxHeight: number;
  isColor: boolean;
}

export interface ZwoStatus {
  connected: boolean;
  usbDetected: boolean;
  name: string;
  width: number;
  height: number;
  isColor: boolean;
  exposureMs: number;
  gain: number;
}
