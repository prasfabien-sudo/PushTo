const crypto = require("node:crypto");
const fs = require("node:fs");
const path = require("node:path");

const mobileRoot = path.resolve(__dirname, "..");
const repositoryRoot = path.resolve(mobileRoot, "..");

const pages = [
  { route: "/", file: "WebPage.h", symbol: "HTTP_PAGE", output: "index.html" },
  { route: "/station", file: "StationPage.h", symbol: "HTTP_STATION_PAGE", output: "station.html" },
  { route: "/calib_page", file: "CalibPage.h", symbol: "HTTP_CALIB_PAGE", output: "calib_page.html" },
  { route: "/config", file: "ConfigPage.h", symbol: "HTTP_CONFIG_PAGE", output: "config.html" },
  { route: "/releasenotes", file: "ReleaseNotes.h", symbol: "HTTP_RELEASE_NOTES_PAGE", output: "releasenotes.html" },
  { route: "/simu", file: "SimuPage.h", symbol: "HTTP_SIMU_PAGE", output: "simu.html" },
  { route: "/help", file: "HelpPage.h", symbol: "HTTP_HELP_PAGE", output: "help.html" },
  { route: "/beta_sky", file: "BetaSkyPage.h", symbol: "HTTP_BETA_SKY_PAGE", output: "beta_sky.html" }
];

function extractRawLiteral(file, symbol) {
  const source = fs.readFileSync(path.join(repositoryRoot, "include", file), "utf8").replace(/^\uFEFF/, "");
  const escapedSymbol = symbol.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
  const pattern = new RegExp(
    `\\bconst\\s+char\\s+${escapedSymbol}\\[\\]\\s+PROGMEM\\s*=\\s*R"rawliteral\\(([\\s\\S]*?)\\)rawliteral";`
  );
  const match = source.match(pattern);
  if (!match) {
    throw new Error(`Impossible d'extraire ${symbol} depuis include/${file}`);
  }
  return match[1];
}

function readCatalog() {
  const source = fs.readFileSync(path.join(repositoryRoot, "include", "ObjectsDB.h"), "utf8").replace(/^\uFEFF/, "");
  const entryPattern =
    /\{\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\}/g;
  const objects = Array.from(source.matchAll(entryPattern), (match) => ({
    name: match[1],
    commonName: match[2],
    type: match[3],
    ra: Number(match[4]),
    dec: Number(match[5]),
    mag: Number(match[6])
  }));
  if (objects.length === 0) {
    throw new Error("Le catalogue d'objets est vide ou son format a changé.");
  }
  return objects;
}

function readMobileCatalog() {
  const curated = readCatalog();
  const starsPath = path.join(mobileRoot, "data", "hyg-stars-v44.json");
  const stars = JSON.parse(fs.readFileSync(starsPath, "utf8"));
  const knownNames = new Set(
    curated.flatMap((object) => [object.name, object.commonName])
      .filter(Boolean)
      .map((name) => name.normalize("NFD").replace(/\p{Diacritic}/gu, "").toLowerCase())
  );
  return [
    ...curated,
    ...stars.filter((star) => {
      const name = star.name.normalize("NFD").replace(/\p{Diacritic}/gu, "").toLowerCase();
      if (knownNames.has(name)) return false;
      knownNames.add(name);
      return true;
    })
  ];
}

function replaceOnce(html, search, replacement, description) {
  const firstIndex = html.indexOf(search);
  if (firstIndex === -1 || html.indexOf(search, firstIndex + search.length) !== -1) {
    throw new Error(`Impossible d'appliquer la modification mobile : ${description}.`);
  }
  return html.replace(search, replacement);
}

function replaceSection(html, startMarker, endMarker, replacement, description) {
  const start = html.indexOf(startMarker);
  const end = html.indexOf(endMarker, start + startMarker.length);
  if (start === -1 || end === -1 || html.indexOf(startMarker, start + startMarker.length) !== -1) {
    throw new Error(`Impossible d'appliquer la modification mobile : ${description}.`);
  }
  return html.slice(0, start) + replacement + html.slice(end);
}

