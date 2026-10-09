const fs = require("node:fs");
const http = require("node:http");
const path = require("node:path");

const root = __dirname;
const include = path.join(root, "include");
const host = "127.0.0.1";
const port = 8000;

const pages = {
  "/": ["WebPage.h", "HTTP_PAGE"],
  "/station": ["StationPage.h", "HTTP_STATION_PAGE"],
  "/calib_page": ["CalibPage.h", "HTTP_CALIB_PAGE"],
  "/config": ["ConfigPage.h", "HTTP_CONFIG_PAGE"],
  "/releasenotes": ["ReleaseNotes.h", "HTTP_RELEASE_NOTES_PAGE"],
  "/simu": ["SimuPage.h", "HTTP_SIMU_PAGE"],
  "/test": ["TestPage.h", "HTTP_TEST_PAGE"],
  "/help": ["HelpPage.h", "HTTP_HELP_PAGE"],
  "/beta_sky": ["BetaSkyPage.h", "HTTP_BETA_SKY_PAGE"],
};

const defaultSites = {
  chavanod: { name: "Chavanod", lat: 45.89, lon: 6.05 },
  annecy: { name: "Annecy", lat: 45.9, lon: 6.12 },
  lyon: { name: "Lyon", lat: 45.76, lon: 4.83 },
  paris: { name: "Paris", lat: 48.85, lon: 2.35 },
};

const state = {
  rawAZ: 0,
  rawALT: 0,
  ticksAZ: 10000,
  ticksALT: 10000,
  dirAZ: 1,
  dirALT: 1,
  lat: 45.89,
  lon: 6.05,
  siteName: "Chavanod",
  sites: { ...defaultSites },
  star1: null,
};

function readEmbeddedPage(headerName, symbol) {
  const source = fs.readFileSync(path.join(include, headerName), "utf8").replace(/^\uFEFF/, "");
  const match = source.match(
    new RegExp(`\\b${symbol}\\[\\]\\s+PROGMEM\\s*=\\s*R"rawliteral\\(([\\s\\S]*?)\\)rawliteral";`),
  );
  if (!match) {
    throw new Error(`Impossible d'extraire la page ${symbol} depuis ${headerName}`);
  }
  return match[1];
}

function readCatalog() {
  const source = fs.readFileSync(path.join(include, "ObjectsDB.h"), "utf8").replace(/^\uFEFF/, "");
  const entryPattern =
    /\{\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\}/g;
  return Array.from(source.matchAll(entryPattern), (match) => ({
    name: match[1],
    commonName: match[2],
    type: match[3],
    ra: Number(match[4]),
    dec: Number(match[5]),
    mag: Number(match[6]),
  }));
}

const catalog = readCatalog();

function send(response, status, contentType, body) {
  response.writeHead(status, {
    "Content-Type": contentType,
    "Content-Length": Buffer.byteLength(body),
    "Cache-Control": "no-store",
  });
  response.end(body);
}

function sendJson(response, value) {
  send(response, 200, "application/json; charset=utf-8", JSON.stringify(value));
}

function sendText(response, status, value) {
  send(response, status, "text/plain; charset=utf-8", value);
}

function status() {
  return {
    rawAZ: state.rawAZ,
    rawALT: state.rawALT,
    ticksAZ: state.ticksAZ,
    ticksALT: state.ticksALT,
    dirAZ: state.dirAZ,
    dirALT: state.dirALT,
    revAZ: state.dirAZ === -1,
    revALT: state.dirALT === -1,
    profile: 0,
    lat: state.lat,
    lon: state.lon,
    siteName: state.siteName,
  };
}

function numericParameter(url, name) {
  const value = url.searchParams.get(name);
  if (value === null || value.trim() === "") return null;
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}

function updateLocation(url) {
  const lat = numericParameter(url, "lat");
  const lon = numericParameter(url, "lon");
  if (lat === null || lon === null) return false;
  state.lat = lat;
  state.lon = lon;
  state.siteName = url.searchParams.get("siteName") || "Lieu personnalisé";
  return true;
}

function setPosition(az, alt) {
  state.rawAZ = Math.round((az / 360) * state.ticksAZ * state.dirAZ);
  state.rawALT = Math.round((alt / 360) * state.ticksALT * state.dirALT);
}

