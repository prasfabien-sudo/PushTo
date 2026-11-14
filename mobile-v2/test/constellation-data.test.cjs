const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");
const { readCatalog } = require("../scripts/prepare-web.cjs");

const root = path.resolve(__dirname, "../..");

function readArrayDeclaration(source, name) {
  const declaration = source.match(new RegExp(`const ${name} = (\\[[\\s\\S]*?\\n    \\]);`));
  assert.ok(declaration, `Déclaration ${name} absente.`);
  return vm.runInNewContext(declaration[1]);
}

function readLineSequences() {
  const rows = fs.readFileSync(path.join(root, "mobile-v2/data/constellation-lines.csv"), "utf8")
    .replace(/^\uFEFF/, "")
    .trim()
    .split(/\r?\n/)
    .slice(1);
  return rows.map((row) => {
    const [abbreviation, count, ...values] = row.split(",").map((value) => value.trim());
    const hrs = values.slice(0, Number(count)).map(Number);
    assert.equal(hrs.length, Number(count));
    return { abbreviation, hrs };
  });
}

test("les 88 figures couvrent les 697 étoiles BSC sans segment fantôme", () => {
  const page = fs.readFileSync(path.join(root, "include/BetaSkyPage.h"), "utf8");
  const dataset = JSON.parse(fs.readFileSync(path.join(root, "mobile-v2/data/constellation-stars.json"), "utf8"));
  const catalog = readCatalog();
  const starsByHr = new Map(dataset.stars.map((star) => [star.hr, star]));
  const sequences = readLineSequences();
  const lines = JSON.parse(JSON.stringify(readArrayDeclaration(page, "CONSTELLATION_LINES")));
  const centers = readArrayDeclaration(page, "CONSTELLATION_CENTERS");
  const expectedLines = [];
  const starsByConstellation = new Map();

  assert.equal(dataset.stars.length, 697);
  assert.equal(starsByHr.size, dataset.stars.length);
  assert.equal(centers.length, 88);
  assert.equal(new Set(centers.map((center) => center.name)).size, 88);
  assert.equal(catalog.length, 947);
  assert.ok(catalog.some((object) => object.name === "M110"));

  for (const { abbreviation, hrs } of sequences) {
    const starConstellation = abbreviation || starsByHr.get(hrs[0]).bayer.match(/([A-Z][A-Za-z]{2})$/)?.[1];
    assert.ok(starConstellation);
    if (!starsByConstellation.has(starConstellation)) starsByConstellation.set(starConstellation, new Set());
    for (const hr of hrs) {
      assert.ok(starsByHr.has(hr), `HR ${hr} absent du sous-catalogue.`);
      starsByConstellation.get(starConstellation).add(hr);
    }
    for (let index = 1; index < hrs.length; index++) {
      expectedLines.push([starsByHr.get(hrs[index - 1]).name, starsByHr.get(hrs[index]).name]);
    }
  }

  assert.equal(starsByConstellation.size, 88);
  assert.equal(lines.length, 750);
  assert.deepEqual(lines, expectedLines);
  assert.deepEqual(new Set(lines.flat()), new Set(dataset.stars.map((star) => star.name)));

  const catalogNames = new Set(catalog.map((object) => object.name));
  for (const [starA, starB] of lines) {
    assert.ok(catalogNames.has(starA), `Clé de catalogue absente : ${starA}.`);
    assert.ok(catalogNames.has(starB), `Clé de catalogue absente : ${starB}.`);
  }
  for (const star of dataset.stars) {
    assert.ok(Number.isFinite(star.ra) && star.ra >= 0 && star.ra < 24);
    assert.ok(Number.isFinite(star.dec) && star.dec >= -90 && star.dec <= 90);
    assert.ok(Number.isFinite(star.mag));
  }

  for (const hrs of starsByConstellation.values()) {
    const cartesian = [...hrs].map((hr) => {
      const star = starsByHr.get(hr);
      const ra = star.ra * Math.PI / 12;
      const dec = star.dec * Math.PI / 180;
      return [Math.cos(dec) * Math.cos(ra), Math.cos(dec) * Math.sin(ra), Math.sin(dec)];
    });
    const [x, y, z] = cartesian.reduce(
      (sum, point) => sum.map((value, index) => value + point[index]),
      [0, 0, 0]
    );
    const expectedRa = (Math.atan2(y, x) * 12 / Math.PI + 24) % 24;
    const expectedDec = Math.asin(z / Math.hypot(x, y, z)) * 180 / Math.PI;
    const center = centers.find((item) =>
      Math.abs(item.ra - expectedRa) < 1e-6 && Math.abs(item.dec - expectedDec) < 1e-5
    );
    assert.ok(center, "Centre de constellation différent du centroïde sphérique des étoiles.");
  }
});
