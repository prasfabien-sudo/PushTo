const API_BASE = "https://nova.astrometry.net/api";

export async function solveWithAstrometry(image, apiKey, reportStatus, dependencies = {}) {
  const request = dependencies.fetch || fetch;
  const wait = dependencies.delay || ((milliseconds) => new Promise((resolve) => setTimeout(resolve, milliseconds)));
  const loginBody = new URLSearchParams({
    "request-json": JSON.stringify({ apikey: apiKey })
  });
  const login = await fetchAstrometryJson(request, `${API_BASE}/login`, {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: loginBody
  });
  if (login.status !== "success" || !login.session) {
    throw new Error(login.errormessage || login.message || "Astrometry.net a refusé la clé API.");
  }

  reportStatus("Envoi privé de l’image à Astrometry.net…");
  const uploadForm = new FormData();
  uploadForm.append("request-json", JSON.stringify({
    session: login.session,
    publicly_visible: "n",
    allow_commercial_use: "d",
    allow_modifications: "d"
  }));
  uploadForm.append("file", image, image.name || "dobson-sky-image");
  const upload = await fetchAstrometryJson(request, `${API_BASE}/upload`, {
    method: "POST",
    body: uploadForm
  });
  if (upload.status !== "success" || !upload.subid) {
    throw new Error(upload.errormessage || upload.message || "Astrometry.net n’a pas accepté l’image.");
  }

  reportStatus("Image envoyée. Recherche du travail de résolution…");
  let jobId;
  for (let attempt = 0; attempt < 60 && !jobId; attempt++) {
    await wait(3000);
    const submission = await fetchAstrometryJson(request, `${API_BASE}/submissions/${upload.subid}`);
    jobId = submission.jobs?.find(Boolean);
    if (submission.processing) reportStatus("Résolution en cours sur Astrometry.net…");
  }
  if (!jobId) throw new Error("Astrometry.net n’a pas démarré la résolution dans le délai imparti.");

  let job;
  for (let attempt = 0; attempt < 60; attempt++) {
    await wait(3000);
    job = await fetchAstrometryJson(request, `${API_BASE}/jobs/${jobId}`);
    if (job.status === "success") break;
    if (job.status === "failure") throw new Error("Astrometry.net n’a pas pu identifier les étoiles de l’image.");
    reportStatus("Résolution astrométrique en cours…");
  }
  if (job?.status !== "success") throw new Error("La résolution dépasse le délai d’attente.");

  const solution = await fetchAstrometryJson(request, `${API_BASE}/jobs/${jobId}/calibration/`);
  const ra = Number(solution.ra);
  const dec = Number(solution.dec);
  const pixelScale = Number(solution.pixscale);
  if (![ra, dec, pixelScale].every(Number.isFinite)) {
    throw new Error("Astrometry.net a renvoyé une calibration incomplète.");
  }
  const raHours = ((ra / 15) % 24 + 24) % 24;
  return `Résolution réussie : AD ${raHours.toFixed(4)} h · Dec ${dec.toFixed(4)}° · échelle ${pixelScale.toFixed(2)}″/pixel.`;
}

async function fetchAstrometryJson(request, url, options = {}) {
  const response = await request(url, options);
  let body;
  try {
    body = await response.json();
  } catch {
    throw new Error(`Réponse Astrometry.net invalide (HTTP ${response.status}).`);
  }
  if (!response.ok) {
    throw new Error(body.errormessage || body.message || `Astrometry.net a répondu HTTP ${response.status}.`);
  }
  return body;
}