function makeMobileCatalogFilteringResponsive(html) {
  const optimizedFilter = `    function applyFilters() {
      const q = document.getElementById('searchInput').value.toLowerCase();
      const maxMag = parseFloat(document.getElementById('magFilterSelect').value);
      const cacheKey = Math.floor(Date.now() / 60000) + "|" + userLat + "|" + userLon;
      if (cacheKey !== mobileCatalogSkyCacheKey) {
        mobileCatalogSkyCache.clear();
        mobileCatalogSkyCacheKey = cacheKey;
      }

      let currDegAZ = (((currentAZ * dirAZ) / ticksAZ) * 360.0) % 360.0;
      let currDegALT = (((currentALT * dirALT) / ticksALT) * 360.0) % 360.0;
      if (currDegAZ < 0) currDegAZ += 360.0;
      if (currDegALT < 0) currDegALT += 360.0;

      const lst = getLST();
      const latRad = userLat * (Math.PI / 180.0);
      const sinLat = Math.sin(latRad);
      const cosLat = Math.cos(latRad);
      let filtered = fullCatalog.filter(o =>
        o.name.toLowerCase().includes(q) ||
        (o.commonName && o.commonName.toLowerCase().includes(q)) ||
        o.type.toLowerCase().includes(q)
      );
      if (!isNaN(maxMag)) filtered = filtered.filter(o => o.mag <= maxMag);

      if (selectedTypeFilter !== 'all') {
        filtered = filtered.filter(o => {
          const type = (o.type || "").toLowerCase();
          if (selectedTypeFilter === 'neb_planetaire') return type.includes('planétaire') || type.includes('planetaire') || type.includes('pn');
          if (selectedTypeFilter === 'planete') return (type === 'planète' || type === 'planete') && !type.includes('neb');
          if (selectedTypeFilter === 'galaxie') return type.includes('gal') || type.includes('gx');
          if (selectedTypeFilter === 'nebuleuse') return (type.includes('neb') || type.includes('dn')) && !type.includes('planét') && !type.includes('planet');
          if (selectedTypeFilter === 'amas') return type.includes('amas') || type.includes('oc') || type.includes('gc');
          if (selectedTypeFilter === 'etoile') return !type.includes('gal') && !type.includes('neb') && !type.includes('amas') && !type.includes('pla');
          return true;
        });
      }

      filtered = filtered.map(o => {
        let position = mobileCatalogSkyCache.get(o);
        if (!position) {
          const precessed = getPrecessedCoords(o.ra, o.dec);
          const decRad = precessed.dec * (Math.PI / 180.0);
          const hourAngle = (lst - precessed.ra) * 15.0 * (Math.PI / 180.0);
          const sinDec = Math.sin(decRad);
          const cosDec = Math.cos(decRad);
          const sinAlt = sinDec * sinLat + cosDec * cosLat * Math.cos(hourAngle);
          const trueAlt = Math.asin(Math.max(-1, Math.min(1, sinAlt)));
          const trueAltDeg = trueAlt * (180.0 / Math.PI);
          let altitude = trueAltDeg;
          if (trueAltDeg > -1.0) {
            const refrArcmin = 1.02 / Math.tan((trueAltDeg + 10.3 / (trueAltDeg + 5.11)) * (Math.PI / 180.0));
            altitude += refrArcmin / 60.0;
          }

          const cosAltitude = Math.cos(trueAlt);
          const azimuthCosine = cosAltitude === 0
            ? 1
            : (sinDec - sinLat * sinAlt) / (cosLat * cosAltitude);
          let azimuth = Math.acos(Math.max(-1, Math.min(1, azimuthCosine))) * (180.0 / Math.PI);
          if (Math.sin(hourAngle) > 0) azimuth = 360.0 - azimuth;

          let timeStr = "Visible";
          if (altitude < 10) {
            const hourAngleHours = (lst - precessed.ra + 24) % 24;
            const hoursToWait = (18.0 - hourAngleHours + 24) % 24;
            if (hoursToWait > 12) {
              timeStr = "Non visible cette nuit";
            } else {
              const hours = Math.floor(hoursToWait);
              const minutes = Math.round((hoursToWait - hours) * 60);
              timeStr = "Dans " + hours + "h" + (minutes < 10 ? "0" : "") + minutes;
            }
          }
          position = { raNow: precessed.ra, decNow: precessed.dec, altitude, azimuth, timeStr };
          mobileCatalogSkyCache.set(o, position);
        }

        let dAZ = position.azimuth - currDegAZ;
        while (dAZ > 180) dAZ -= 360;
        while (dAZ < -180) dAZ += 360;
        const dALT = position.altitude - currDegALT;
        return { ...o, ...position, dist: Math.sqrt(dAZ * dAZ + dALT * dALT) };
      });

      if (filterVisibleOnly) filtered = filtered.filter(o => o.altitude >= 10);
      filtered.sort((a, b) => a.dist - b.dist);
      renderCatalog(filtered);
    }

`;
  return replaceSection(
    html,
    "    function applyFilters() {",
    "    function renderCatalog(items) {",
    optimizedFilter,
    "calcul et filtrage du catalogue mobile"
  );
}

