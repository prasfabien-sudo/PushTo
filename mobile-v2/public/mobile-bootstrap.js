(() => {
  const isMobileStartupPath = window.location.pathname === "/" || window.location.pathname === "/index.html";
  if (window.__dobsonMobileApp && isMobileStartupPath
    && sessionStorage.getItem("dobson-mobile-startup-routed") !== "true") {
    sessionStorage.setItem("dobson-mobile-startup-routed", "true");
    window.location.replace("/beta_sky.html");
  }

  const originalFetch = window.fetch.bind(window);
  const apiRoutes = new Set([
    "/status",
    "/cmd",
    "/calib",
    "/simstep",
    "/get_sites",
    "/get_sky_prefs",
    "/save_sky_prefs"
  ]);
  let resolveCatalog;
  let rejectCatalog;
  window.__dobsonNativeCatalogPromise = new Promise((resolve, reject) => {
    resolveCatalog = resolve;
    rejectCatalog = reject;
  });
  window.__resolveDobsonCatalog = resolveCatalog;
  window.__rejectDobsonCatalog = rejectCatalog;

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

      if (url.pathname === "/get_sites" && !window.__dobsonEspConnected) {
        return Promise.resolve(new Response(localStorage.getItem("dobson-mobile-sites") || "{}", {
          status: 200,
          headers: { "Content-Type": "application/json" }
        }));
      }

      if (url.pathname === "/cmd" && !window.__dobsonEspConnected) {
        const action = url.searchParams.get("action");
        if (action === "save_sites") {
          const sites = url.searchParams.get("json");
          try {
            const parsedSites = JSON.parse(sites);
            if (!parsedSites || typeof parsedSites !== "object" || Array.isArray(parsedSites)) {
              throw new Error("Liste de lieux invalide.");
            }
            localStorage.setItem("dobson-mobile-sites", JSON.stringify(parsedSites));
            return Promise.resolve(new Response(JSON.stringify({ status: "OK", local: true }), {
              status: 200,
              headers: { "Content-Type": "application/json" }
            }));
          } catch (error) {
            return Promise.resolve(new Response(JSON.stringify({ status: "ERROR", message: error.message }), {
              status: 400,
              headers: { "Content-Type": "application/json" }
            }));
          }
        }

        if (action === "set_config") {
          const latitudeValue = url.searchParams.get("lat");
          const longitudeValue = url.searchParams.get("lon");
          const ticksAZValue = url.searchParams.get("ticksAZ");
          const ticksALTValue = url.searchParams.get("ticksALT");
          const latitude = Number(latitudeValue);
          const longitude = Number(longitudeValue);
          const ticksAZ = Number(ticksAZValue);
          const ticksALT = Number(ticksALTValue);
          if (!latitudeValue || !longitudeValue || !ticksAZValue || !ticksALTValue
            || !Number.isFinite(latitude) || Math.abs(latitude) > 90
            || !Number.isFinite(longitude) || Math.abs(longitude) > 180
            || !Number.isFinite(ticksAZ) || ticksAZ <= 0
            || !Number.isFinite(ticksALT) || ticksALT <= 0) {
            return Promise.resolve(new Response(JSON.stringify({ status: "ERROR", message: "Configuration invalide." }), {
              status: 400,
              headers: { "Content-Type": "application/json" }
            }));
          }
          const configuration = {
            ticksAZ,
            ticksALT,
            revAZ: url.searchParams.get("revAZ") === "1",
            revALT: url.searchParams.get("revALT") === "1",
            lat: latitude,
            lon: longitude,
            siteName: url.searchParams.get("siteName") || "Lieu personnalisé"
          };
          localStorage.setItem("dobson-mobile-device-config", JSON.stringify(configuration));
          localStorage.setItem("dobson-mobile-location", JSON.stringify({ latitude, longitude }));
          return Promise.resolve(new Response(JSON.stringify({ status: "OK", local: true }), {
            status: 200,
            headers: { "Content-Type": "application/json" }
          }));
        }
      }

      if (url.pathname === "/status" && !window.__dobsonEspConnected) {
        let location = { latitude: 45.89, longitude: 6.05 };
        let orientation = { azimuth: 0, altitude: 0, pitch: 90 };
        let configuration = {};
        try {
          location = { ...location, ...JSON.parse(localStorage.getItem("dobson-mobile-location") || "{}") };
          orientation = { ...orientation, ...(window.__dobsonPhoneOrientation || {}) };
          configuration = JSON.parse(localStorage.getItem("dobson-mobile-device-config") || "{}");
        } catch (error) {
          console.warn("Impossible de lire la position locale du planétarium.", error);
        }
        const ticksAZ = Number(configuration.ticksAZ);
        const ticksALT = Number(configuration.ticksALT);
        const latitude = Number(location.latitude);
        const longitude = Number(location.longitude);
        const azimuth = Number(orientation.azimuth);
        const measuredAltitude = Number(orientation.altitude);
        const altitude = Number.isFinite(measuredAltitude)
          ? Math.max(-90, Math.min(90, measuredAltitude))
          : Math.max(-90, Math.min(90, 90 - Math.abs(Number(orientation.pitch) || 0)));
        const status = {
          rawAZ: (((azimuth || 0) % 360) + 360) % 360,
          rawALT: altitude < 0 ? altitude + 360 : altitude,
          ticksAZ: Number.isFinite(ticksAZ) && ticksAZ > 0 ? ticksAZ : 360,
          ticksALT: Number.isFinite(ticksALT) && ticksALT > 0 ? ticksALT : 360,
          dirAZ: 1,
          dirALT: 1,
          pinAzA: 18,
          pinAzB: 19,
          pinAltA: 21,
          pinAltB: 22,
          revAZ: configuration.revAZ === true,
          revALT: configuration.revALT === true,
          lat: Number.isFinite(latitude) && Math.abs(latitude) <= 90 ? latitude : 45.89,
          lon: Number.isFinite(longitude) && Math.abs(longitude) <= 180 ? longitude : 6.05
        };
        return Promise.resolve(new Response(JSON.stringify(status), {
          status: 200,
          headers: { "Content-Type": "application/json" }
        }));
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

    if (url.origin === window.location.origin && apiRoutes.has(url.pathname)) {
      const base = (localStorage.getItem("dobsonApiBase") || "http://192.168.4.1").replace(/\/+$/, "");
      const apiUrl = `${base}${url.pathname}${url.search}`;
      const request = input instanceof Request ? new Request(apiUrl, input) : apiUrl;
      return originalFetch(request, init);
    }

    return originalFetch(input, init);
  };
})();
