import { Capacitor } from "@capacitor/core";
import { DobsonNative } from "@dobson/capacitor-dobson";
import { loadMobileCatalog } from "./mobile-catalog.js";
import { solveWithAstrometry } from "./astrometry.js";
import { findAstronomyImage } from "./astronomy-images.js";
import { hasSignificantSkyMotion } from "./sky-motion.js";
import "./mobile.css";

window.__resolveDobsonNativePlugin?.(DobsonNative);
window.__dobsonNativeCatalogPromise = Capacitor.isNativePlatform()
  ? loadMobileCatalog(window.location.pathname, window.fetch.bind(window))
  : Promise.resolve(null);
window.__dobsonNativeCatalogPromise.then(window.__resolveDobsonCatalog, window.__rejectDobsonCatalog);

if (Capacitor.isNativePlatform()) {
  document.addEventListener("DOMContentLoaded", () => {
    installNativePanel();
    installNightSystemBars();
    installSimulationControls();
    installAstronomyImages();
    installAugmentedReality();
  }, { once: true });
}

function installSimulationControls() {
  const isSimulationPage = window.location.pathname.endsWith("/simu.html");
  if (isSimulationPage) {
    const card = document.querySelector(".card");
    const modeButton = document.createElement("button");
    modeButton.type = "button";
    modeButton.className = "dobson-simulation-toggle";
    const description = document.createElement("p");
    description.textContent = "La simulation utilise des compteurs locaux, indépendants du télescope. Les réglages et alignements modifiés pendant la simulation sont temporaires.";
    card.insertBefore(description, card.querySelector(".step-picker"));
    card.insertBefore(modeButton, card.querySelector(".step-picker"));
    const refresh = () => {
      const active = window.__dobsonSimulationActive();
      modeButton.textContent = active ? "Désactiver la simulation" : "Activer la simulation";
      modeButton.setAttribute("aria-pressed", String(active));
      for (const control of card.querySelectorAll('[onclick^="stepSelected"], [onclick="resetSimulation()"]')) {
        control.disabled = !active;
      }
    };
    modeButton.addEventListener("click", () => {
      try {
        if (window.__dobsonSimulationActive()) window.__dobsonStopSimulation();
        else window.__dobsonStartSimulation();
        window.location.reload();
      } catch (error) {
        document.querySelector("#simStatus").textContent = `Impossible de changer le mode simulation : ${error.message}`;
        console.error("Changement du mode simulation impossible.", error);
        refresh();
      }
    });
    refresh();
  }
  if (!window.__dobsonSimulationActive()) return;
  const banner = document.createElement("div");
  banner.className = "dobson-simulation-banner";
  banner.setAttribute("role", "status");
  const text = document.createElement("span");
  text.textContent = "Simulation active — encodeurs réels non affichés";
  const exit = document.createElement("button");
  exit.type = "button";
  exit.textContent = "Quitter";
  exit.setAttribute("aria-label", "Quitter la simulation et revenir aux données réelles");
  exit.addEventListener("click", () => {
    window.__dobsonStopSimulation();
    window.location.reload();
  });
  banner.append(text, exit);
  document.body.appendChild(banner);
}

function installNightSystemBars() {
  if (Capacitor.getPlatform() !== "android") return;
  const status = document.createElement("output");
  status.className = "dobson-connection-notice";
  status.setAttribute("role", "status");
  status.setAttribute("aria-live", "polite");
  document.body.appendChild(status);
  let queue = Promise.resolve();
  const update = () => {
    const enabled = document.documentElement.dataset.nightMode === "true";
    queue = queue.then(() => DobsonNative.setNightSystemBars({ enabled })).then(() => {
      status.textContent = "";
    }).catch((error) => {
      status.textContent = "Impossible de modifier la barre d’état pour le mode nuit.";
      console.error("Modification de la barre d’état impossible.", error);
    });
  };
  new MutationObserver(update).observe(document.documentElement, {
    attributes: true,
    attributeFilter: ["data-night-mode"]
  });
  update();
}

function installNativePanel() {
  document.body.classList.add("dobson-native-app");
  installNativeNavigation();
  const panel = document.createElement("details");
  panel.className = "dobson-native-panel";
  panel.innerHTML = `
    <summary aria-label="Connexion au télescope ESP32">Connexion ESP32</summary>
    <div class="dobson-native-content">
      <label for="dobson-ssid">Réseau Wi-Fi de l’ESP32</label>
      <input id="dobson-ssid" type="text" value="Dobson_PushTo" autocomplete="off">
      <label for="dobson-wifi-password">Mot de passe Wi-Fi (si nécessaire)</label>
      <input id="dobson-wifi-password" type="password" autocomplete="new-password">
      <div class="dobson-native-actions">
        <button id="dobson-connect" type="button">Connecter</button>
        <button id="dobson-disconnect" type="button">Déconnecter</button>
      </div>
      <button id="dobson-test-connection" type="button">Tester la lecture des encodeurs</button>
      <output id="dobson-native-status" role="status" aria-live="polite"></output>
      <output id="dobson-encoder-test-status" role="status" aria-live="polite"></output>
    </div>`;
  const isSkyPage = window.location.pathname.endsWith("/beta_sky.html");
  const isConfigPage = window.location.pathname.endsWith("/config.html");

  const ssid = panel.querySelector("#dobson-ssid");
  const password = panel.querySelector("#dobson-wifi-password");
  const status = panel.querySelector("#dobson-native-status");
  const testButton = panel.querySelector("#dobson-test-connection");
  const testStatus = panel.querySelector("#dobson-encoder-test-status");
  let previousEncoderTest;
  const connectionNotice = document.createElement("output");
  connectionNotice.className = "dobson-connection-notice";
  connectionNotice.setAttribute("role", "status");
  connectionNotice.setAttribute("aria-live", "polite");
  document.body.appendChild(connectionNotice);
  let noticeTimer;

  const publishWifiState = (state) => {
    const wasConnected = window.__dobsonEspConnected;
    window.__dobsonEspConnected = state.connected;
    if (!state.connected) {
      previousEncoderTest = undefined;
      testStatus.textContent = "";
    }
    window.dispatchEvent(new CustomEvent("dobson:wifiState", { detail: state }));
    if (isConfigPage && state.connected && state.ssid) ssid.value = state.ssid;
    if (state.connected && !wasConnected) {
      status.textContent = "";
      connectionNotice.textContent = `Connecté à ${state.ssid}.`;
      clearTimeout(noticeTimer);
      noticeTimer = setTimeout(() => { connectionNotice.textContent = ""; }, 3500);
    }
  };
  let wifiRevision = 0;
  DobsonNative.addListener("wifiState", (state) => {
    wifiRevision++;
    publishWifiState(state);
  }).then(async () => {
    const revision = wifiRevision;
    const state = await DobsonNative.getWifiState();
    if (wifiRevision === revision) publishWifiState(state);
  }).catch((error) => {
    connectionNotice.textContent = error.message || "Impossible de suivre la connexion Wi-Fi.";
    console.error("Suivi de la connexion Wi-Fi impossible.", error);
  });

  panel.querySelector("#dobson-connect").addEventListener("click", async () => {
    status.textContent = "";
    try {
      const result = await DobsonNative.connect({
        ssid: ssid.value.trim(),
        password: password.value
      });
      publishWifiState({ connected: true, ssid: result.ssid });
      password.value = "";
    } catch (error) {
      status.textContent = error.message || "La connexion Wi-Fi a échoué.";
    }
  });

  panel.querySelector("#dobson-disconnect").addEventListener("click", async () => {
    try {
      await DobsonNative.disconnect({ ssid: ssid.value.trim() });
      publishWifiState({ connected: false, ssid: ssid.value.trim() });
      status.textContent = "";
    } catch (error) {
      status.textContent = error.message || "La déconnexion Wi-Fi a échoué.";
    }
  });

  testButton.addEventListener("click", async () => {
    if (!window.__dobsonEspConnected) {
      testStatus.textContent = "Connectez d’abord l’application au Wi-Fi de l’ESP32 avec le bouton Connecter.";
      return;
    }
    testButton.disabled = true;
    testStatus.textContent = "Lecture TCP de la position et de la résolution en cours…";
    try {
      const { rawAZ, rawALT, ticksAZ, ticksALT } = await DobsonNative.getEncoderStatus();
      const differences = previousEncoderTest
        ? ` Variation depuis le dernier test : AZ ${rawAZ - previousEncoderTest.rawAZ}, ALT ${rawALT - previousEncoderTest.rawALT}.`
        : "";
      previousEncoderTest = { rawAZ, rawALT };
      testStatus.textContent = `Lecture TCP réussie (192.168.4.1:4030). Compteurs : AZ ${rawAZ}, ALT ${rawALT}. Résolutions annoncées : AZ ${ticksAZ}, ALT ${ticksALT}.${differences} Déplacez chaque axe puis relancez le test pour vérifier les variations.`;
    } catch (error) {
      testStatus.textContent = `Échec de lecture TCP (192.168.4.1:4030) : ${error.message || "erreur inconnue"}. Fermez SkySafari et tout autre client, puis réessayez.`;
      console.error("Diagnostic des encodeurs impossible.", error);
    } finally {
      testButton.disabled = false;
    }
  });

  if (isSkyPage) installSkySensorControls();
  if (isConfigPage) installSkySettingsSection(panel);

  if (isSkyPage && navigator.geolocation && !window.__dobsonSimulationActive()) {
    navigator.geolocation.getCurrentPosition(({ coords }) => {
      localStorage.setItem("dobson-mobile-location", JSON.stringify({
        latitude: coords.latitude,
        longitude: coords.longitude
      }));
    }, () => {}, { enableHighAccuracy: true, timeout: 10000, maximumAge: 300000 });
  }
}

