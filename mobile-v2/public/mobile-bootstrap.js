(() => {
  const isMobileStartupPath = window.location.pathname === "/" || window.location.pathname === "/index.html";
  if (window.__dobsonMobileApp && isMobileStartupPath
    && sessionStorage.getItem("dobson-mobile-startup-routed") !== "true") {
    sessionStorage.setItem("dobson-mobile-startup-routed", "true");
    window.location.replace("/beta_sky.html");
  }

  const originalFetch = window.fetch.bind(window);
  let resolveCatalog;
  let rejectCatalog;
  let resolveNativePlugin;
  window.__dobsonNativeCatalogPromise = new Promise((resolve, reject) => {
    resolveCatalog = resolve;
    rejectCatalog = reject;
  });
  window.__dobsonNativePluginPromise = new Promise((resolve) => {
    resolveNativePlugin = resolve;
  });
  window.__resolveDobsonCatalog = resolveCatalog;
  window.__rejectDobsonCatalog = rejectCatalog;
  window.__resolveDobsonNativePlugin = resolveNativePlugin;

  const jsonResponse = (value, status = 200) => new Response(JSON.stringify(value), {
    status,
    headers: { "Content-Type": "application/json" }
  });
  const readLocalJson = (key, fallback, description) => {
    try {
      return JSON.parse(localStorage.getItem(key) || JSON.stringify(fallback));
    } catch (error) {
      console.error(`Lecture locale impossible (${description}).`, error);
      throw error;
    }
  };
  const getDeviceConfiguration = () => readLocalJson("dobson-mobile-device-config", {}, "configuration");
  const getRawEncoderStatus = async (configuration) => {
    if (window.__dobsonEspConnected) {
      const nativePlugin = await window.__dobsonNativePluginPromise;
      return nativePlugin.getEncoderStatus();
    }
    const orientation = {
      azimuth: 0,
      altitude: 0,
      pitch: 90,
      ...(window.__dobsonPhoneOrientation || {})
    };
    const measuredAltitude = Number(orientation.altitude);
    const altitude = Number.isFinite(measuredAltitude)
      ? Math.max(-90, Math.min(90, measuredAltitude))
      : Math.max(-90, Math.min(90, 90 - Math.abs(Number(orientation.pitch) || 0)));
    const ticksAZ = Number(configuration.ticksAZ);
    const ticksALT = Number(configuration.ticksALT);
    const resolutionAZ = Number.isFinite(ticksAZ) && ticksAZ > 0 ? ticksAZ : 360;
    const resolutionALT = Number.isFinite(ticksALT) && ticksALT > 0 ? ticksALT : 360;
    const azimuth = (((Number(orientation.azimuth) || 0) % 360) + 360) % 360;
    return {
      rawAZ: azimuth / 360 * resolutionAZ,
      rawALT: (altitude < 0 ? altitude + 360 : altitude) / 360 * resolutionALT,
      ticksAZ: resolutionAZ,
      ticksALT: resolutionALT
    };
  };
  const getMobileStatus = async () => {
    const configuration = getDeviceConfiguration();
    const location = readLocalJson("dobson-mobile-location", { latitude: 45.89, longitude: 6.05 }, "position");
    const encoders = await getRawEncoderStatus(configuration);
    const rawAZ = Number(encoders.rawAZ) + Number(configuration.offsetAZ || 0);
    const rawALT = Number(encoders.rawALT) + Number(configuration.offsetALT || 0);
    const ticksAZ = Number(configuration.ticksAZ) > 0 ? Number(configuration.ticksAZ) : Number(encoders.ticksAZ);
    const ticksALT = Number(configuration.ticksALT) > 0 ? Number(configuration.ticksALT) : Number(encoders.ticksALT);
    const latitude = Number(configuration.lat ?? location.latitude);
    const longitude = Number(configuration.lon ?? location.longitude);
    return {
      rawAZ,
      rawALT,
      ticksAZ,
      ticksALT,
      dirAZ: configuration.revAZ === true ? -1 : 1,
      dirALT: configuration.revALT === true ? -1 : 1,
      pinAzA: 18,
      pinAzB: 19,
      pinAltA: 21,
      pinAltB: 22,
      revAZ: configuration.revAZ === true,
      revALT: configuration.revALT === true,
      lat: Number.isFinite(latitude) && Math.abs(latitude) <= 90 ? latitude : 45.89,
      lon: Number.isFinite(longitude) && Math.abs(longitude) <= 180 ? longitude : 6.05,
      siteName: configuration.siteName || "Position du téléphone"
    };
  };
  const saveConfiguration = (configuration) => {
    localStorage.setItem("dobson-mobile-device-config", JSON.stringify(configuration));
  };
  const getCurrentRawPosition = async (configuration) => {
    const encoders = await getRawEncoderStatus(configuration);
    return {
      azimuth: Number(encoders.rawAZ),
      altitude: Number(encoders.rawALT),
      ticksAZ: Number(configuration.ticksAZ) > 0 ? Number(configuration.ticksAZ) : Number(encoders.ticksAZ),
      ticksALT: Number(configuration.ticksALT) > 0 ? Number(configuration.ticksALT) : Number(encoders.ticksALT)
    };
  };
  const handleMobileCommand = async (url) => {
    const action = url.searchParams.get("action");
    let configuration = getDeviceConfiguration();
    if (action === "save_sites") {
      const sites = url.searchParams.get("json");
      try {
        const parsedSites = JSON.parse(sites);
        if (!parsedSites || typeof parsedSites !== "object" || Array.isArray(parsedSites)) {
          throw new Error("Liste de lieux invalide.");
        }
        localStorage.setItem("dobson-mobile-sites", JSON.stringify(parsedSites));
        return jsonResponse({ status: "OK", local: true });
      } catch (error) {
        return jsonResponse({ status: "ERROR", message: error.message }, 400);
      }
    }

    if (action === "set_config" || action === "set_location") {
      const latitudeValue = url.searchParams.get("lat");
      const longitudeValue = url.searchParams.get("lon");
      const latitude = Number(latitudeValue);
      const longitude = Number(longitudeValue);
      if (latitudeValue === null || longitudeValue === null
        || !Number.isFinite(latitude) || Math.abs(latitude) > 90
        || !Number.isFinite(longitude) || Math.abs(longitude) > 180) {
        return jsonResponse({ status: "ERROR", message: "Coordonnées invalides." }, 400);
      }
      if (action === "set_config") {
        const ticksAZ = Number(url.searchParams.get("ticksAZ"));
        const ticksALT = Number(url.searchParams.get("ticksALT"));
        if (!Number.isFinite(ticksAZ) || ticksAZ <= 0 || !Number.isFinite(ticksALT) || ticksALT <= 0) {
          return jsonResponse({ status: "ERROR", message: "Résolution des encodeurs invalide." }, 400);
        }
        const revAZ = url.searchParams.get("revAZ") === "1";
        const revALT = url.searchParams.get("revALT") === "1";
        if (revAZ !== (configuration.revAZ === true) || revALT !== (configuration.revALT === true)
          || ticksAZ !== Number(configuration.ticksAZ) || ticksALT !== Number(configuration.ticksALT)) {
          configuration.offsetAZ = 0;
          configuration.offsetALT = 0;
          delete configuration.star1;
        }
        configuration = {
          ...configuration,
          ticksAZ,
          ticksALT,
          revAZ,
          revALT,
          lat: latitude,
          lon: longitude,
          siteName: url.searchParams.get("siteName") || "Lieu personnalisé"
        };
        saveConfiguration(configuration);
      } else {
        configuration = {
          ...configuration,
          lat: latitude,
          lon: longitude,
          siteName: url.searchParams.get("siteName") || "Lieu personnalisé"
        };
        saveConfiguration(configuration);
      }
      localStorage.setItem("dobson-mobile-location", JSON.stringify({ latitude, longitude }));
      return jsonResponse({ status: "OK", local: true });
    }

    if (action === "set_zero_alt" || action === "set_zero_az" || action === "align") {
      const current = await getCurrentRawPosition(configuration);
      if (action !== "set_zero_alt") configuration.offsetAZ = -current.azimuth;
      if (action !== "set_zero_az") configuration.offsetALT = -current.altitude;
      saveConfiguration(configuration);
      return jsonResponse({ status: "OK", local: true });
    }

    if (action === "set_polaris") {
      const current = await getCurrentRawPosition(configuration);
      const latitude = Number(configuration.lat ?? 45.89);
      configuration.offsetAZ = -current.azimuth;
      configuration.offsetALT = latitude / 360 * current.ticksALT * (configuration.revALT ? -1 : 1)
        - current.altitude;
      saveConfiguration(configuration);
      return jsonResponse({ status: "OK", local: true });
    }

    if (action === "align_star") {
      const azimuth = Number(url.searchParams.get("az"));
      const altitude = Number(url.searchParams.get("alt"));
      if (!Number.isFinite(azimuth) || azimuth < 0 || azimuth >= 360
        || !Number.isFinite(altitude) || altitude < 0 || altitude > 90) {
        return jsonResponse({ status: "ERROR", message: "Coordonnées de l'étoile invalides." }, 400);
      }
      const current = await getCurrentRawPosition(configuration);
      configuration.offsetAZ = azimuth / 360 * current.ticksAZ * (configuration.revAZ ? -1 : 1) - current.azimuth;
      configuration.offsetALT = altitude / 360 * current.ticksALT * (configuration.revALT ? -1 : 1) - current.altitude;
      saveConfiguration(configuration);
      return jsonResponse({ status: "OK", local: true });
    }

    if (action === "align_star1") {
      const azimuth = Number(url.searchParams.get("az"));
      const altitude = Number(url.searchParams.get("alt"));
      if (!Number.isFinite(azimuth) || azimuth < 0 || azimuth >= 360
        || !Number.isFinite(altitude) || altitude < 0 || altitude > 90) {
        return jsonResponse({ status: "ERROR", message: "Coordonnées de l'étoile 1 invalides." }, 400);
      }
      const current = await getCurrentRawPosition(configuration);
      configuration.star1 = {
        azimuth,
        altitude,
        azimuthCount: current.azimuth,
        altitudeCount: current.altitude
      };
      saveConfiguration(configuration);
      return jsonResponse({ status: "OK", step: 1, local: true });
    }

    if (action === "align_star2") {
      const azimuth = Number(url.searchParams.get("az"));
      const altitude = Number(url.searchParams.get("alt"));
      const first = configuration.star1;
      if (!first || !Number.isFinite(azimuth) || azimuth < 0 || azimuth >= 360
        || !Number.isFinite(altitude) || altitude < 0 || altitude > 90) {
        return jsonResponse({ status: "ERROR", message: "Coordonnées ou première étoile invalides." }, 400);
      }
      let deltaAZ = azimuth - first.azimuth;
      if (deltaAZ > 180) deltaAZ -= 360;
      if (deltaAZ < -180) deltaAZ += 360;
      const deltaALT = altitude - first.altitude;
      const current = await getCurrentRawPosition(configuration);
      const deltaCountAZ = current.azimuth - first.azimuthCount;
      const deltaCountALT = current.altitude - first.altitudeCount;
      const ticksAZ = Math.abs(deltaCountAZ * 360 / deltaAZ);
      const ticksALT = Math.abs(deltaCountALT * 360 / deltaALT);
      if (Math.abs(deltaAZ) < 5 || Math.abs(deltaALT) < 5 || deltaCountAZ === 0 || deltaCountALT === 0
        || deltaCountAZ * (configuration.revAZ ? -1 : 1) * deltaAZ <= 0
        || deltaCountALT * (configuration.revALT ? -1 : 1) * deltaALT <= 0
        || !Number.isFinite(ticksAZ) || ticksAZ < 100 || ticksAZ > 10000000
        || !Number.isFinite(ticksALT) || ticksALT < 100 || ticksALT > 10000000) {
        return jsonResponse({ status: "ERROR", message: "Écart insuffisant ou calibration incohérente." }, 400);
      }
      configuration.ticksAZ = ticksAZ;
      configuration.ticksALT = ticksALT;
      configuration.offsetAZ = azimuth / 360 * ticksAZ * (configuration.revAZ ? -1 : 1) - current.azimuth;
      configuration.offsetALT = altitude / 360 * ticksALT * (configuration.revALT ? -1 : 1) - current.altitude;
      delete configuration.star1;
      saveConfiguration(configuration);
      return jsonResponse({ status: "OK", step: 2, ticksAZ, ticksALT, local: true });
    }

    return jsonResponse({
      status: "ERROR",
      message: "Cette commande est indisponible : l’ESP32 ne fournit que les positions des encodeurs."
    }, 501);
  };

  window.fetch = (input, init) => {
    const rawUrl = input instanceof Request ? input.url : input;
    const url = new URL(rawUrl, window.location.href);

    if (window.__dobsonMobileApp && url.origin === window.location.origin) {
      if (url.pathname === "/get_sky_prefs") {
        return Promise.resolve(new Response(localStorage.getItem("dobson-mobile-sky-prefs") || "{}", {
          status: 200,
          headers: { "Content-Type": "application/json" }
        }));
      }

      if (url.pathname === "/save_sky_prefs") {
        const preferences = Object.fromEntries(
          [...url.searchParams.entries()].map(([key, value]) => {
            const number = Number(value);
            return [key, value !== "" && Number.isFinite(number) ? number : value];
          })
        );
        localStorage.setItem("dobson-mobile-sky-prefs", JSON.stringify(preferences));
        return Promise.resolve(new Response(null, { status: 204 }));
      }

      if (url.pathname === "/get_sites") {
        return Promise.resolve(new Response(localStorage.getItem("dobson-mobile-sites") || "{}", {
          status: 200,
          headers: { "Content-Type": "application/json" }
        }));
      }

      if (url.pathname === "/cmd") return handleMobileCommand(url);

      if (url.pathname === "/status") {
        return getMobileStatus().then((status) => jsonResponse(status));
      }

      if (url.pathname === "/calib" || url.pathname === "/simstep") {
        return Promise.resolve(jsonResponse({
          status: "ERROR",
          message: "Cette fonction n’est plus disponible sur l’ESP32."
        }, 501));
      }
    }

    if (url.origin === window.location.origin && url.pathname === "/catalog") {
      return window.__dobsonNativeCatalogPromise.then((objects) => {
        if (objects === null) {
          return originalFetch(input, init);
        }
        return new Response(JSON.stringify(objects), {
          status: 200,
          headers: { "Content-Type": "application/json" }
        });
      });
    }

    return originalFetch(input, init);
  };
})();
