const fs = require("node:fs");
const path = require("node:path");

const root = path.resolve(__dirname, "../..");
const sourcePath = path.join(root, "include", "BetaSkyPage.h");
const catalogPath = path.join(root, "include", "ObjectsDB.h");
const dataPath = path.join(__dirname, "..", "data", "constellation-stars.json");
const linesPath = path.join(__dirname, "..", "data", "constellation-lines.csv");

const constellationNames = {
  And: "Andromède",
  Ant: "Machine pneumatique",
  Aps: "Oiseau de paradis",
  Aql: "Aigle",
  Aqr: "Verseau",
  Ara: "Autel",
  Ari: "Bélier",
  Aur: "Cocher",
  Boo: "Bouvier",
  Cae: "Burin",
  Cam: "Girafe",
  Cnc: "Cancer",
  CVn: "Chiens de chasse",
  CMa: "Grand Chien",
  CMi: "Petit Chien",
  Cap: "Capricorne",
  Car: "Carène",
  Cas: "Cassiopée",
  Cen: "Centaure",
  Cep: "Céphée",
  Cet: "Baleine",
  Cha: "Caméléon",
  Cir: "Compas",
  Col: "Colombe",
  Com: "Chevelure de Bérénice",
  CrA: "Couronne australe",
  CrB: "Couronne boréale",
  Crv: "Corbeau",
  Crt: "Coupe",
  Cru: "Croix du Sud",
  Cyg: "Cygne",
  Del: "Dauphin",
  Dor: "Dorade",
  Dra: "Dragon",
  Equ: "Petit Cheval",
  Eri: "Éridan",
  For: "Fourneau",
  Gem: "Gémeaux",
  Gru: "Grue",
  Her: "Hercule",
  Hor: "Horloge",
  Hya: "Hydre",
  Hyi: "Hydre mâle",
  Ind: "Indien",
  Lac: "Lézard",
  Leo: "Lion",
  Lep: "Lièvre",
  Lib: "Balance",
  LMi: "Petit Lion",
  Lup: "Loup",
  Lyn: "Lynx",
  Lyr: "Lyre",
  Men: "Table",
  Mic: "Microscope",
  Mon: "Licorne",
  Mus: "Mouche",
  Nor: "Règle",
  Oct: "Octant",
  Oph: "Ophiuchus",
  Ori: "Orion",
  Pav: "Paon",
  Peg: "Pégase",
  Per: "Persée",
  Phe: "Phénix",
  Pic: "Peintre",
  PsA: "Poisson austral",
  Psc: "Poissons",
  Pup: "Poupe",
  Pyx: "Boussole",
  Ret: "Réticule",
  Sge: "Flèche",
  Sgr: "Sagittaire",
  Sco: "Scorpion",
  Scl: "Sculpteur",
  Sct: "Écu",
  Ser: "Serpent",
  Sex: "Sextant",
  Tau: "Taureau",
  Tel: "Télescope",
  Tri: "Triangle",
  TrA: "Triangle austral",
  Tuc: "Toucan",
  UMa: "Grande Ourse",
  UMi: "Petite Ourse",
  Vel: "Voiles",
  Vir: "Vierge",
  Vol: "Poisson volant",
  Vul: "Petit Renard"
};

function readConstellationSequences(starByHr) {
  const rows = fs.readFileSync(linesPath, "utf8").replace(/^\uFEFF/, "").trim().split(/\r?\n/).slice(1);
  const sequences = [];
  for (const row of rows) {
    const [abbreviation, count, ...values] = row.split(",").map((value) => value.trim());
    const starCount = Number(count);
    if (!Number.isInteger(starCount) || starCount < 2) continue;
    const hrs = values.slice(0, starCount).map(Number);
    if (hrs.length !== starCount || hrs.some((hr) => !starByHr.has(hr))) {
      throw new Error(`Ligne de constellation invalide ou étoile BSC absente : ${abbreviation || "sans abréviation"}.`);
    }
    const starConstellations = hrs.map((hr) => starByHr.get(hr).bayer.match(/([A-Z][A-Za-z]{2})$/)?.[1]).filter(Boolean);
    const inferredAbbreviation = abbreviation || starConstellations[0];
    if (!inferredAbbreviation || (!abbreviation && starConstellations.some((value) => value !== inferredAbbreviation))) {
      throw new Error(`Impossible d'associer cette séquence à une constellation : ${hrs.join(", ")}.`);
    }
    sequences.push({ abbreviation: inferredAbbreviation, hrs });
  }

  const abbreviations = new Set(sequences.map(({ abbreviation }) => abbreviation));
  if (abbreviations.size !== 88 || Object.keys(constellationNames).length !== 88) {
    throw new Error(`La source doit couvrir 88 constellations (trouvé : ${abbreviations.size}).`);
  }
  for (const abbreviation of abbreviations) {
    if (!constellationNames[abbreviation]) {
      throw new Error(`Nom français manquant pour la constellation ${abbreviation}.`);
    }
  }
  return sequences;
}

function formatNumber(value, digits) {
  if (!Number.isFinite(value)) throw new Error("Coordonnée stellaire invalide.");
  return Number(value.toFixed(digits)).toString();
}

function replaceDeclaration(source, declaration, replacement) {
  const start = source.indexOf(declaration);
  const end = source.indexOf("\n    ];", start);
  if (start < 0 || end < 0 || source.indexOf(declaration, start + declaration.length) >= 0) {
    throw new Error(`Déclaration introuvable ou non unique : ${declaration}`);
  }
  return source.slice(0, start) + replacement + source.slice(end + "\n    ];".length);
}