function installAstronomyImages() {
  const objectPreview = document.querySelector("#objectPreview");
  if (!objectPreview) return;

  const sourceLink = document.createElement("a");
  sourceLink.className = "dobson-astronomy-source";
  sourceLink.target = "_blank";
  sourceLink.rel = "noopener noreferrer";
  sourceLink.textContent = "Voir la fiche sur Le Journal de l’Astronomie";
  sourceLink.hidden = true;
  const status = document.createElement("output");
  status.className = "dobson-astronomy-status";
  status.setAttribute("role", "status");
  status.setAttribute("aria-live", "polite");
  objectPreview.insertAdjacentElement("afterend", sourceLink);
  sourceLink.insertAdjacentElement("afterend", status);

  let requestId = 0;
  window.__loadSkyObjectImage = async (object) => {
    const currentRequestId = ++requestId;
    const searchUrl = new URL("/", "https://journaldelastronomie.fr");
    searchUrl.searchParams.set("s", object.name || object.commonName || "");
    sourceLink.href = searchUrl.href;
    sourceLink.hidden = false;
    status.textContent = "Recherche d’une image astronomique…";

    try {
      const imageData = await findAstronomyImage(object);
      if (currentRequestId !== requestId) return;
      if (!imageData) {
        status.textContent = "Aucune image de cet objet n’a été trouvée sur le site.";
        return;
      }

      const image = document.createElement("img");
      image.src = imageData.imageUrl;
      image.alt = imageData.alt;
      image.loading = "lazy";
      image.decoding = "async";
      image.referrerPolicy = "no-referrer";
      image.addEventListener("error", () => {
        if (currentRequestId !== requestId) return;
        image.remove();
        status.textContent = "Image indisponible ; consultez la fiche du site.";
      }, { once: true });
      objectPreview.replaceChildren(image);
      sourceLink.href = imageData.articleUrl;
      status.textContent = "Image : Le Journal de l’Astronomie.";
    } catch (error) {
      if (currentRequestId !== requestId) return;
      status.textContent = "Image indisponible ; consultez la fiche du site.";
      console.error("Chargement de l’image astronomique impossible.", error);
    }
  };
}

function installNativeNavigation() {
  const items = [
    { href: "/beta_sky.html", route: "/beta_sky", label: "Vue 3D du ciel", icon: "icon-globe-2" },
    { href: "/index.html", route: "/", label: "Mode Encodeurs", icon: "icon-telescope" },
    { href: "/config.html", route: "/config", label: "Configuration", icon: "icon-settings" }
  ];
  const currentPath = window.location.pathname.replace(/\.html?$/, "").replace(/\/+$/, "") || "/";
  const currentRoute = ({
    "/": "/beta_sky",
    "/index": "/",
    "/station": "/",
    "/calib_page": "/",
    "/simu": "/",
    "/help": "/config",
    "/releasenotes": "/config"
  })[currentPath] || currentPath;
  const navigation = document.querySelector(".bottom-nav");
  const nightModeToggle = document.getElementById("nightModeToggle");

  if (navigation) {
    navigation.setAttribute("aria-label", "Navigation principale");
    navigation.replaceChildren(...items.map((item) => {
      const link = document.createElement("a");
      link.className = "nav-item";
      link.href = item.href;
      link.title = item.label;
      link.setAttribute("aria-label", item.label);
      if (item.route === currentRoute) link.setAttribute("aria-current", "page");
      link.innerHTML = `<span class="nav-icon"><svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#${item.icon}"></use></svg></span>`;
      const label = document.createElement("span");
      label.className = "nav-label";
      label.textContent = item.label;
      link.appendChild(label);
      return link;
    }));
  }

  if (nightModeToggle) {
    nightModeToggle.className = "dobson-night-toggle";
    nightModeToggle.innerHTML = '<svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-moon"></use></svg>';
    document.body.appendChild(nightModeToggle);
  }
}