function makeMobileSkyRendererResponsive(html) {
  const replacements = [
    [
      '    let fieldOfView = 34;',
      `    let fieldOfView = window.__dobsonMobileApp ? 100 : 34;
    const mobileSkyViewStorageKey = 'dobson-mobile-sky-view-v1';
    let mobileSkyViewSaveTimer;
      let mobileSkyViewRestored = false;
      function saveMobileSkyView() {
        if (!window.__dobsonMobileApp || !mobileSkyViewRestored) return;
      try {
        localStorage.setItem(mobileSkyViewStorageKey, JSON.stringify({
          fieldOfView,
          touchHeading,
          touchElevation,
          alignOffsetHeading,
          alignOffsetElevation
        }));
      } catch (error) {
        console.error('Enregistrement de la vue du ciel impossible.', error);
      }
    }
    function scheduleMobileSkyViewSave() {
      if (!window.__dobsonMobileApp || !mobileSkyViewRestored) return;
      clearTimeout(mobileSkyViewSaveTimer);
      mobileSkyViewSaveTimer = setTimeout(saveMobileSkyView, 250);
    }
    function restoreMobileSkyView() {
      if (!window.__dobsonMobileApp) return;
      mobileSkyViewRestored = true;
      try {
        const saved = JSON.parse(localStorage.getItem(mobileSkyViewStorageKey) || 'null');
        if (!saved || typeof saved !== 'object') {
          fieldOfView = 100;
          return;
        }
        const values = ['fieldOfView', 'touchHeading', 'touchElevation', 'alignOffsetHeading', 'alignOffsetElevation'];
        if (!values.every(key => Number.isFinite(Number(saved[key])))) return;
        fieldOfView = Math.max(2, Math.min(120, Number(saved.fieldOfView)));
        touchHeading = ((Number(saved.touchHeading) % 360) + 360) % 360;
        touchElevation = Math.max(-360, Math.min(360, Number(saved.touchElevation)));
        alignOffsetHeading = ((Number(saved.alignOffsetHeading) % 360) + 360) % 360;
        alignOffsetElevation = Math.max(-90, Math.min(90, Number(saved.alignOffsetElevation)));
      } catch (error) {
        console.error('Restauration de la vue du ciel impossible.', error);
      }
    }
    window.addEventListener('pagehide', saveMobileSkyView);`,
      "restauration et sauvegarde persistante de la vue mobile"
    ],
    [
      '        await loadSavedSkyPrefs();\n        resizeCanvas();',
      '        await loadSavedSkyPrefs();\n        restoreMobileSkyView();\n        resizeCanvas();',
      "restauration du cadrage après les préférences"
    ],
    [
      "    const ctx2d = canvas.getContext('2d');",
      "    const ctx2d = window.createDobsonSkyContext(canvas);",
      "initialisation du rendu WebGL mobile avec repli Canvas"
    ],
    [
      "      viewReadout.textContent = `Offset AZ ${Math.round(currentHeading)}° · ALT ${Math.round(currentElevation)}°`;",
      "      ctx2d.flush?.();\n      viewReadout.textContent = `Offset AZ ${Math.round(currentHeading)}° · ALT ${Math.round(currentElevation)}°`;",
      "dessin des primitives WebGL"
    ],
    [
      '      const ratio = Math.min(window.devicePixelRatio || 1, 2);',
      '      const ratio = Math.min(window.devicePixelRatio || 1, window.__dobsonMobileApp ? 1.5 : 2);',
      "résolution de rendu adaptée aux écrans mobiles"
    ],
    [
      '        if (distance && pinchDistance) fieldOfView = Math.max(2, Math.min(75, fieldOfView * pinchDistance / distance));',
      '        if (distance && pinchDistance) {\n          fieldOfView = Math.max(2, Math.min(window.__dobsonMobileApp ? 120 : 75, fieldOfView * pinchDistance / distance));\n          scheduleMobileSkyViewSave();\n        }',
      "enregistrement du zoom au pincement"
    ],
    [
      '      fieldOfView = Math.max(2, Math.min(75, fieldOfView + Math.sign(event.deltaY) * 2));',
      '      fieldOfView = Math.max(2, Math.min(window.__dobsonMobileApp ? 120 : 75, fieldOfView + Math.sign(event.deltaY) * 2));\n      scheduleMobileSkyViewSave();',
      "enregistrement du zoom à la molette"
    ],
    [
      "      fieldOfView = Math.max(2, fieldOfView - 4);\n    });\n    document.getElementById('zoomOutBtn').addEventListener('click', () => {\n      fieldOfView = Math.min(75, fieldOfView + 4);\n    });",
      "      fieldOfView = Math.max(2, fieldOfView - 4);\n      scheduleMobileSkyViewSave();\n    });\n    document.getElementById('zoomOutBtn').addEventListener('click', () => {\n      fieldOfView = Math.min(window.__dobsonMobileApp ? 120 : 75, fieldOfView + 4);\n      scheduleMobileSkyViewSave();\n    });",
      "enregistrement du zoom par boutons"
    ],
    [
      "      const panPanFactor = (projectionMode !== 'actuel') ? (60 / fieldOfView) : (34 / fieldOfView);\n\n      applyTouchPan(event.clientX - dragPoint.x, event.clientY - dragPoint.y, panPanFactor);",
      `      const projectionReferenceFov = projectionMode !== 'actuel' ? 60 : 34;
      const panPanFactor = (projectionReferenceFov / fieldOfView) * (scene.clientWidth / 390);

      applyTouchPan(event.clientX - dragPoint.x, event.clientY - dragPoint.y, panPanFactor);
      scheduleMobileSkyViewSave();`,
      "enregistrement du déplacement au doigt"
    ],
    [
      "      alignOffsetElevation = 0;\n      if (projectionMode === 'actuel') fieldOfView = 34;\n      else if (projectionMode === 'stéréo') fieldOfView = 60;\n      else fieldOfView = 45;\n    });",
      "      alignOffsetElevation = 0;\n      if (projectionMode === 'actuel') fieldOfView = 34;\n      else if (projectionMode === 'stéréo') fieldOfView = 60;\n      else fieldOfView = 45;\n      scheduleMobileSkyViewSave();\n    });",
      "enregistrement du recentrage"
    ],
    [
      "      touchHeading = 0;\n      touchElevation = 0;\n    });",
      "      touchHeading = 0;\n      touchElevation = 0;\n      scheduleMobileSkyViewSave();\n    });",
      "enregistrement de la remise à zéro du déplacement"
    ],
    [
      "alignOffsetHeading = (alignOffsetHeading - step + 360) % 360;",
      "alignOffsetHeading = (alignOffsetHeading - step + 360) % 360;\n      scheduleMobileSkyViewSave();",
      "enregistrement de l'alignement horizontal"
    ],
    [
      "alignOffsetHeading = (alignOffsetHeading + step + 360) % 360;",
      "alignOffsetHeading = (alignOffsetHeading + step + 360) % 360;\n      scheduleMobileSkyViewSave();",
      "enregistrement de l'alignement horizontal"
    ],
    [
      "alignOffsetElevation = Math.max(-90, Math.min(90, alignOffsetElevation + step));",
      "alignOffsetElevation = Math.max(-90, Math.min(90, alignOffsetElevation + step));\n      scheduleMobileSkyViewSave();",
      "enregistrement de l'alignement vertical"
    ],
    [
      "      else fieldOfView = 34; // Actuel / Perspective\n    }",
      "      else fieldOfView = 34; // Actuel / Perspective\n      scheduleMobileSkyViewSave();\n    }",
      "enregistrement du mode de projection"
    ],
    [
      "alignOffsetElevation = Math.max(-90, Math.min(90, alignOffsetElevation - step));",
      "alignOffsetElevation = Math.max(-90, Math.min(90, alignOffsetElevation - step));\n      scheduleMobileSkyViewSave();",
      "enregistrement de l'alignement vertical"
    ],
    [
      `      const keyframes = [
        { alt: -18, top: '#020305', mid: '#05080f', bot: '#080c14' },
        { alt: -12, top: '#05080f', mid: '#0f1423', bot: '#1b1b3a' },
        { alt: -6,  top: '#090d16', mid: '#241b3b', bot: '#4a154b' },
        { alt: 0,   top: '#0f172a', mid: '#4a223f', bot: '#b23a1e' },
        { alt: 6,   top: '#1b365d', mid: '#2a6f97', bot: '#e07a5f' },
        { alt: 15,  top: '#0d47a1', mid: '#1565c0', bot: '#2979ff' }
      ];`,
      `      const keyframes = [
        { alt: -18, top: '#020711', mid: '#071426', bot: '#0b1c2b' },
        { alt: -12, top: '#061325', mid: '#0b1d32', bot: '#152a45' },
        { alt: -6,  top: '#0a1a2e', mid: '#17324b', bot: '#354054' },
        { alt: 0,   top: '#10243b', mid: '#274354', bot: '#76523c' },
        { alt: 6,   top: '#123459', mid: '#1d5670', bot: '#a8774e' },
        { alt: 15,  top: '#123b68', mid: '#1b5680', bot: '#287a91' }
      ];`,
      "palette mobile inspirée du bleu nuit, du turquoise et de l’or de l’icône"
    ],
    [
      '    function render() {\n      requestAnimationFrame(render);',
      '    const mobileEquatorialCache = new WeakMap();\n    const mobileHorizontalCache = new WeakMap();\n    let mobileHorizontalFrame = null;\n    let mobileLastLabelUpdate = 0;\n    function render() {\n      requestAnimationFrame(render);',
      "préparation du rendu mobile continu"
    ],
    [
      '    function horizontalPosition(object, siderealDegrees, latRadians) {\n      const dec = object.dec * Math.PI / 180;',
      `    function horizontalPosition(object, siderealDegrees, latRadians) {
      if (window.__dobsonMobileApp && mobileHorizontalFrame && Number.isFinite(object.ra) && Number.isFinite(object.dec)) {
        const cachedPosition = mobileHorizontalCache.get(object);
        if (cachedPosition && cachedPosition.frameKey === mobileHorizontalFrame.key) return cachedPosition.value;
        let equatorial = mobileEquatorialCache.get(object);
        if (!equatorial) {
          const ra = object.ra * Math.PI / 12;
          const dec = object.dec * Math.PI / 180;
          equatorial = { cosRa: Math.cos(ra), sinRa: Math.sin(ra), cosDec: Math.cos(dec), sinDec: Math.sin(dec) };
          mobileEquatorialCache.set(object, equatorial);
        }
        const cosHourAngle = mobileHorizontalFrame.cosSidereal * equatorial.cosRa
          + mobileHorizontalFrame.sinSidereal * equatorial.sinRa;
        const sinHourAngle = mobileHorizontalFrame.sinSidereal * equatorial.cosRa
          - mobileHorizontalFrame.cosSidereal * equatorial.sinRa;
        const east = -equatorial.cosDec * sinHourAngle;
        const north = equatorial.sinDec * mobileHorizontalFrame.cosLatitude
          - equatorial.cosDec * mobileHorizontalFrame.sinLatitude * cosHourAngle;
        const sinAltitude = equatorial.sinDec * mobileHorizontalFrame.sinLatitude
          + equatorial.cosDec * mobileHorizontalFrame.cosLatitude * cosHourAngle;
        const altitudeRad = Math.asin(Math.max(-1, Math.min(1, sinAltitude)));
        const azimuthRad = Math.atan2(east, north);
        const result = {
          altitude: altitudeRad * 180 / Math.PI,
          azimuth: (azimuthRad * 180 / Math.PI + 360) % 360,
          position: [east, sinAltitude, north],
          raw: { alt: altitudeRad, az: azimuthRad }
        };
        mobileHorizontalCache.set(object, { frameKey: mobileHorizontalFrame.key, value: result });
        return result;
      }

      const dec = object.dec * Math.PI / 180;`,
      "projection horizontale pré-calculée"
    ],
    [
      `      const latRadians = latitude * Math.PI / 180;\n\n      // V5.73.0 Calcul d'un objet de référence central pour les projections mathématiques`,
      `      const latRadians = latitude * Math.PI / 180;
      if (window.__dobsonMobileApp) {
        const siderealRadians = sidereal * Math.PI / 180;
        mobileHorizontalFrame = {
          key: Math.round(sidereal * 1000),
          cosSidereal: Math.cos(siderealRadians),
          sinSidereal: Math.sin(siderealRadians),
          cosLatitude: Math.cos(latRadians),
          sinLatitude: Math.sin(latRadians)
        };
      }

      // V5.73.0 Calcul d'un objet de référence central pour les projections mathématiques`,
      "mise à jour de la transformation céleste mobile"
    ],
    [
      '      lastScreenObjects = [];\n\n      // Objets célestes',
      `      const mobileRenderFilters = window.__dobsonMobileApp ? {
        maxMagnitude: getSelectedMagnitude(),
        showGalaxies: document.getElementById('showGal').checked,
        showNebulae: document.getElementById('showNeb').checked,
        showClusters: document.getElementById('showAmas').checked,
        showStars: document.getElementById('showStars').checked
      } : null;
      lastScreenObjects = [];

      // Objets célestes`,
      "mise en cache des filtres du rendu mobile"
    ],
    [
      '        if (!matchesFilter(object)) continue;',
      `        if (window.__dobsonMobileApp) {
          const { isGal, isNeb, isAmas, isStar } = getObjectType(object);
          if (object.mag > mobileRenderFilters.maxMagnitude
            || !(isGal && mobileRenderFilters.showGalaxies)
              && !(isNeb && mobileRenderFilters.showNebulae)
              && !(isAmas && mobileRenderFilters.showClusters)
              && !(isStar && mobileRenderFilters.showStars)) continue;
        } else if (!matchesFilter(object)) continue;`,
      "suppression des recherches DOM par étoile"
    ],
    [
      '      viewReadout.textContent = \`Offset AZ \${Math.round(currentHeading)}° · ALT \${Math.round(currentElevation)}°\`;\n\n      // Éléments réutilisés entre les frames pour éviter le clignotement\n      const labelPool = labelsElement.labelPool || (labelsElement.labelPool = []);\n      let labelUsed = 0;',
      `      viewReadout.textContent = \`Offset AZ \${Math.round(currentHeading)}° · ALT \${Math.round(currentElevation)}°\`;
      if (window.__dobsonMobileApp) {
        const now = performance.now();
        if (now - mobileLastLabelUpdate < 80) return;
        mobileLastLabelUpdate = now;
      }

      // Éléments réutilisés entre les frames pour éviter le clignotement
      const labelPool = labelsElement.labelPool || (labelsElement.labelPool = []);
      let labelUsed = 0;`,
      "découplage des étiquettes du rendu fluide"
    ],
    [
      '      const placedLabels = [];',
      '      const placedLabelGrid = new Map();\n      let mobileLabelCount = 0;',
      "index spatial des étiquettes du ciel"
    ],
    [
      '        let leftPct = (item.screenX + 1) * 50;\n        let topPct = (1 - item.screenY) * 50;\n\n        // V5.73.0 Seuil de collision dynamique selon FOV et mode\n        let collisionPanFactor = (projectionMode !== \'actuel\') ? (60 / fieldOfView) : (34 / fieldOfView);\n        const thresholdX = Math.max(1.0, 6.5 / collisionPanFactor);\n        const thresholdY = Math.max(0.8, 4.0 / collisionPanFactor);\n        const overlap = placedLabels.some(p => Math.abs(p.left - leftPct) < thresholdX && Math.abs(p.top - topPct) < thresholdY);\n        if (overlap) continue;\n\n        placedLabels.push({ left: leftPct, top: topPct });',
      '        if (window.__dobsonMobileApp && mobileLabelCount >= 250) continue;\n        const leftPct = (item.screenX + 1) * 50;\n        const topPct = (1 - item.screenY) * 50;\n\n        // V5.73.0 Seuil de collision dynamique selon FOV et mode\n        const collisionPanFactor = (projectionMode !== \'actuel\') ? (60 / fieldOfView) : (34 / fieldOfView);\n        const thresholdX = Math.max(1.0, 6.5 / collisionPanFactor);\n        const thresholdY = Math.max(0.8, 4.0 / collisionPanFactor);\n        const cellX = Math.floor(leftPct / thresholdX);\n        const cellY = Math.floor(topPct / thresholdY);\n        let overlap = false;\n        for (let x = cellX - 1; x <= cellX + 1 && !overlap; x++) {\n          for (let y = cellY - 1; y <= cellY + 1 && !overlap; y++) {\n            const cell = placedLabelGrid.get(x + \':\' + y);\n            overlap = Boolean(cell && cell.some(p => Math.abs(p.left - leftPct) < thresholdX && Math.abs(p.top - topPct) < thresholdY));\n          }\n        }\n        if (overlap) continue;\n        const cellKey = cellX + \':\' + cellY;\n        if (!placedLabelGrid.has(cellKey)) placedLabelGrid.set(cellKey, []);\n        placedLabelGrid.get(cellKey).push({ left: leftPct, top: topPct });',
      "optimisation des collisions d'étiquettes"
    ],
    [
      '        else label.textContent = item.object.name ? item.object.name : item.object.commonName;\n        \n      }\n      for (let i = labelUsed; i < labelPool.length; i++) labelPool[i].style.display = \'none\';',
      '        else label.textContent = item.object.name ? item.object.name : item.object.commonName;\n        mobileLabelCount++;\n        \n      }\n      for (let i = labelUsed; i < labelPool.length; i++) labelPool[i].style.display = \'none\';',
      "limitation des étiquettes mobiles"
    ]
  ];

  for (const [search, replacement, description] of replacements) {
    html = replaceOnce(html, search, replacement, description);
  }
  return html;
}

