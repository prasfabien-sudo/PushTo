const catalogPages = new Set(["/", "/index.html", "/beta_sky.html", "/station.html"]);

export async function loadMobileCatalog(pathname, fetchCatalog) {
  if (!catalogPages.has(pathname)) return null;

  const response = await fetchCatalog("/mobile-catalog.json");
  if (!response.ok) {
    throw new Error(`Impossible de charger le catalogue mobile (HTTP ${response.status}).`);
  }
  const data = await response.json();
  if (!Array.isArray(data.catalog)) {
    throw new Error("Le fichier du catalogue mobile est invalide.");
  }
  return data.catalog;
}