function installSkySensorControls() {
  const scene = document.querySelector("#scene");
  if (!scene) return;

  document.body.classList.add("dobson-sky-mobile");
  const sceneMenu = document.createElement("details");
  sceneMenu.className = "dobson-scene-menu";
  sceneMenu.innerHTML = '<summary aria-label="Afficher les commandes de la vue du ciel" title="Commandes de la vue du ciel"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-chevron-down"></use></svg></summary><div class="dobson-scene-menu-content"></div>';
  const sceneMenuContent = sceneMenu.querySelector(".dobson-scene-menu-content");
  document.body.appendChild(sceneMenu);

  const modeButton = document.createElement("button");
  modeButton.id = "dobson-sensor-mode";
  modeButton.type = "button";
  modeButton.className = "dobson-sensor-mode";
  modeButton.setAttribute("aria-pressed", "false");
  modeButton.innerHTML = '<svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-compass"></use></svg>';
  document.body.appendChild(modeButton);
  const sensorStatus = document.createElement("output");
  sensorStatus.id = "dobson-sensor-status";
  sensorStatus.className = "dobson-sensor-status";
  sensorStatus.setAttribute("role", "status");
  sensorStatus.setAttribute("aria-live", "polite");
  sceneMenuContent.appendChild(sensorStatus);

  let sensorListener;
  let sensorErrorListener;
  let sensorsStarted = false;
  let skyTrackingActive = false;
  const simulationActive = window.__dobsonSimulationActive();
  let autoFollow = !simulationActive && localStorage.getItem("dobson-sky-control-mode-v3") !== "touch";
  modeButton.disabled = simulationActive;
  let autoFollowBaseline;
  const gestureStarts = new Map();

  const controls = document.querySelector(".controls");
  const sceneTools = document.createElement("div");
  sceneTools.className = "dobson-scene-tools";
  sceneTools.setAttribute("role", "group");
  sceneTools.setAttribute("aria-label", "Actions rapides de la vue du ciel");
  for (const id of ["centerButton"]) {
    const button = document.getElementById(id);
    if (button) sceneTools.appendChild(button);
  }
  document.querySelector(".scene-action-toolbar")?.replaceWith(sceneTools);
  sceneMenuContent.appendChild(sceneTools);

  const settingsButton = document.getElementById("openSettingsModalBtn");
  settingsButton?.remove();

  const alignment = controls?.querySelector(".offset-inline-group");
  if (alignment) {
    const alignmentPanel = document.createElement("details");
    alignmentPanel.className = "dobson-scene-alignment";
    alignmentPanel.innerHTML = '<summary aria-label="Ouvrir les réglages d’offset" title="Réglages d’offset"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-gamepad-2"></use></svg><span>Réglages d’offset</span></summary><div class="dobson-scene-alignment-content"><span>Offset AZ / ALT</span></div>';
    const alignmentContent = alignmentPanel.querySelector(".dobson-scene-alignment-content");
    alignmentContent.appendChild(alignment);
    const offsetReadout = document.querySelector("#viewReadout");
    if (offsetReadout) {
      offsetReadout.classList.remove("scene-label", "view");
      offsetReadout.classList.add("dobson-scene-offset-readout");
      alignmentContent.appendChild(offsetReadout);
    }
    sceneMenuContent.appendChild(alignmentPanel);
  }

  const timeToolbar = controls?.querySelector(".unified-toolbar");
  if (timeToolbar) {
    const timePanel = document.createElement("details");
    timePanel.className = "dobson-scene-time-panel";
    timePanel.innerHTML = '<summary aria-label="Régler la date et l’heure du ciel" title="Date et heure"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-clock-3"></use></svg><span>Date et heure</span></summary><div class="dobson-scene-time-content"></div>';
    timePanel.querySelector(".dobson-scene-time-content").appendChild(timeToolbar);
    sceneMenuContent.appendChild(timePanel);
  }

  controls?.querySelector(".magnitude-filter")?.remove();

  const getPointingOrientation = (orientation) => ({
    ...orientation,
    azimuth: orientation.cameraAzimuth,
    altitude: orientation.cameraAltitude
  });
  const publishOrientation = (orientation) => {
    const pointing = getPointingOrientation(orientation);
    pointing.receivedAt = Date.now();
    window.__dobsonPhoneOrientation = pointing;
    window.dispatchEvent(new CustomEvent("dobson:orientation", { detail: pointing }));
  };

  const updateMode = () => {
    window.__dobsonAutoFollow = autoFollow;
    modeButton.setAttribute("aria-pressed", String(autoFollow));
    modeButton.setAttribute("aria-label", simulationActive
      ? "Suivi par capteurs indisponible pendant la simulation"
      : autoFollow
      ? "Revenir à la navigation tactile"
      : "Activer le pointage automatique par capteurs");
    modeButton.title = simulationActive ? "Suivi par capteurs indisponible pendant la simulation"
      : autoFollow ? "Capteurs actifs — toucher pour naviguer au doigt" : "Navigation tactile — toucher pour suivre les capteurs";
    modeButton.querySelector("use").setAttribute("href", autoFollow
      ? "/icons.svg#icon-compass"
      : "/icons.svg#icon-hand");
    sensorStatus.textContent = "";
  };

  const stopSensorsNow = async () => {
    try {
      if (sensorsStarted) await DobsonNative.stopSensors();
    } finally {
      sensorsStarted = false;
      if (sensorListener) {
        await sensorListener.remove();
        sensorListener = undefined;
      }
      if (sensorErrorListener) {
        await sensorErrorListener.remove();
        sensorErrorListener = undefined;
      }
    }
  };

  const startSensorsNow = async () => {
    if (sensorsStarted) return;
    try {
      sensorListener = await DobsonNative.addListener("orientation", (orientation) => {
        const pointingOrientation = getPointingOrientation(orientation);
        const altitude = Number.isFinite(pointingOrientation.altitude)
          ? pointingOrientation.altitude
          : 90 - Math.abs(pointingOrientation.pitch);
        const currentOrientation = {
          azimuth: pointingOrientation.azimuth,
          altitude: pointingOrientation.altitude
        };
        if (autoFollow && !skyTrackingActive) {
          if (altitude >= 2 || hasSignificantSkyMotion(autoFollowBaseline, currentOrientation, 1.5)) {
            skyTrackingActive = true;
          } else if (!autoFollowBaseline) {
            autoFollowBaseline = currentOrientation;
          }
        }
        if (autoFollow && skyTrackingActive) {
          publishOrientation(orientation);
        }
      });
      sensorErrorListener = await DobsonNative.addListener("sensorError", ({ message }) => {
        sensorStatus.textContent = message || "Le capteur a signalé une erreur.";
        console.error("Erreur du capteur :", message);
      });
      sensorsStarted = true;
      await DobsonNative.startSensors({ intervalMs: 16 });
    } catch (error) {
      try {
        await stopSensorsNow();
      } catch (cleanupError) {
        console.error("Nettoyage des capteurs impossible.", cleanupError);
      }
      autoFollow = false;
      localStorage.setItem("dobson-sky-control-mode-v3", "touch");
      updateMode();
      sensorStatus.textContent = error.message || "Impossible d’activer les capteurs.";
      throw error;
    }
  };

  let sensorQueue = Promise.resolve();
  const queueSensorOperation = (operation) => {
    const result = sensorQueue.then(operation);
    sensorQueue = result.catch((error) => console.error("Commande des capteurs impossible.", error));
    return result;
  };
  const startSensors = () => queueSensorOperation(startSensorsNow);
  const stopSensors = () => queueSensorOperation(stopSensorsNow);

  const enableAutoFollow = async ({ immediate = false } = {}) => {
    if (simulationActive) {
      sensorStatus.textContent = "Quittez la simulation pour utiliser les capteurs et la caméra AR.";
      return false;
    }
    autoFollow = true;
    autoFollowBaseline = undefined;
    skyTrackingActive = immediate;
    window.__dobsonPhoneOrientation = undefined;
    window.dispatchEvent(new Event("dobson:resetTouchOffset"));
    localStorage.setItem("dobson-sky-control-mode-v3", "auto");
    updateMode();
    try {
      await startSensors();
      return true;
    } catch (error) {
      console.error("Impossible d’activer le pointage automatique.", error);
      return false;
    }
  };
  window.__dobsonEnableAutoFollow = enableAutoFollow;

  const enterTouchNavigation = () => {
    if (!autoFollow) return;
    autoFollow = false;
    skyTrackingActive = false;
    localStorage.setItem("dobson-sky-control-mode-v3", "touch");
    updateMode();
    stopSensors().catch((error) => {
      sensorStatus.textContent = error.message || "Impossible d’arrêter les capteurs.";
    });
  };

  modeButton.addEventListener("click", () => {
    if (autoFollow) {
      enterTouchNavigation();
    } else {
      enableAutoFollow();
    }
  });

  const activeScenePointers = new Set();
  scene.addEventListener("pointerdown", (event) => {
    if (event.target instanceof Element && event.target.closest("#dobson-sensor-mode, .scene-zoom-controls, .dobson-scene-alignment, .dobson-scene-time-panel, .dobson-solver-panel, .dobson-scene-tools, button, input, select")) return;
    activeScenePointers.add(event.pointerId);
    if (activeScenePointers.size > 1) {
      gestureStarts.clear();
      return;
    }
    if (autoFollow) gestureStarts.set(event.pointerId, { x: event.clientX, y: event.clientY });
  });
  scene.addEventListener("pointermove", (event) => {
    const start = gestureStarts.get(event.pointerId);
    if (autoFollow && activeScenePointers.size === 1 && start && Math.hypot(event.clientX - start.x, event.clientY - start.y) >= 8) {
      enterTouchNavigation();
    }
  });
  const clearGesture = (event) => {
    activeScenePointers.delete(event.pointerId);
    gestureStarts.delete(event.pointerId);
  };
  scene.addEventListener("pointerup", clearGesture);
  scene.addEventListener("pointercancel", clearGesture);
  document.addEventListener("visibilitychange", () => {
    if (document.hidden) {
      stopSensors().catch((error) => console.error("Impossible d’arrêter les capteurs en arrière-plan.", error));
    } else if (autoFollow) {
      startSensors().catch((error) => console.error("Impossible de redémarrer les capteurs.", error));
    }
  });

  updateMode();
  if (autoFollow) enableAutoFollow();
}

