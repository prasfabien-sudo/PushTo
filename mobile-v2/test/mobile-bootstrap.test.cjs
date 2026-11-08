const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");
const test = require("node:test");

function createMobileWindow(pathname = "/beta_sky.html", session = new Map()) {
  const storage = new Map();
  const requests = [];
  const redirects = [];
  const window = {
    __dobsonMobileApp: true,
    location: {
      href: `https://localhost${pathname}`,
      origin: "https://localhost",
      pathname,
      replace: (url) => redirects.push(url)
    },
    fetch: async (url) => {
      requests.push(String(url));
      return new Response("remote");
    },
    __dobsonPhoneOrientation: { azimuth: 120, altitude: 60, pitch: 30 }
  };
  const localStorage = {
    getItem: (key) => storage.get(key) ?? null,
    setItem: (key, value) => storage.set(key, String(value))
  };
  const sessionStorage = {
    getItem: (key) => session.get(key) ?? null,
    setItem: (key, value) => session.set(key, String(value))
  };
  const context = vm.createContext({ window, localStorage, sessionStorage, URL, Request, Response, Promise, console });
  const source = fs.readFileSync(path.resolve(__dirname, "../public/mobile-bootstrap.js"), "utf8");
  vm.runInContext(source, context);
  return { window, localStorage, sessionStorage, requests, redirects };
}

test("l’application mobile ouvre la vue 3D comme écran de démarrage", () => {
  const { redirects, window, sessionStorage } = createMobileWindow("/");

  assert.deepEqual(redirects, ["/beta_sky.html"]);
  const firstStartup = createMobileWindow("/index.html");
  assert.deepEqual(firstStartup.redirects, ["/beta_sky.html"]);
  const returningToEncoder = createMobileWindow("/index.html", new Map([
    ["dobson-mobile-startup-routed", "true"]
  ]));
  assert.deepEqual(returningToEncoder.redirects, []);
  assert.equal(sessionStorage.getItem("dobson-mobile-startup-routed"), "true");
  assert.equal(window.location.pathname, "/");
});

test("la vue du ciel mobile charge ses préférences et capteurs sans appeler l’ESP32", async () => {
  const { window, localStorage, requests } = createMobileWindow();

  const status = await (await window.fetch("/status")).json();
  assert.equal(status.rawAZ, 120);
  assert.equal(status.rawALT, 60);
  assert.equal(status.lat, 45.89);
  assert.equal(status.lon, 6.05);

  await window.fetch("/save_sky_prefs?mag=6&showGrid=1");
  assert.equal(localStorage.getItem("dobson-mobile-sky-prefs"), '{"mag":6,"showGrid":1}');
  assert.equal(await (await window.fetch("/get_sky_prefs")).json().then((prefs) => prefs.mag), 6);
  assert.deepEqual(requests, []);
});

test("la vue du ciel mobile démarre à l’horizon quand aucun capteur ne fournit de mesure", async () => {
  const { window } = createMobileWindow();
  window.__dobsonPhoneOrientation = undefined;

  const status = await (await window.fetch("/status")).json();

  assert.equal(status.rawALT, 0);
  assert.equal(status.rawAZ, 0);
});

test("la vue du ciel mobile récupère les encodeurs ESP32 après connexion", async () => {
  const { window, requests } = createMobileWindow();
  window.__dobsonEspConnected = true;

  await window.fetch("/status");
  assert.deepEqual(requests, ["http://192.168.4.1/status"]);
});

test("la vue du ciel mobile conserve l’altitude signée des capteurs sous l’horizon", async () => {
  const { window } = createMobileWindow();
  window.__dobsonPhoneOrientation = { azimuth: 210, altitude: -25, pitch: 30 };

  const status = await (await window.fetch("/status")).json();

  assert.equal(status.rawAZ, 210);
  assert.equal(status.rawALT, 335);
});

