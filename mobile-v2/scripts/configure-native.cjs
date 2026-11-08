const fs = require("node:fs");
const path = require("node:path");

const root = path.resolve(__dirname, "..");

function configureAndroid() {
  const manifestPath = path.join(root, "android", "app", "src", "main", "AndroidManifest.xml");
  if (!fs.existsSync(manifestPath)) return;
  const manifest = fs.readFileSync(manifestPath, "utf8").replace(
    /<application\b([^>]*)>/,
    (tag, attributes) => {
      const updatedAttributes = /\bandroid:usesCleartextTraffic\s*=/.test(attributes)
        ? attributes.replace(/android:usesCleartextTraffic\s*=\s*["'][^"']*["']/, 'android:usesCleartextTraffic="true"')
        : `${attributes} android:usesCleartextTraffic="true"`;
      return `<application${updatedAttributes}>`;
    }
  );
  fs.writeFileSync(manifestPath, manifest);
}

function configureIos() {
  const appDirectory = path.join(root, "ios", "App", "App");
  if (!fs.existsSync(appDirectory)) return;
  const plistPath = path.join(appDirectory, "Info.plist");
  if (!fs.existsSync(plistPath)) return;

  let plist = fs.readFileSync(plistPath, "utf8");
  const requiredKeys = [
    ["NSLocalNetworkUsageDescription", "Connexion au réseau local du télescope Dobson Push-To."],
    ["NSCameraUsageDescription", "La caméra permet d'afficher le ciel en réalité augmentée."],
    ["NSPhotoLibraryUsageDescription", "L'accès aux photos permet d'enregistrer vos captures dans l'album AstroPhoto."],
    ["NSPhotoLibraryAddUsageDescription", "L'application enregistre vos captures dans l'album AstroPhoto."],
    ["NSLocationWhenInUseUsageDescription", "La position permet de calculer le ciel visible depuis votre emplacement."],
    ["NSAppTransportSecurity", null]
  ];
  for (const [key, value] of requiredKeys) {
    if (plist.includes(`<key>${key}</key>`)) continue;
    const entry = value === null
      ? `<key>${key}</key>\n\t<dict>\n\t\t<key>NSAllowsLocalNetworking</key>\n\t\t<true/>\n\t</dict>`
      : `<key>${key}</key>\n\t<string>${value}</string>`;
    plist = plist.replace(/<\/dict>\s*<\/plist>/, `${entry}\n</dict>\n</plist>`);
  }
  fs.writeFileSync(plistPath, plist);
}

configureAndroid();
configureIos();
