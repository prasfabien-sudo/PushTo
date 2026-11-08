export function angularSeparationDegrees(previous, current) {
  if (!previous || !current) return 0;
  const values = [previous.azimuth, previous.altitude, current.azimuth, current.altitude].map(Number);
  if (!values.every(Number.isFinite)) return 0;

  const [previousAzimuth, previousAltitude, currentAzimuth, currentAltitude] = values.map((value) => value * Math.PI / 180);
  const cosine = Math.sin(previousAltitude) * Math.sin(currentAltitude)
    + Math.cos(previousAltitude) * Math.cos(currentAltitude) * Math.cos(currentAzimuth - previousAzimuth);
  return Math.acos(Math.max(-1, Math.min(1, cosine))) * 180 / Math.PI;
}

export function hasSignificantSkyMotion(previous, current, thresholdDegrees = 6) {
  return angularSeparationDegrees(previous, current) >= thresholdDegrees;
}