function makeMobileCatalogResponsive(html) {
  const replacements = [
    [
      '        </table>\n      </div>\n    </div>\n  </div>\n\n    <nav class="bottom-nav"',
      '        </table>\n        <div id="dobson-catalog-more" class="dobson-catalog-more" hidden>\n          <output id="dobson-catalog-count" role="status" aria-live="polite"></output>\n          <button id="dobson-catalog-more-button" type="button">Afficher plus d’objets</button>\n        </div>\n      </div>\n    </div>\n  </div>\n\n    <nav class="bottom-nav"',
      "contrôles de pagination du catalogue"
    ],
    [
      '    const catalogRows = new Map();',
      '    const catalogRows = new Map();\n    let mobileCatalogLimit = 100;\n    let mobileCatalogItems = [];\n    let lastCatalogRefresh = 0;',
      "état de pagination du catalogue"
    ],
    [
      '    let mobileCatalogItems = [];\n    let lastCatalogRefresh = 0;',
      '    let mobileCatalogItems = [];\n    let lastCatalogRefresh = 0;\n    const mobileCatalogSkyCache = new Map();\n    let mobileCatalogSkyCacheKey = "";',
      "cache des positions célestes mobiles"
    ],
    [
      '        updateGuidance(degAZ, degALT);\n        applyFilters();',
      '        updateGuidance(degAZ, degALT);\n        if (!window.__dobsonMobileApp || Date.now() - lastCatalogRefresh >= 1000) {\n          lastCatalogRefresh = Date.now();\n          applyFilters();\n        }',
      "fréquence de recalcul des filtres"
    ],
    [
      "          applyFilters();\n        });\n    }\n\n    function toggleTimeFilter()",
      "          applyFilters();\n          if (window.__dobsonMobileApp) lastCatalogRefresh = Date.now();\n        });\n    }\n\n    function toggleTimeFilter()",
      "horodatage du calcul initial"
    ],
    [
      "    function renderCatalog(items) {\n      const tbody = document.getElementById('catalogBody');",
      "    function renderCatalog(items) {\n      mobileCatalogItems = items;\n      const renderItems = window.__dobsonMobileApp ? items.slice(0, mobileCatalogLimit) : items;\n      const moreControl = document.getElementById('dobson-catalog-more');\n      const moreCount = document.getElementById('dobson-catalog-count');\n      const moreButton = document.getElementById('dobson-catalog-more-button');\n      const tbody = document.getElementById('catalogBody');\n      if (moreControl) moreControl.hidden = items.length <= renderItems.length;\n      if (moreCount) moreCount.textContent = `${renderItems.length} objets affichés sur ${items.length}`;",
      "limitation du rendu initial"
    ],
    [
      "      if (items.length === 0) {\n        catalogRows.forEach(row => row.remove());",
      "      if (items.length === 0) {\n        if (moreControl) moreControl.hidden = true;\n        catalogRows.forEach(row => row.remove());",
      "état du catalogue vide"
    ],
    [
      "      items.forEach((o, index) => {",
      "      renderItems.forEach((o, index) => {",
      "rendu du catalogue paginé"
    ],
    [
      "      catalogRows.forEach((row, key) => {\n        if (!visibleKeys.has(key)) row.remove();\n      });\n    }",
      "      catalogRows.forEach((row, key) => {\n        if (!visibleKeys.has(key)) row.remove();\n      });\n      if (moreButton) moreButton.hidden = items.length <= renderItems.length;\n    }\n\n    document.getElementById('dobson-catalog-more-button').addEventListener('click', () => {\n      mobileCatalogLimit += 100;\n      renderCatalog(mobileCatalogItems);\n    });",
      "commande pour afficher plus d'objets"
    ]
  ];

  for (const [search, replacement, description] of replacements) {
    html = replaceOnce(html, search, replacement, description);
  }
  return makeMobileCatalogFilteringResponsive(html);
}

