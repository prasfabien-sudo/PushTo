import { Capacitor } from "@capacitor/core";
import { DobsonNative } from "@dobson/capacitor-dobson";
import { loadMobileCatalog } from "./mobile-catalog.js";
import { solveWithAstrometry } from "./astrometry.js";
import { findAstronomyImage } from "./astronomy-images.js";
import { hasSignificantSkyMotion } from "./sky-motion.js";
import "./mobile.css";

window.__dobsonNativeCatalogPromise = Capacitor.isNativePlatform()
  ? loadMobileCatalog(window.location.pathname, window.fetch.bind(window))
  : Promise.resolve(null);
window.__dobsonNativeCatalogPromise.then(window.__resolveDobsonCatalog, window.__rejectDobsonCatalog);

if (Capacitor.isNativePlatform()) {
  document.addEventListener("DOMContentLoaded", () => {
    installNativePanel();
    installAstronomyImages();
    installAugmentedReality();
    installPlateSolver();
  }, { once: true });
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
      <label for="dobson-api-base">Adresse du serveur ESP32</label>
      <input id="dobson-api-base" type="url" value="" inputmode="url" autocomplete="url">
      <label for="dobson-wifi-password">Mot de passe Wi-Fi (si nécessaire)</label>
      <input id="dobson-wifi-password" type="password" autocomplete="new-password">
      <div class="dobson-native-actions">
        <button id="dobson-connect" type="button">Connecter</button>
        <button id="dobson-disconnect" type="button">Déconnecter</button>
      </div>
      <output id="dobson-native-status" role="status" aria-live="polite"></output>
    </div>`;
  const isSkyPage = window.location.pathname.endsWith("/beta_sky.html");
  const isConfigPage = window.location.pathname.endsWith("/config.html");
  if (!isSkyPage && !isConfigPage) document.body.prepend(panel);

  const ssid = panel.querySelector("#dobson-ssid");
  const apiBase = panel.querySelector("#dobson-api-base");
  const password = panel.querySelector("#dobson-wifi-password");
  const status = panel.querySelector("#dobson-native-status");

  const publishWifiState = (state) => {
    window.__dobsonEspConnected = state.connected;
    window.dispatchEvent(new CustomEvent("dobson:wifiState", { detail: state }));
    status.textContent = state.connected
      ? `Connecté à ${state.ssid}.`
      : "La connexion au réseau ESP32 a été interrompue.";
  };
  DobsonNative.addListener("wifiState", publishWifiState).catch((error) => {
    status.textContent = error.message || "Impossible de suivre la connexion Wi-Fi.";
  });

  apiBase.value = localStorage.getItem("dobsonApiBase") || "http://192.168.4.1";
  apiBase.addEventListener("change", () => {
    try {
      const parsed = new URL(apiBase.value);
      if (parsed.protocol !== "http:" && parsed.protocol !== "https:") {
        throw new Error("L’adresse doit utiliser HTTP ou HTTPS.");
      }
      localStorage.setItem("dobsonApiBase", parsed.origin);
      apiBase.value = parsed.origin;
      status.textContent = "";
    } catch (error) {
      status.textContent = error.message || "Adresse ESP32 invalide.";
    }
  });

  panel.querySelector("#dobson-connect").addEventListener("click", async () => {
    status.textContent = "Demande de connexion au réseau Wi-Fi…";
    try {
      const result = await DobsonNative.connect({
        ssid: ssid.value.trim(),
        password: password.value
      });
      publishWifiState({ connected: true, ssid: result.ssid });
      password.value = "";
      if (result.baseUrl) {
        localStorage.setItem("dobsonApiBase", result.baseUrl);
        apiBase.value = result.baseUrl;
      }
      status.textContent = `Connecté à ${result.ssid}.`;
    } catch (error) {
      status.textContent = error.message || "La connexion Wi-Fi a échoué.";
    }
  });

  panel.querySelector("#dobson-disconnect").addEventListener("click", async () => {
    try {
      await DobsonNative.disconnect({ ssid: ssid.value.trim() });
      publishWifiState({ connected: false, ssid: ssid.value.trim() });
      status.textContent = "Déconnexion demandée.";
    } catch (error) {
      status.textContent = error.message || "La déconnexion Wi-Fi a échoué.";
    }
  });

  if (isSkyPage) installSkySensorControls();
  if (isConfigPage) installSkySettingsSection(panel);

  if (isSkyPage && navigator.geolocation) {
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
    { href: "/station.html", route: "/station", label: "Mise en station", icon: "icon-crosshair" },
    { href: "/config.html", route: "/config", label: "Configuration", icon: "icon-settings" },
    { href: "/help.html", route: "/help", label: "Aide", icon: "icon-book-open" }
  ];
  const currentPath = window.location.pathname.replace(/\.html?$/, "").replace(/\/+$/, "") || "/";
  const currentRoute = ({
    "/": "/beta_sky",
    "/index": "/",
    "/calib_page": "/station",
    "/simu": "/",
    "/releasenotes": "/help"
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
  modeButton.innerHTML = '<svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-compass"></use></svg><span></span>';
  sceneMenuContent.appendChild(modeButton);

  const modeLabel = modeButton.querySelector("span");
  const sensorStatus = document.createElement("output");
  sensorStatus.id = "dobson-sensor-status";
  sensorStatus.className = "dobson-sensor-status";
  sensorStatus.setAttribute("role", "status");
  sensorStatus.setAttribute("aria-live", "polite");
  scene.appendChild(sensorStatus);

  let sensorListener;
  let sensorErrorListener;
  let sensorsStarted = false;
  let skyTrackingActive = false;
  let autoFollow = localStorage.getItem("dobson-sky-control-mode-v3") !== "touch";
  let autoFollowBaseline;
  let lastSensorStatus = 0;
  let lastOrientation;
  const gestureStarts = new Map();

  const controls = document.querySelector(".controls");
  const sceneTools = document.createElement("div");
  sceneTools.className = "dobson-scene-tools";
  sceneTools.setAttribute("role", "group");
  sceneTools.setAttribute("aria-label", "Actions rapides de la vue du ciel");
  for (const id of ["centerButton", "manualFullscreenBtn"]) {
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
    window.__dobsonPhoneOrientation = pointing;
    window.dispatchEvent(new CustomEvent("dobson:orientation", { detail: pointing }));
  };

  const updateMode = () => {
    modeButton.setAttribute("aria-pressed", String(autoFollow));
    modeButton.setAttribute("aria-label", autoFollow
      ? "Revenir à la navigation tactile"
      : "Activer le pointage automatique par capteurs");
    modeButton.title = autoFollow ? "Capteurs actifs — toucher pour naviguer au doigt" : "Navigation tactile — toucher pour suivre les capteurs";
    modeLabel.textContent = autoFollow ? "Auto" : "Doigt";
    sensorStatus.textContent = autoFollow
      ? "Capteurs automatiques : attente d’un mouvement du téléphone"
      : "Navigation tactile";
  };

  const stopSensors = async () => {
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

  const startSensors = async () => {
    if (sensorsStarted) return;
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
        if (altitude >= 2 || hasSignificantSkyMotion(autoFollowBaseline, currentOrientation)) {
          skyTrackingActive = true;
        } else if (!autoFollowBaseline) {
          autoFollowBaseline = currentOrientation;
        }
      }
      if (skyTrackingActive) {
        lastOrientation = orientation;
        publishOrientation(orientation);
      }
      const now = performance.now();
      if (now - lastSensorStatus >= 250) {
        lastSensorStatus = now;
        sensorStatus.textContent = autoFollow
          ? skyTrackingActive
            ? `Pointage automatique · altitude ${altitude.toFixed(0)}°`
            : "Déplacez le téléphone pour activer le pointage"
          : "Navigation tactile";
      }
    });
    sensorErrorListener = await DobsonNative.addListener("sensorError", ({ message }) => {
      sensorStatus.textContent = message || "Le capteur a signalé une erreur.";
    });
    try {
      sensorsStarted = true;
      await DobsonNative.startSensors({ intervalMs: 16 });
    } catch (error) {
      await stopSensors();
      autoFollow = false;
      localStorage.setItem("dobson-sky-control-mode-v3", "touch");
      updateMode();
      sensorStatus.textContent = error.message || "Impossible d’activer les capteurs.";
      throw error;
    }
  };

  const enableAutoFollow = async () => {
    autoFollow = true;
    autoFollowBaseline = undefined;
    skyTrackingActive = false;
    localStorage.setItem("dobson-sky-control-mode-v3", "auto");
    updateMode();
    try {
      await startSensors();
    } catch (error) {
      console.error("Impossible d’activer le pointage automatique.", error);
    }
  };

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

  scene.addEventListener("pointerdown", (event) => {
    if (event.target instanceof Element && event.target.closest("#dobson-sensor-mode, .scene-zoom-controls, .dobson-scene-alignment, .dobson-scene-time-panel, .dobson-solver-panel, .dobson-scene-tools, button, input, select")) return;
    if (autoFollow) gestureStarts.set(event.pointerId, { x: event.clientX, y: event.clientY });
  });
  scene.addEventListener("pointermove", (event) => {
    const start = gestureStarts.get(event.pointerId);
    if (autoFollow && start && Math.hypot(event.clientX - start.x, event.clientY - start.y) >= 8) {
      enterTouchNavigation();
    }
  });
  const clearGesture = (event) => gestureStarts.delete(event.pointerId);
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
        <legend>Catalogue céleste</legend>
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

  const skyContent = section.querySelector(".dobson-config-sky-content");
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

  const card = configForm.closest(".card");
  const advancedSection = [...card.children].find((element) =>
    element !== configForm
      && element.matches("details.settings-section")
      && element.querySelector(":scope > summary")?.textContent.trim().startsWith("Outils avancés"));
  if (advancedSection) technicalContent.appendChild(advancedSection);
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

      localStorage.setItem("dobson-mobile-location", JSON.stringify({ latitude, longitude }));
      try {
        const response = await fetch(`/cmd?${parameters}`);
        if (!response.ok) throw new Error("Impossible d’enregistrer la configuration.");
        const result = await response.json();
        if (result.status !== "OK") throw new Error("L’ESP32 n’a pas confirmé la configuration.");
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
        input.checked = value === undefined ? true : value === true || value === 1 || value === "1";
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
  const status = document.querySelector("#skyStatus");
  if (!toggle || !video || !scene) return;
  document.querySelector(".dobson-scene-menu-content")?.appendChild(toggle);

  let stream;
  const stop = () => {
    if (stream) {
      stream.getTracks().forEach((track) => track.stop());
      stream = undefined;
    }
    video.srcObject = null;
    scene.classList.remove("dobson-ar-active");
    toggle.setAttribute("aria-pressed", "false");
    toggle.setAttribute("aria-label", "Activer la caméra en réalité augmentée");
    toggle.textContent = "Caméra AR";
  };

  toggle.addEventListener("click", async () => {
    if (stream) {
      stop();
      return;
    }
    if (!navigator.mediaDevices?.getUserMedia) {
      status.textContent = "La caméra n’est pas disponible sur cet appareil.";
      return;
    }
    try {
      stream = await navigator.mediaDevices.getUserMedia({
        audio: false,
        video: { facingMode: { ideal: "environment" } }
      });
      video.srcObject = stream;
      await video.play();
      scene.classList.add("dobson-ar-active");
      toggle.setAttribute("aria-pressed", "true");
      toggle.setAttribute("aria-label", "Désactiver la caméra en réalité augmentée");
      toggle.textContent = "Arrêter AR";
      status.textContent = "Caméra AR active. La boussole et les capteurs se commandent séparément.";
    } catch (error) {
      stop();
      status.textContent = error.message || "Impossible d’accéder à la caméra.";
    }
  });

  document.addEventListener("visibilitychange", () => {
    if (document.hidden && stream) stop();
  });
}

function installPlateSolver() {
  if (!window.location.pathname.endsWith("/beta_sky.html")) return;
  const scene = document.querySelector("#scene");
  if (!scene) return;

  const section = document.createElement("details");
  section.className = "dobson-solver-panel";
  section.innerHTML = `
    <summary aria-label="Ouvrir la résolution astrométrique" title="Résolution astrométrique">
      <svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-sparkles"></use></svg>
      <span>Résolution d’image</span>
    </summary>
    <div class="dobson-solver-content">
      <p>Identifier le centre et l’échelle d’une image du ciel avec Astrometry.net. Une clé API et une image sont nécessaires.</p>
      <label for="dobson-astrometry-key">Clé API Astrometry.net</label>
      <input id="dobson-astrometry-key" type="password" autocomplete="off" placeholder="Saisir votre clé API">
      <div class="dobson-solver-actions">
        <button id="dobson-save-astrometry-key" type="button">Enregistrer la clé</button>
        <button id="dobson-clear-astrometry-key" type="button">Effacer la clé</button>
      </div>
      <label for="dobson-solver-image">Image du ciel</label>
      <input id="dobson-solver-image" type="file" accept="image/*" capture="environment">
      <button id="dobson-solve-image" type="button">Résoudre en ligne</button>
      <output id="dobson-solver-status" role="status" aria-live="polite">
        L’image ne sera envoyée à Astrometry.net qu’à votre demande et restera privée.
      </output>
    </div>`;
  document.querySelector(".dobson-scene-menu-content")?.appendChild(section);

  const apiKeyInput = section.querySelector("#dobson-astrometry-key");
  const imageInput = section.querySelector("#dobson-solver-image");
  const solveButton = section.querySelector("#dobson-solve-image");
  const solverStatus = section.querySelector("#dobson-solver-status");
  let savedApiKey = "";
  const savedKeyPromise = DobsonNative.getAstrometryApiKey().then(({ apiKey }) => {
    savedApiKey = apiKey;
    solverStatus.textContent = apiKey
      ? "Clé API enregistrée de manière sécurisée."
      : "Saisissez votre clé API Astrometry.net pour activer la résolution en ligne.";
  }).catch((error) => {
    solverStatus.textContent = error.message || "Impossible de lire la clé API enregistrée.";
    return "";
  });

  section.querySelector("#dobson-save-astrometry-key").addEventListener("click", async () => {
    const apiKey = apiKeyInput.value.trim();
    if (!apiKey) {
      solverStatus.textContent = "Saisissez une clé API avant de l’enregistrer.";
      return;
    }
    try {
      await DobsonNative.saveAstrometryApiKey({ apiKey });
      savedApiKey = apiKey;
      apiKeyInput.value = "";
      solverStatus.textContent = "Clé API enregistrée de manière sécurisée.";
    } catch (error) {
      solverStatus.textContent = error.message || "Impossible d’enregistrer la clé API.";
    }
  });

  section.querySelector("#dobson-clear-astrometry-key").addEventListener("click", async () => {
    try {
      await DobsonNative.clearAstrometryApiKey();
      savedApiKey = "";
      apiKeyInput.value = "";
      solverStatus.textContent = "Clé API effacée.";
    } catch (error) {
      solverStatus.textContent = error.message || "Impossible d’effacer la clé API.";
    }
  });

  solveButton.addEventListener("click", async () => {
    if (!imageInput.files?.[0]) {
      imageInput.click();
      return;
    }

    solveButton.disabled = true;
    try {
      await savedKeyPromise;
      if (!savedApiKey) throw new Error("Enregistrez d’abord votre clé API Astrometry.net.");
      const result = await solveWithAstrometry(imageInput.files[0], savedApiKey, (message) => {
        solverStatus.textContent = message;
      });
      solverStatus.textContent = result;
    } catch (error) {
      solverStatus.textContent = error.message || "La résolution astrométrique a échoué.";
    } finally {
      solveButton.disabled = false;
    }
  });

  imageInput.addEventListener("change", () => {
    if (imageInput.files?.[0]) solveButton.click();
  });
}