test("les capteurs natifs utilisent l’axe supérieur du téléphone pour le pointage", () => {
  const android = fs.readFileSync(path.resolve(__dirname, "../plugins/capacitor-dobson/android/src/main/java/fr/dobsonpushto/nativeplugin/DobsonNativePlugin.kt"), "utf8");
  const ios = fs.readFileSync(path.resolve(__dirname, "../plugins/capacitor-dobson/ios/Sources/DobsonNativePlugin/DobsonNativePlugin.swift"), "utf8");

  assert.match(android, /val east = rotation\[1\]\.toDouble\(\)/);
  assert.match(android, /val north = rotation\[4\]\.toDouble\(\)/);
  assert.match(android, /val up = rotation\[7\]\.toDouble\(\)/);
  assert.match(android, /val cameraEast = -rotation\[2\]\.toDouble\(\)/);
  assert.match(android, /val cameraNorth = -rotation\[5\]\.toDouble\(\)/);
  assert.match(android, /val cameraUp = -rotation\[8\]\.toDouble\(\)/);
  assert.match(android, /\.put\("cameraAzimuth", cameraAzimuth\)/);
  assert.match(android, /\.put\("cameraAltitude", cameraAltitude\)/);
  assert.match(ios, /let north = rotation\.m12/);
  assert.match(ios, /let east = -rotation\.m22/);
  assert.match(ios, /let up = rotation\.m32/);
  assert.match(ios, /let cameraNorth = -rotation\.m13/);
  assert.match(ios, /let cameraEast = rotation\.m23/);
  assert.match(ios, /let cameraUp = -rotation\.m33/);
});

test("l’écran d’accueil mobile utilise des données locales hors connexion ESP32", async () => {
  const { window, requests } = createMobileWindow("/index.html");

  const status = await (await window.fetch("/status")).json();

  assert.equal(status.rawAZ, 120);
  assert.equal(status.rawALT, 60);
  assert.deepEqual(requests, []);
});

test("les paramètres du ciel restent disponibles et enregistrés sur la page de configuration hors ligne", async () => {
  const { window, localStorage, requests } = createMobileWindow("/config.html");

  await window.fetch("/save_sky_prefs?showGrid=0&projMode=st%C3%A9r%C3%A9o");

  assert.equal(localStorage.getItem("dobson-mobile-sky-prefs"), '{"showGrid":0,"projMode":"stéréo"}');
  assert.deepEqual(await (await window.fetch("/get_sky_prefs")).json(), {
    showGrid: 0,
    projMode: "stéréo"
  });
  assert.deepEqual(requests, []);
});

test("la configuration et les lieux sont enregistrés localement hors connexion ESP32", async () => {
  const { window, localStorage, requests } = createMobileWindow("/config.html");
  const parameters = new URLSearchParams({
    action: "set_config",
    ticksAZ: "4096",
    ticksALT: "2048",
    revAZ: "1",
    revALT: "0",
    lat: "48.5",
    lon: "-2.25",
    siteName: "Jardin"
  });

  const configResponse = await window.fetch(`/cmd?${parameters}`);
  assert.deepEqual(await configResponse.json(), { status: "OK", local: true });
  assert.deepEqual(JSON.parse(localStorage.getItem("dobson-mobile-device-config")), {
    ticksAZ: 4096,
    ticksALT: 2048,
    revAZ: true,
    revALT: false,
    lat: 48.5,
    lon: -2.25,
    siteName: "Jardin"
  });

  const sites = { "48.5,-2.25": { name: "Jardin", lat: 48.5, lon: -2.25 } };
  await window.fetch(`/cmd?action=save_sites&json=${encodeURIComponent(JSON.stringify(sites))}`);
  assert.deepEqual(await (await window.fetch("/get_sites")).json(), sites);
  assert.deepEqual(requests, []);

  const status = await (await window.fetch("/status")).json();
  assert.equal(status.ticksAZ, 4096);
  assert.equal(status.ticksALT, 2048);
  assert.equal(status.revAZ, true);
  assert.equal(status.lat, 48.5);
  assert.equal(status.lon, -2.25);
});

test("la configuration locale refuse les coordonnées et résolutions invalides", async () => {
  const { window, localStorage, requests } = createMobileWindow("/config.html");
  const response = await window.fetch("/cmd?action=set_config&ticksAZ=0&ticksALT=10&lat=91&lon=0");

  assert.equal(response.status, 400);
  assert.equal((await response.json()).status, "ERROR");
  assert.equal(localStorage.getItem("dobson-mobile-device-config"), null);
  assert.deepEqual(requests, []);
});