function handleCommand(url, response) {
  const action = url.searchParams.get("action");
  if (!action) {
    sendText(response, 400, "Bad Request: Missing action parameter");
    return;
  }

  if (action === "set_config") {
    const ticksAZ = numericParameter(url, "ticksAZ");
    const ticksALT = numericParameter(url, "ticksALT");
    const revAZ = numericParameter(url, "revAZ");
    const revALT = numericParameter(url, "revALT");
    if ([ticksAZ, ticksALT, revAZ, revALT].includes(null) || !updateLocation(url)) {
      sendText(response, 400, "Bad Request: Missing or invalid config parameters");
      return;
    }
    state.ticksAZ = ticksAZ;
    state.ticksALT = ticksALT;
    state.dirAZ = revAZ ? -1 : 1;
    state.dirALT = revALT ? -1 : 1;
    sendJson(response, { status: "OK" });
  } else if (action === "set_location") {
    const updated = updateLocation(url);
    sendText(response, updated ? 200 : 400, updated ? "OK" : "Bad Request: Missing lat or lon");
  } else if (action === "set_zero_alt") {
    state.rawALT = 0;
    sendText(response, 200, "OK");
  } else if (action === "set_zero_az") {
    state.rawAZ = 0;
    sendText(response, 200, "OK");
  } else if (action === "set_polaris") {
    state.rawAZ = 0;
    state.rawALT = Math.round((state.lat / 360) * state.ticksALT * state.dirALT);
    sendText(response, 200, "OK");
  } else if (action === "align_star" || action === "align_star1" || action === "align_star2") {
    const az = numericParameter(url, "az");
    const alt = numericParameter(url, "alt");
    if (az === null || alt === null) {
      sendText(response, 400, "Bad Request: Missing or invalid az or alt");
    } else if (action === "align_star") {
      setPosition(az, alt);
      sendText(response, 200, "OK");
    } else if (action === "align_star1") {
      state.star1 = [az, alt];
      sendJson(response, { status: "OK", step: 1 });
    } else if (!state.star1) {
      sendText(response, 400, "Veuillez valider l'etoile 1 d'abord");
    } else {
      setPosition((state.star1[0] + az) / 2, (state.star1[1] + alt) / 2);
      state.star1 = null;
      sendJson(response, { status: "OK", step: 2 });
    }
  } else if (action === "align") {
    state.rawAZ = 0;
    state.rawALT = 0;
    sendText(response, 200, "OK");
  } else if (action === "save_sites") {
    try {
      state.sites = JSON.parse(url.searchParams.get("json") || "");
      sendJson(response, { status: "OK" });
    } catch (error) {
      sendText(response, 400, `JSON invalide: ${error.message}`);
    }
  } else if (action === "select_target") {
    sendText(response, 200, "OK");
  } else {
    sendText(response, 400, "Unknown action");
  }
}

function handleCalibration(url, response) {
  const command = url.searchParams.get("cmd");
  if (command === "start") {
    state.rawAZ = 0;
    state.rawALT = 0;
    sendText(response, 200, "OK");
  } else if (command === "fin_az") {
    if (state.rawAZ !== 0) state.ticksAZ = Math.abs(state.rawAZ);
    sendText(response, 200, "OK");
  } else if (command === "fin_alt_90") {
    if (state.rawALT !== 0) state.ticksALT = Math.abs(state.rawALT) * 4;
    sendText(response, 200, "OK");
  } else {
    sendText(response, 400, "Bad Request: Invalid calibration command");
  }
}

function handleSimulationStep(url, response) {
  const axis = url.searchParams.get("axis");
  const delta = numericParameter(url, "delta");
  if (!axis || delta === null) {
    sendText(response, 400, "Bad Request: Missing or invalid axis or delta");
  } else if (axis === "az") {
    state.rawAZ += Math.trunc(delta) * state.dirAZ;
    sendText(response, 200, "OK");
  } else if (axis === "alt") {
    state.rawALT += Math.trunc(delta) * state.dirALT;
    sendText(response, 200, "OK");
  } else if (axis === "reset") {
    state.rawAZ = 0;
    state.rawALT = 0;
    sendText(response, 200, "OK");
  } else {
    sendText(response, 400, "Bad Request: Invalid axis");
  }
}

const server = http.createServer((request, response) => {
  const url = new URL(request.url, `http://${host}:${port}`);
  if (request.method !== "GET") {
    sendText(response, 405, "Méthode non autorisée");
  } else if (pages[url.pathname]) {
    const [header, symbol] = pages[url.pathname];
    send(response, 200, "text/html; charset=utf-8", readEmbeddedPage(header, symbol));
  } else if (url.pathname === "/icons.svg") {
    send(response, 200, "image/svg+xml; charset=utf-8", readEmbeddedPage("Icons.h", "HTTP_ICONS"));
  } else if (url.pathname === "/status") {
    sendJson(response, status());
  } else if (url.pathname === "/catalog") {
    sendJson(response, catalog);
  } else if (url.pathname === "/get_sites") {
    sendJson(response, state.sites);
  } else if (url.pathname === "/cmd") {
    handleCommand(url, response);
  } else if (url.pathname === "/calib") {
    handleCalibration(url, response);
  } else if (url.pathname === "/simstep") {
    handleSimulationStep(url, response);
  } else {
    sendText(response, 404, "Route inconnue");
  }
});

server.listen(port, host, () => {
  console.log(`Simulation Dobson Push-To accessible sur http://${host}:${port}`);
  console.log("Arrêt avec Ctrl+C. Les données simulées sont conservées en mémoire.");
});

server.on("error", (error) => {
  console.error(`Impossible de démarrer le serveur local : ${error.message}`);
  process.exitCode = 1;
});

process.on("SIGINT", () => {
  server.close(() => {
    console.log("\nArrêt du serveur de simulation.");
    process.exit(0);
  });
});