function installSkySettingsSection(connectionPanel) {
  const configForm = document.querySelector("#configForm");
  if (!configForm) return;

  const section = document.createElement("details");
  section.className = "settings-section dobson-config-sky-options";
  section.innerHTML = `
    <summary>Options du ciel<svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-chevron-down"></use></svg></summary>
    <div class="settings-content dobson-config-sky-content">
      <fieldset class="dobson-sky-preference-group">
        <legend>Repères</legend>
        <label><span>Grille et horizon</span><input type="checkbox" data-sky-pref="showGrid"></label>
      </fieldset>
      <fieldset class="dobson-sky-preference-group">
        <legend>Navigation au doigt</legend>
        <label><span>Inverser l’azimut</span><input type="checkbox" id="dobson-touch-reverse-azimuth"></label>
        <label><span>Inverser l’altitude</span><input type="checkbox" id="dobson-touch-reverse-altitude"></label>
      </fieldset>
      <fieldset class="dobson-sky-preference-group">
        <legend>Catalogue céleste</legend>
        <label><span>Messier sans limite de magnitude</span><input type="checkbox" data-sky-pref="showMessierAll"></label>
        <div class="dobson-sky-preference-headings"><span>Catégorie</span><span>Objets</span><span>Noms</span></div>
        <label class="dobson-sky-object-preference"><span>Galaxies</span><input type="checkbox" data-sky-pref="showGal"><input type="checkbox" data-sky-pref="labelGal" aria-label="Afficher les noms des galaxies"></label>
        <label class="dobson-sky-object-preference"><span>Nébuleuses</span><input type="checkbox" data-sky-pref="showNeb"><input type="checkbox" data-sky-pref="labelNeb" aria-label="Afficher les noms des nébuleuses"></label>
        <label class="dobson-sky-object-preference"><span>Amas d’étoiles</span><input type="checkbox" data-sky-pref="showAmas"><input type="checkbox" data-sky-pref="labelAmas" aria-label="Afficher les noms des amas d’étoiles"></label>
        <label class="dobson-sky-object-preference"><span>Étoiles</span><input type="checkbox" data-sky-pref="showStars"><input type="checkbox" data-sky-pref="labelStars" aria-label="Afficher les noms des étoiles"></label>
        <label class="dobson-sky-object-preference"><span>Constellations</span><input type="checkbox" data-sky-pref="showConst"><input type="checkbox" data-sky-pref="labelConst" aria-label="Afficher les noms des constellations"></label>
      </fieldset>
      <div class="dobson-sky-magnitude">
        <label for="dobson-sky-magnitude">Magnitude maximale <output id="dobson-sky-magnitude-value">Toutes</output></label>
        <input id="dobson-sky-magnitude" type="range" min="0" max="1000" step="1" value="1000" aria-valuetext="Toutes les magnitudes">
        <div class="dobson-sky-magnitude-scale" aria-hidden="true"><span>0</span><span>6</span><span>8</span><span>Toutes</span></div>
      </div>
      <output class="dobson-sky-preference-status" role="status" aria-live="polite"></output>
    </div>`;
  const currentProjection = configForm.querySelector("#projectionModeSelect");
  const projectionSection = currentProjection?.closest(".settings-section");
  configForm.prepend(section);
  const additionalPages = document.createElement("details");
  additionalPages.className = "settings-section";
  additionalPages.innerHTML = '<summary>Autres fonctions<svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-chevron-down"></use></svg></summary><div class="settings-content"><p><a href="/station.html">Mise en station</a></p><p><a href="/help.html">Aide</a></p></div>';
  configForm.appendChild(additionalPages);

  const skyContent = section.querySelector(".dobson-config-sky-content");
  for (const id of ["dobson-touch-reverse-azimuth", "dobson-touch-reverse-altitude"]) {
    const input = section.querySelector(`#${id}`);
    input.checked = localStorage.getItem(id) === "true";
    input.addEventListener("change", () => localStorage.setItem(id, String(input.checked)));
  }
  if (projectionSection) {
    projectionSection.classList.add("dobson-projection-options");
    skyContent.prepend(projectionSection);
  }

  const findSection = (prefix, parent = configForm) => [...parent.children].find((element) =>
    element.matches("details.settings-section")
      && element.querySelector(":scope > summary")?.textContent.trim().startsWith(prefix));
  const technicalSection = document.createElement("details");
  technicalSection.className = "settings-section dobson-technical-options";
  technicalSection.innerHTML = `
    <summary>Paramètres techniques<svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-chevron-down"></use></svg></summary>
    <div class="settings-content dobson-technical-content"></div>`;
  const technicalContent = technicalSection.querySelector(".dobson-technical-content");
  for (const prefix of ["Encodeurs", "Broches GPIO", "Sens de rotation"]) {
    const subsection = findSection(prefix);
    if (subsection) technicalContent.appendChild(subsection);
  }

  const locationSection = document.createElement("details");
  locationSection.className = "settings-section dobson-location-options";
  locationSection.innerHTML = `
    <summary>Lieu d’observation<svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-chevron-down"></use></svg></summary>
    <div class="settings-content dobson-location-content"></div>`;
  const locationContent = locationSection.querySelector(".dobson-location-content");
  for (const prefix of ["Lieu d'observation actif", "Ajouter un nouveau lieu"]) {
    const subsection = findSection(prefix);
    if (subsection) locationContent.appendChild(subsection);
  }
  const siteLatitude = locationContent.querySelector("#newSiteLat");
  const siteLongitude = locationContent.querySelector("#newSiteLon");
  if (siteLatitude && siteLongitude) {
    const gpsButton = document.createElement("button");
    gpsButton.type = "button";
    gpsButton.textContent = "Utiliser ma position GPS";
    const gpsStatus = document.createElement("output");
    gpsStatus.setAttribute("role", "status");
    gpsStatus.setAttribute("aria-live", "polite");
    siteLongitude.parentElement.insertAdjacentElement("afterend", gpsButton);
    gpsButton.insertAdjacentElement("afterend", gpsStatus);
    gpsButton.addEventListener("click", () => {
      if (!navigator.geolocation) {
        gpsStatus.textContent = "La géolocalisation n’est pas disponible sur cet appareil.";
        return;
      }
      gpsButton.disabled = true;
      gpsStatus.textContent = "Recherche de la position GPS…";
      navigator.geolocation.getCurrentPosition(({ coords }) => {
        siteLatitude.value = coords.latitude.toFixed(5);
        siteLongitude.value = coords.longitude.toFixed(5);
        gpsStatus.textContent = `Position obtenue (précision : ${Math.round(coords.accuracy)} m). Donnez un nom au lieu puis ajoutez-le.`;
        gpsButton.disabled = false;
      }, (error) => {
        gpsStatus.textContent = `Impossible d’obtenir la position GPS : ${error.message}`;
        gpsButton.disabled = false;
      }, { enableHighAccuracy: true, timeout: 15000, maximumAge: 0 });
    });
  }

  const card = configForm.closest(".card");
  const advancedSection = [...card.children].find((element) =>
    element !== configForm
      && element.matches("details.settings-section")
      && element.querySelector(":scope > summary")?.textContent.trim().startsWith("Outils avancés"));
  if (advancedSection) {
    technicalContent.appendChild(advancedSection);
    installAdvancedPlateSolver(advancedSection.querySelector(".settings-content"));
  }
  const astrometrySection = document.createElement("details");
  astrometrySection.className = "settings-section";
  astrometrySection.innerHTML = `
    <summary>Clé API Astrometry.net<svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-chevron-down"></use></svg></summary>
    <div class="settings-content dobson-astrometry-settings">
      <label for="dobson-astrometry-key">Clé API Astrometry.net</label>
      <input id="dobson-astrometry-key" type="text" autocomplete="off" autocapitalize="off" spellcheck="false">
      <div class="dobson-solver-actions">
        <button id="dobson-save-astrometry-key" type="button">Enregistrer la clé</button>
        <button id="dobson-clear-astrometry-key" type="button">Effacer la clé</button>
      </div>
      <output role="status" aria-live="polite"></output>
    </div>`;
  technicalContent.appendChild(astrometrySection);
  const apiKeyInput = astrometrySection.querySelector("#dobson-astrometry-key");
  const apiKeyStatus = astrometrySection.querySelector("output");
  DobsonNative.getAstrometryApiKey().then(({ apiKey }) => {
    apiKeyInput.value = apiKey;
  }).catch((error) => {
    apiKeyStatus.textContent = error.message || "Impossible de lire la clé API enregistrée.";
    console.error("Lecture de la clé Astrometry.net impossible.", error);
  });
  astrometrySection.querySelector("#dobson-save-astrometry-key").addEventListener("click", async () => {
    const apiKey = apiKeyInput.value.trim();
    if (!apiKey) {
      apiKeyStatus.textContent = "Saisissez une clé API avant de l’enregistrer.";
      return;
    }
    try {
      await DobsonNative.saveAstrometryApiKey({ apiKey });
      apiKeyInput.value = apiKey;
      apiKeyStatus.textContent = "Clé API enregistrée.";
    } catch (error) {
      apiKeyStatus.textContent = error.message || "Impossible d’enregistrer la clé API.";
      console.error("Enregistrement de la clé Astrometry.net impossible.", error);
    }
  });
  astrometrySection.querySelector("#dobson-clear-astrometry-key").addEventListener("click", async () => {
    try {
      await DobsonNative.clearAstrometryApiKey();
      apiKeyInput.value = "";
      apiKeyStatus.textContent = "Clé API effacée.";
    } catch (error) {
      apiKeyStatus.textContent = error.message || "Impossible d’effacer la clé API.";
      console.error("Suppression de la clé Astrometry.net impossible.", error);
    }
  });
  connectionPanel.classList.add("settings-section", "dobson-config-connection-panel");
  connectionPanel.querySelector("summary").insertAdjacentHTML("beforeend", '<svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-chevron-down"></use></svg>');
  technicalContent.appendChild(connectionPanel);
  const configurationStatus = document.createElement("output");
  configurationStatus.className = "dobson-config-save-status";
  configurationStatus.setAttribute("role", "status");
  configurationStatus.setAttribute("aria-live", "polite");
  technicalContent.appendChild(configurationStatus);

  configForm.querySelector('button[type="submit"]')?.remove();
  configForm.removeAttribute("onsubmit");
  configForm.append(locationSection, technicalSection);

  let configSaveTimer;
  let configSaveQueue = Promise.resolve();
  const persistConfiguration = () => {
    configSaveQueue = configSaveQueue.then(async () => {
      const latitudeText = document.querySelector("#lat").value.trim();
      const longitudeText = document.querySelector("#lon").value.trim();
      const latitude = Number(latitudeText);
      const longitude = Number(longitudeText);
      if (!latitudeText || !longitudeText || !Number.isFinite(latitude) || !Number.isFinite(longitude)
        || Math.abs(latitude) > 90 || Math.abs(longitude) > 180) {
        configurationStatus.textContent = "Saisissez une latitude et une longitude valides.";
        return;
      }

      const siteSelect = document.querySelector("#siteSelect");
      const parameters = new URLSearchParams({
        action: "set_config",
        ticksAZ: document.querySelector("#ticksAZ").value,
        ticksALT: document.querySelector("#ticksALT").value,
        revAZ: document.querySelector("#revAZ").checked ? "1" : "0",
        revALT: document.querySelector("#revALT").checked ? "1" : "0",
        lat: latitudeText,
        lon: longitudeText,
        siteName: siteSelect.selectedOptions[0]?.textContent.trim() || "Lieu personnalisé"
      });

      if (!window.__dobsonSimulationActive()) {
        localStorage.setItem("dobson-mobile-location", JSON.stringify({ latitude, longitude }));
      }
      try {
        const response = await fetch(`/cmd?${parameters}`);
        if (!response.ok) throw new Error("Impossible d’enregistrer la configuration.");
        const result = await response.json();
        if (result.status !== "OK") throw new Error("La configuration locale n’a pas été enregistrée.");
        configurationStatus.textContent = "";
      } catch (error) {
        configurationStatus.textContent = error.message || "Impossible d’enregistrer la configuration.";
        console.error("Enregistrement automatique de la configuration impossible.", error);
      }
    });
  };
  const scheduleConfigurationSave = () => {
    clearTimeout(configSaveTimer);
    configSaveTimer = setTimeout(persistConfiguration, 300);
  };
  const configurationFields = new Set(["ticksAZ", "ticksALT", "revAZ", "revALT", "siteSelect", "lat", "lon"]);
  configForm.addEventListener("change", (event) => {
    if (configurationFields.has(event.target.id)) scheduleConfigurationSave();
  });
  const siteSelect = configForm.querySelector("#siteSelect");
  if (siteSelect) {
    new MutationObserver(scheduleConfigurationSave).observe(siteSelect, { childList: true });
  }
  configForm.addEventListener("submit", (event) => {
    event.preventDefault();
    scheduleConfigurationSave();
  });

  const inputs = [...section.querySelectorAll("[data-sky-pref]")];
  const status = section.querySelector(".dobson-sky-preference-status");
  const magnitude = section.querySelector("#dobson-sky-magnitude");
  const magnitudeValue = section.querySelector("#dobson-sky-magnitude-value");
  let skyPreferences = {};

  const magnitudeFromPosition = (position) => {
    if (position >= 1000) return 99;
    if (position <= 700) return position * 6 / 700;
    if (position <= 850) return 6 + (position - 700) * 2 / 150;
    return 8 + (position - 850) * 4 / 149;
  };
  const positionFromMagnitude = (value) => {
    if (!Number.isFinite(value) || value >= 13) return 1000;
    const bounded = Math.max(0, Math.min(12, value));
    if (bounded <= 6) return Math.round(bounded * 700 / 6);
    if (bounded <= 8) return Math.round(700 + (bounded - 6) * 150 / 2);
    return Math.round(850 + (bounded - 8) * 149 / 4);
  };
  const updateMagnitudeLabel = () => {
    const position = Number(magnitude.value);
    const allMagnitudes = position >= 1000;
    magnitudeValue.textContent = allMagnitudes ? "Toutes" : `≤ ${magnitudeFromPosition(position).toFixed(2)}`;
    magnitude.setAttribute("aria-valuetext", allMagnitudes
      ? "Toutes les magnitudes"
      : `Magnitude maximale ${magnitudeFromPosition(position).toFixed(2)}`);
  };
  const syncNameControls = () => {
    for (const [objectKey, labelKey] of [
      ["showGal", "labelGal"],
      ["showNeb", "labelNeb"],
      ["showAmas", "labelAmas"],
      ["showStars", "labelStars"],
      ["showConst", "labelConst"]
    ]) {
      const objectInput = inputs.find((input) => input.dataset.skyPref === objectKey);
      const labelInput = inputs.find((input) => input.dataset.skyPref === labelKey);
      labelInput.disabled = !objectInput.checked;
      if (!objectInput.checked) labelInput.checked = false;
    }
  };
  const saveSkyPreferences = async (updates = {}) => {
    skyPreferences = {
      ...skyPreferences,
      ...updates,
      projMode: currentProjection?.value || skyPreferences.projMode || "actuel"
    };
    const query = new URLSearchParams(
      Object.entries(skyPreferences).map(([key, value]) => [key, String(value)])
    );
    const response = await fetch(`/save_sky_prefs?${query}`);
    if (!response.ok) throw new Error("Impossible d’enregistrer les options du ciel.");
    status.textContent = "";
  };
  window.__saveMobileSkyPrefs = saveSkyPreferences;

  const loadSkyPreferences = async () => {
    try {
      const response = await fetch("/get_sky_prefs");
      if (!response.ok) throw new Error("Impossible de charger les options du ciel.");
      skyPreferences = await response.json();
      if (currentProjection && ["actuel", "stéréo", "gnomon"].includes(skyPreferences.projMode)) {
        currentProjection.value = skyPreferences.projMode;
      }
      for (const input of inputs) {
        const value = skyPreferences[input.dataset.skyPref];
        input.checked = value === undefined ? input.dataset.skyPref !== "showMessierAll" : value === true || value === 1 || value === "1";
        input.disabled = false;
      }
      magnitude.disabled = false;
      magnitude.value = positionFromMagnitude(Number(skyPreferences.mag ?? 99));
      updateMagnitudeLabel();
      syncNameControls();
      section.classList.add("dobson-sky-preferences-ready");
      status.textContent = "";
    } catch (error) {
      for (const input of inputs) input.disabled = false;
      magnitude.disabled = false;
      syncNameControls();
      section.classList.add("dobson-sky-preferences-ready");
      status.textContent = error.message || "Impossible de charger les options du ciel.";
      console.error("Chargement des options du ciel impossible.", error);
    }
  };

  for (const input of inputs) {
    input.disabled = true;
    input.addEventListener("change", () => {
      syncNameControls();
      const values = Object.fromEntries(inputs.map((item) => [item.dataset.skyPref, item.checked ? 1 : 0]));
      saveSkyPreferences(values).catch((error) => {
        status.textContent = error.message;
        console.error("Enregistrement des options du ciel impossible.", error);
      });
    });
  }
  magnitude.disabled = true;
  magnitude.addEventListener("input", updateMagnitudeLabel);
  magnitude.addEventListener("change", () => {
    saveSkyPreferences({ mag: magnitudeFromPosition(Number(magnitude.value)) }).catch((error) => {
      status.textContent = error.message;
      console.error("Enregistrement de la magnitude maximale impossible.", error);
    });
  });
  loadSkyPreferences();
}