function createCenter(abbreviation, stars) {
  let x = 0;
  let y = 0;
  let z = 0;
  for (const star of stars) {
    const ra = star.ra * Math.PI / 12;
    const dec = star.dec * Math.PI / 180;
    x += Math.cos(dec) * Math.cos(ra);
    y += Math.cos(dec) * Math.sin(ra);
    z += Math.sin(dec);
  }
  const length = Math.hypot(x, y, z);
  if (!length) throw new Error(`Centre de constellation indéterminé : ${abbreviation}.`);
  const ra = (Math.atan2(y, x) * 12 / Math.PI + 24) % 24;
  const dec = Math.asin(z / length) * 180 / Math.PI;
  return { name: constellationNames[abbreviation], ra, dec };
}

function updateCatalog(stars) {
  const source = fs.readFileSync(catalogPath, "utf8").replace(/^\uFEFF/, "");
  const existingNames = new Set();
  const starByName = new Map(stars.map((star) => [star.name, star]));
  const entryPattern = /(\{\s*)"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\}/g;
  let updatedCount = 0;
  let updated = source.replace(entryPattern, (entry, opening, name, commonName, type) => {
    existingNames.add(name);
    const star = starByName.get(name);
    if (!star) return entry;
    updatedCount++;
    return `${opening}${JSON.stringify(name)}, ${JSON.stringify(commonName)}, ${JSON.stringify(type)}, ${formatNumber(star.ra, 7)}, ${formatNumber(star.dec, 6)}, ${formatNumber(star.mag, 2)}}`;
  });

  const additions = stars.filter((star) => !existingNames.has(star.name));
  if (updatedCount + additions.length !== stars.length) {
    throw new Error("Les références stellaires ne sont pas uniques dans ObjectsDB.h.");
  }
  const marker = "  // CATALOGUE MESSIER INTÉGRAL";
  const insertionPoint = updated.indexOf(marker);
  if (insertionPoint < 0 || updated.indexOf(marker, insertionPoint + marker.length) >= 0) {
    throw new Error("Impossible de trouver le début du catalogue Messier dans ObjectsDB.h.");
  }

  const newline = updated.includes("\r\n") ? "\r\n" : "\n";
  const newEntries = additions.map((star) =>
    `  {${JSON.stringify(star.name)}, ${JSON.stringify(star.commonName)}, ${JSON.stringify(star.type)}, ${formatNumber(star.ra, 7)}, ${formatNumber(star.dec, 6)}, ${formatNumber(star.mag, 2)}}`
  );
  if (newEntries.length) {
    const block = [
      "  // Étoiles des figures : Bright Star Catalogue, Hoffleit & Warren (1991), VizieR V/50.",
      ...newEntries,
      ""
    ].join(newline) + newline;
    updated = updated.slice(0, insertionPoint) + block + updated.slice(insertionPoint);
  }
  fs.writeFileSync(catalogPath, updated, "utf8");
  return { updatedCount, addedCount: additions.length };
}

function sync() {
  const dataset = JSON.parse(fs.readFileSync(dataPath, "utf8"));
  if (!dataset || !Array.isArray(dataset.stars) || dataset.stars.length < 400) {
    throw new Error("Le sous-catalogue BSC doit contenir au moins 400 étoiles.");
  }
  const starByHr = new Map(dataset.stars.map((star) => [Number(star.hr), star]));
  if (starByHr.size !== dataset.stars.length) throw new Error("Identifiants HR en double.");

  const sequences = readConstellationSequences(starByHr);
  const pairs = [];
  const starsByConstellation = new Map();
  for (const { abbreviation, hrs } of sequences) {
    if (!starsByConstellation.has(abbreviation)) starsByConstellation.set(abbreviation, new Set());
    const constellationStars = starsByConstellation.get(abbreviation);
    for (const hr of hrs) constellationStars.add(hr);
    for (let index = 1; index < hrs.length; index++) {
      pairs.push([starByHr.get(hrs[index - 1]).name, starByHr.get(hrs[index]).name]);
    }
  }

  const centers = [...starsByConstellation.entries()].map(([abbreviation, hrs]) =>
    createCenter(abbreviation, [...hrs].map((hr) => starByHr.get(hr)))
  );
  const linesDeclaration = `const CONSTELLATION_LINES = [\n${pairs.map((pair) => `      ${JSON.stringify(pair)}`).join(",\n")}\n    ];`;
  const centersDeclaration = `const CONSTELLATION_CENTERS = [\n${centers.map((center) =>
    `      { name: ${JSON.stringify(center.name)}, ra: ${formatNumber(center.ra, 7)}, dec: ${formatNumber(center.dec, 6)} }`
  ).join(",\n")}\n    ];`;

  let page = fs.readFileSync(sourcePath, "utf8").replace(/^\uFEFF/, "");
  page = replaceDeclaration(page, "const CONSTELLATION_LINES = [", linesDeclaration);
  page = replaceDeclaration(page, "const CONSTELLATION_CENTERS = [", centersDeclaration);
  const attribution = "    // Adapted from ConstellationLines by Marc van der Sluys, CC BY 4.0.";
  if (!page.includes(attribution)) {
    page = page.replace("    const CONSTELLATION_LINES = [", `${attribution}\n    const CONSTELLATION_LINES = [`);
  }
  const catalogResult = updateCatalog(dataset.stars);
  fs.writeFileSync(sourcePath, page, "utf8");

  console.log(
    `Constellations synchronisées : ${centers.length}; étoiles : ${dataset.stars.length}; segments : ${pairs.length}; étoiles ajoutées : ${catalogResult.addedCount}; coordonnées corrigées : ${catalogResult.updatedCount}.`
  );
}

sync();