function makeMobileHtml(html, output) {
  html = html.replace(/\r\n/g, "\n");
  for (const page of pages) {
    const target = page.output;
    const escapedRoute = page.route.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
    const routePattern = new RegExp(`(href\\s*=\\s*["'])${escapedRoute}(["'])`, "g");
    html = html.replace(routePattern, `$1/${target}$2`);
  }

  const bootstrap = '<script>window.__dobsonMobileApp = true;</script>\n  <script src="/mobile-bootstrap.js"></script>\n  <script type="module" src="/mobile.js"></script>';
  if (output === "index.html") {
    html = makeMobileCatalogResponsive(html);
  }
  if (html.includes('id="skyCanvas"')) {
    html = makeMobileSkyRendererResponsive(html);
    html = html.replace('</head>', '  <script src="/sky-webgl.js"></script>\n</head>');
    html = html.replace(
      '<div class="scene-zoom-controls">',
      '<div class="scene-zoom-controls"><button id="dobson-ar-toggle" type="button" class="zoom-btn" title="Caméra en réalité augmentée et pointage automatique" aria-label="Activer la caméra en réalité augmentée" aria-pressed="false">Caméra AR</button>'
    );
    html = html.replace(
      '<canvas id="skyCanvas"',
      '<video id="dobson-ar-video" aria-hidden="true" autoplay muted playsinline></video>\n      <canvas id="skyCanvas"'
    );
    html = html.replace(
      '</head>',
      '<style>#scene.dobson-ar-active #dobson-ar-video{display:block;position:absolute;inset:0;width:100%;height:100%;object-fit:cover;z-index:0}#dobson-ar-video{display:none}#scene.dobson-ar-active #skyCanvas{background:transparent}</style>\n</head>'
    );
  }
  if (!/<head(?:\s[^>]*)?>/i.test(html)) {
    throw new Error("La page HTML extraite ne contient pas de balise <head>.");
  }
  return html.replace(/<\/head>/i, `  ${bootstrap}\n</head>`);
}

function prepare() {
  const pagesDirectory = path.join(mobileRoot, "src");
  const publicDirectory = path.join(mobileRoot, "public");
  fs.mkdirSync(pagesDirectory, { recursive: true });
  fs.mkdirSync(publicDirectory, { recursive: true });

  for (const page of pages) {
    const html = makeMobileHtml(extractRawLiteral(page.file, page.symbol), page.output);
    fs.writeFileSync(path.join(pagesDirectory, page.output), html);
  }

  const icons = extractRawLiteral("Icons.h", "HTTP_ICONS");
  fs.writeFileSync(path.join(publicDirectory, "icons.svg"), icons);

  const objects = readMobileCatalog();
  const revision = crypto.createHash("sha256").update(JSON.stringify(objects)).digest("hex");
  fs.writeFileSync(
    path.join(publicDirectory, "mobile-catalog.json"),
    JSON.stringify({ revision, catalog: objects })
  );
  return { pageCount: pages.length, objectCount: objects.length };
}

if (require.main === module) {
  const result = prepare();
  console.log(`Pages extraites : ${result.pageCount}; objets du catalogue : ${result.objectCount}.`);
}

module.exports = { makeMobileHtml, pages, prepare, readCatalog, readMobileCatalog };