function installAugmentedReality() {
  const toggle = document.querySelector("#dobson-ar-toggle");
  const video = document.querySelector("#dobson-ar-video");
  const scene = document.querySelector("#scene");
  const status = document.querySelector("#dobson-sensor-status");
  if (!toggle || !video || !scene) return;
  const marker = document.createElement("button");
  marker.id = "dobson-telescope-marker";
  marker.type = "button";
  marker.hidden = true;
  marker.setAttribute("aria-label", "Position du télescope : déplacer le cercle ou utiliser les flèches");
  marker.title = "Déplacer vers la position du télescope";
  scene.appendChild(marker);
  const moveMarker = (clientX, clientY) => {
    const bounds = scene.getBoundingClientRect();
    const x = Math.max(0, Math.min(bounds.width, clientX - bounds.left));
    const y = Math.max(0, Math.min(bounds.height, clientY - bounds.top));
    if (!window.__dobsonSetTelescopeMarker(2 * x / bounds.width - 1, 1 - 2 * y / bounds.height)) {
      status.textContent = "Cette position est hors de la zone de projection.";
      document.querySelector(".dobson-scene-menu").open = true;
    } else {
      status.textContent = "";
    }
  };
  let markerPointer;
  let markerGrab;
  marker.addEventListener("pointerdown", (event) => {
    if (markerPointer !== undefined) return;
    event.preventDefault();
    event.stopPropagation();
    markerPointer = event.pointerId;
    const bounds = marker.getBoundingClientRect();
    markerGrab = { x: event.clientX - (bounds.left + bounds.width / 2), y: event.clientY - (bounds.top + bounds.height / 2) };
    marker.setPointerCapture(event.pointerId);
  });
  marker.addEventListener("pointermove", (event) => {
    if (event.pointerId !== markerPointer) return;
    event.preventDefault();
    event.stopPropagation();
    moveMarker(event.clientX - markerGrab.x, event.clientY - markerGrab.y);
  });
  const releaseMarker = (event) => {
    if (event.pointerId !== markerPointer) return;
    event.stopPropagation();
    markerPointer = undefined;
    markerGrab = undefined;
  };
  marker.addEventListener("pointerup", releaseMarker);
  marker.addEventListener("pointercancel", releaseMarker);
  marker.addEventListener("keydown", (event) => {
    const directions = { ArrowLeft: [-5, 0], ArrowRight: [5, 0], ArrowUp: [0, -5], ArrowDown: [0, 5] };
    const delta = directions[event.key];
    if (!delta) return;
    event.preventDefault();
    const bounds = marker.getBoundingClientRect();
    moveMarker(bounds.left + bounds.width / 2 + delta[0], bounds.top + bounds.height / 2 + delta[1]);
  });
  document.querySelector(".dobson-scene-menu-content")?.appendChild(toggle);
  toggle.title = "Caméra en réalité augmentée et pointage automatique";

  const capture = document.createElement("button");
  capture.type = "button";
  capture.className = "dobson-ar-capture";
  capture.setAttribute("aria-label", "Prendre une photo du ciel");
  capture.title = "Prendre une photo du ciel";
  capture.innerHTML = '<svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-camera"></use></svg>';
  capture.hidden = true;
  scene.appendChild(capture);
  const openPhoto = document.createElement("button");
  openPhoto.type = "button";
  openPhoto.className = "dobson-ar-photo-open";
  openPhoto.textContent = "IA";
  openPhoto.setAttribute("aria-label", "Ouvrir la dernière photo pour la résolution IA");
  openPhoto.hidden = true;
  scene.appendChild(openPhoto);

  const photoDialog = document.createElement("div");
  photoDialog.className = "dobson-photo-dialog";
  photoDialog.hidden = true;
  photoDialog.setAttribute("role", "dialog");
  photoDialog.setAttribute("aria-modal", "true");
  photoDialog.setAttribute("aria-labelledby", "dobson-photo-title");
  photoDialog.innerHTML = `
    <div class="dobson-photo-content">
      <h2 id="dobson-photo-title">Photo du ciel</h2>
      <img alt="Photo prise avec la caméra arrière">
      <p>Le bouton IA envoie la photo à Astrometry.net et tente de réaligner le centre de la vue. La rotation et le champ de la caméra ne sont pas calibrés.</p>
      <div class="dobson-photo-actions">
        <button type="button" class="dobson-photo-solve">IA</button>
        <a class="dobson-photo-download">Télécharger</a>
        <button type="button" class="dobson-photo-close">Fermer</button>
      </div>
      <output role="status" aria-live="polite"></output>
    </div>`;
  document.body.appendChild(photoDialog);
  const photoPreview = photoDialog.querySelector("img");
  const photoSolve = photoDialog.querySelector(".dobson-photo-solve");
  const photoDownload = photoDialog.querySelector(".dobson-photo-download");
  const photoStatus = photoDialog.querySelector("output");
  const photoClose = photoDialog.querySelector(".dobson-photo-close");
  let capturedImage;
  let capturedCalibration;
  let photoUrl;
  let solvingPhoto = false;
  let capturing = false;

  openPhoto.addEventListener("click", () => {
    photoDialog.hidden = false;
    photoSolve.focus();
  });
  photoClose.addEventListener("click", () => {
    photoDialog.hidden = true;
    if (!openPhoto.hidden) openPhoto.focus();
    else toggle.focus();
  });
  photoSolve.addEventListener("click", async () => {
    if (!capturedImage || solvingPhoto) return;
    solvingPhoto = true;
    photoSolve.disabled = true;
    capture.disabled = true;
    photoStatus.textContent = "Connexion à Astrometry.net…";
    try {
      photoStatus.textContent = await solveImage(capturedImage, (message) => { photoStatus.textContent = message; }, (solution) => {
        window.__dobsonApplyPhotoCalibration(solution, capturedCalibration);
      });
      photoStatus.textContent += " Centre de la vue réaligné ; rotation et champ non calibrés.";
    } catch (error) {
      photoStatus.textContent = error.message || "La résolution astrométrique a échoué.";
      console.error("Résolution de la photo AR impossible.", error);
    } finally {
      solvingPhoto = false;
      photoSolve.disabled = false;
      capture.disabled = false;
    }
  });

  let stream;
  let starting = false;
  let generation = 0;
  const stop = () => {
    generation++;
    if (stream) {
      stream.getTracks().forEach((track) => track.stop());
      stream = undefined;
    }
    video.srcObject = null;
    scene.classList.remove("dobson-ar-active");
    toggle.setAttribute("aria-pressed", "false");
    toggle.setAttribute("aria-label", "Activer la caméra en réalité augmentée");
    toggle.textContent = "Caméra AR";
    capture.hidden = true;
    openPhoto.hidden = true;
    marker.hidden = true;
  };

  toggle.addEventListener("click", async () => {
    if (stream) {
      stop();
      return;
    }
    if (starting) return;
    if (window.__dobsonSimulationActive()) {
      status.textContent = "Quittez la simulation pour utiliser la caméra AR.";
      document.querySelector(".dobson-scene-menu").open = true;
      return;
    }
    if (!navigator.mediaDevices?.getUserMedia) {
      status.textContent = "La caméra n’est pas disponible sur cet appareil.";
      console.error(status.textContent);
      return;
    }
    starting = true;
    const requestGeneration = generation;
    try {
      const newStream = await navigator.mediaDevices.getUserMedia({
        audio: false,
        video: { facingMode: { ideal: "environment" } }
      });
      if (requestGeneration !== generation || document.hidden) {
        newStream.getTracks().forEach((track) => track.stop());
        return;
      }
      stream = newStream;
      video.srcObject = stream;
      await video.play();
      if (requestGeneration !== generation) return;
      scene.classList.add("dobson-ar-active");
      toggle.setAttribute("aria-pressed", "true");
      toggle.setAttribute("aria-label", "Désactiver la caméra en réalité augmentée");
      toggle.textContent = "Arrêter AR";
      capture.hidden = false;
      openPhoto.hidden = !capturedImage;
      await window.__dobsonEnableAutoFollow({ immediate: true });
    } catch (error) {
      stop();
      status.textContent = error.message || "Impossible d’accéder à la caméra.";
      console.error("Activation de la caméra AR impossible.", error);
    } finally {
      starting = false;
    }
  });

  const captureVideoFrame = async () => {
    if (video.readyState < HTMLMediaElement.HAVE_CURRENT_DATA || !video.videoWidth || !video.videoHeight) {
      throw new Error("La caméra n’a pas encore fourni d’image. Réessayez dans un instant.");
    }
    const canvas = document.createElement("canvas");
    canvas.width = video.videoWidth;
    canvas.height = video.videoHeight;
    const context = canvas.getContext("2d");
    if (!context) throw new Error("Impossible de préparer la photo.");
    context.drawImage(video, 0, 0);
    const blob = await new Promise((resolve) => canvas.toBlob(resolve, "image/jpeg", 0.95));
    if (!blob) throw new Error("Impossible de créer la photo.");
    return blob;
  };

  capture.addEventListener("click", async () => {
    if (capturing) return;
    capturing = true;
    capture.disabled = true;
    try {
      const calibration = window.__dobsonCaptureCalibration();
      const blob = await captureVideoFrame();
      capturedImage = new File([blob], `ciel-${Date.now()}.jpg`, { type: "image/jpeg" });
      capturedCalibration = calibration;
      if (photoUrl) URL.revokeObjectURL(photoUrl);
      photoUrl = URL.createObjectURL(capturedImage);
      photoPreview.src = photoUrl;
      photoDownload.href = photoUrl;
      photoDownload.download = capturedImage.name;
      photoStatus.textContent = "Enregistrement dans AstroPhoto…";
      photoSolve.disabled = true;
      openPhoto.hidden = false;
      photoDialog.hidden = false;
      photoSolve.focus();
      const base64 = await new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = () => {
          if (typeof reader.result !== "string" || !reader.result.includes(",")) {
            reject(new Error("Impossible de préparer la photo pour la galerie."));
            return;
          }
          resolve(reader.result.slice(reader.result.indexOf(",") + 1));
        };
        reader.onerror = () => reject(reader.error || new Error("Impossible de lire la photo."));
        reader.readAsDataURL(capturedImage);
      });
      await DobsonNative.saveAstroPhoto({ base64, filename: capturedImage.name });
      photoStatus.textContent = "Photo enregistrée dans AstroPhoto.";
    } catch (error) {
      if (photoDialog.hidden) {
        status.textContent = error.message || "Impossible de prendre la photo.";
        document.querySelector(".dobson-scene-menu").open = true;
      } else {
        photoStatus.textContent = error.message || "Impossible d’enregistrer la photo dans AstroPhoto.";
      }
      console.error("Capture ou enregistrement de la photo impossible.", error);
    } finally {
      capturing = false;
      capture.disabled = false;
      photoSolve.disabled = false;
    }
  });

  document.addEventListener("visibilitychange", () => {
    if (document.hidden && (stream || starting)) stop();
  });
  window.addEventListener("pagehide", () => {
    if (photoUrl) URL.revokeObjectURL(photoUrl);
  });
}

