(() => {
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

      if (url.pathname === "/status" && !window.__dobsonEspConnected) {
        let location = { latitude: 45.89, longitude: 6.05 };
        let orientation = { azimuth: 0, altitude: 0, pitch: 90 };
        try {
          location = { ...location, ...JSON.parse(localStorage.getItem("dobson-mobile-location") || "{}") };
          orientation = { ...orientation, ...(window.__dobsonPhoneOrientation || {}) };
        } catch (error) {
          console.warn("Impossible de lire la position locale du planétarium.", error);
        }
        const ticks = 360;
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
          ticksAZ: ticks,
          ticksALT: ticks,
          dirAZ: 1,
          dirALT: 1,
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