async function solveImage(image, reportStatus, onCalibration) {
  const { apiKey } = await DobsonNative.getAstrometryApiKey();
  if (!apiKey) throw new Error("Enregistrez d’abord votre clé API dans les paramètres techniques.");
  return solveWithAstrometry(image, apiKey, reportStatus, { onCalibration });
}

function installAdvancedPlateSolver(container) {
  if (!container) return;
  const section = document.createElement("details");
  section.className = "dobson-solver-panel";
  section.innerHTML = `
    <summary aria-label="Ouvrir la résolution astrométrique" title="Résolution astrométrique">
      <svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-sparkles"></use></svg>
      <span>Résolution d’image</span>
    </summary>
    <div class="dobson-solver-content">
      <p>Identifier le centre et l’échelle d’une image avec Astrometry.net. Configurez la clé API dans les paramètres techniques. L’image n’est envoyée qu’à votre demande et est marquée privée.</p>
      <label for="dobson-solver-image">Image du ciel</label>
      <input id="dobson-solver-image" type="file" accept="image/*">
      <button id="dobson-solve-image" type="button">Résoudre en ligne</button>
      <output id="dobson-solver-status" role="status" aria-live="polite"></output>
    </div>`;
  container.appendChild(section);

  const imageInput = section.querySelector("#dobson-solver-image");
  const solveButton = section.querySelector("#dobson-solve-image");
  const solverStatus = section.querySelector("#dobson-solver-status");
  solveButton.addEventListener("click", async () => {
    const image = imageInput.files?.[0];
    if (!image) {
      solverStatus.textContent = "Choisissez une image du ciel.";
      return;
    }

    solveButton.disabled = true;
    solverStatus.textContent = "Connexion à Astrometry.net…";
    try {
      const result = await solveImage(image, (message) => {
        solverStatus.textContent = message;
      });
      solverStatus.textContent = result;
    } catch (error) {
      solverStatus.textContent = error.message || "La résolution astrométrique a échoué.";
      console.error("Résolution astrométrique impossible.", error);
    } finally {
      solveButton.disabled = false;
    }
  });

  imageInput.addEventListener("change", () => {
    solverStatus.textContent = "";
  });
}
