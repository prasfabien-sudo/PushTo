const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");
const { makeMobileHtml, pages, prepare, readCatalog, readMobileCatalog } = require("../scripts/prepare-web.cjs");

test("extrait les huit écrans et enrichit le catalogue mobile depuis les données firmware", () => {
  const result = prepare();
  assert.equal(result.pageCount, 8);
  assert.equal(readCatalog().length, 947);
  assert.equal(result.objectCount, readMobileCatalog().length);
  assert.ok(result.objectCount > 8000);
  const mobileCatalog = JSON.parse(fs.readFileSync(path.resolve(__dirname, "../public/mobile-catalog.json"), "utf8"));
  assert.equal(mobileCatalog.catalog.length, result.objectCount);

  for (const page of pages) {
    const html = fs.readFileSync(path.resolve(__dirname, "../src", page.output), "utf8");
    assert.match(html, /mobile-bootstrap\.js/);
    assert.match(html, /mobile\.js/);
    assert.match(html, /window\.__dobsonMobileApp = true/);
    assert.match(html, /<html lang="fr">/);
    if (page.output === "beta_sky.html") {
      assert.match(html, /dobson-ar-toggle/);
      assert.match(html, /Caméra en réalité augmentée et pointage automatique/);
      assert.match(html, /dobson-ar-video/);
      assert.match(html, /mobileHorizontalFrame/);
      assert.match(html, /now - lastObjectLabelLayout >= 80/);
      assert.match(html, /updateObjectLabels\(lastScreenObjects, width, height, labelTime\)/);
      assert.match(html, /mobileRenderFilters/);
      assert.match(html, /showMessierAll/);
      assert.match(html, /isMessierObject\(object\)/);
      assert.match(html, /const cameraElevation = elevation \+ touchElevation \+ alignOffsetElevation \+ cameraCalibrationElevation/);
      assert.match(html, /normalizeSkyAngles\(cameraHeading, cameraElevation\)/);
      assert.match(html, /skyCameraBasis\(cameraHeading, cameraElevation\)/);
      assert.match(html, /const right = \[Math\.cos\(azimuthRadians\), 0, -Math\.sin\(azimuthRadians\)\]/);
      assert.match(html, /applyTouchPan\(event\.clientX - dragPoint\.x, event\.clientY - dragPoint\.y, panPanFactor\)/);
      assert.match(html, /Math\.min\(window\.__dobsonMobileApp \? 120 : 75, fieldOfView \+ 4\)/);
      assert.match(html, /Math\.min\(window\.__dobsonMobileApp \? 120 : 75, fieldOfView \* pinchDistance \/ distance\)/);
      assert.match(html, /Math\.min\(window\.__dobsonMobileApp \? 120 : 75, fieldOfView \+ Math\.sign\(event\.deltaY\) \* 2\)/);
      assert.match(html, /top: '#123b68', mid: '#1b5680', bot: '#287a91'/);
      assert.match(html, /window\.__dobsonMobileApp \? 1\.5 : 2/);
      assert.match(html, /if \(!window\.__dobsonMobileApp\) \{\s*ctx2d\.shadowColor = colorHex/);
      assert.match(html, /window\.__dobsonEspConnected \? 100 : 33/);
      assert.match(html, /window\.setInterval\(pollEncoderStatus, 100\)/);
      assert.doesNotMatch(html, /mobileLastFrameTime/);
      assert.match(html, /selected\.size >= 250/);
      assert.match(html, /placedLabelGrid/);
      assert.doesNotMatch(html, /placedLabels\.some/);
      assert.match(html, /constellationLabelStates = new Map\(\)/);
      assert.doesNotMatch(html, /replaceChildren\(constLabels\)/);
      assert.match(html, /window\.__dobsonSetTelescopeMarker/);
      assert.match(html, /window\.__dobsonApplyPhotoCalibration/);
    }
    if (page.output === "index.html") {
      assert.match(html, /dobson-catalog-more-button/);
      assert.match(html, /mobileCatalogLimit = 100/);
      assert.match(html, /Date\.now\(\) - lastCatalogRefresh >= 1000/);
      assert.match(html, /mobileCatalogSkyCache\.clear\(\)/);
      assert.match(html, /const lst = getLST\(\)/);
      assert.ok(html.indexOf("fullCatalog.filter(o =>") < html.indexOf("mobileCatalogSkyCache.get(o)"));
    }
  }
});

test("normalise les directions au-delà du zénith et du nadir sans limite de tours", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../../include/BetaSkyPage.h"), "utf8");
  const functionSource = source.match(/    function normalizeSkyAngles\(azimuth, altitude\) \{[\s\S]*?\n    \}/);
  assert.ok(functionSource);
  const normalizeSkyAngles = vm.runInNewContext(`${functionSource[0]}; normalizeSkyAngles`);

  assert.deepEqual({ ...normalizeSkyAngles(42, 77) }, { azimuth: 42, altitude: 77 });
  assert.deepEqual({ ...normalizeSkyAngles(42, 91) }, { azimuth: 222, altitude: 89 });
  assert.deepEqual({ ...normalizeSkyAngles(42, 270) }, { azimuth: 42, altitude: -90 });
  assert.deepEqual({ ...normalizeSkyAngles(42, 450) }, { azimuth: 42, altitude: 90 });
  assert.deepEqual({ ...normalizeSkyAngles(42, -451) }, { azimuth: 222, altitude: -89 });
});

test("la caméra garde une orientation continue quand elle franchit le zénith", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../../include/BetaSkyPage.h"), "utf8");
  const functionSource = source.match(/    function skyCameraBasis\(azimuth, altitude\) \{[\s\S]*?\n    \}/);
  assert.ok(functionSource);
  const skyCameraBasis = vm.runInNewContext(`${functionSource[0]}; skyCameraBasis`);
  const before = skyCameraBasis(42, 89.99);
  const after = skyCameraBasis(42, 90.01);
  const dot = (a, b) => a.reduce((sum, value, index) => sum + value * b[index], 0);

  assert.ok(dot(before.forward, after.forward) > 0.999999);
  assert.ok(dot(before.right, after.right) > 0.999999);
  assert.ok(dot(before.up, after.up) > 0.999999);
});

test("la caméra descend sous l’horizon quand le geste tactile descend", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../../include/BetaSkyPage.h"), "utf8");
  const basisSource = source.match(/    function skyCameraBasis\(azimuth, altitude\) \{[\s\S]*?\n    \}/);
  const normalizeSource = source.match(/    function normalizeSkyAngles\(azimuth, altitude\) \{[\s\S]*?\n    \}/);
  const panSource = source.match(/    function applyTouchPan\(deltaX, deltaY, panPanFactor\) \{[\s\S]*?\n    \}/);
  assert.ok(basisSource);
  assert.ok(normalizeSource);
  assert.ok(panSource);
  const skyCameraBasis = vm.runInNewContext(`${basisSource[0]}; skyCameraBasis`);
  const normalizeSkyAngles = vm.runInNewContext(`${normalizeSource[0]}; normalizeSkyAngles`);
  const panState = vm.runInNewContext(`(() => {
    let touchHeading = 0;
    let touchElevation = 0;
    ${panSource[0]}
    return {
      pan: applyTouchPan,
      get heading() { return touchHeading; },
      get elevation() { return touchElevation; },
      set heading(value) { touchHeading = value; },
      set elevation(value) { touchElevation = value; }
    };
  })()`, {
    reverseTouchAzimuth: false,
    reverseTouchAltitude: false,
    window: { __dobsonMobileApp: true }
  });

  panState.elevation = 1;
  panState.pan(0, 8, 1);
  assert.equal(panState.elevation, -1);
  assert.equal(normalizeSkyAngles(42, panState.elevation).altitude, -1);

  panState.elevation = 89.99;
  const beforeZenith = skyCameraBasis(42, panState.elevation);
  panState.pan(0, -0.08, 1);
  assert.ok(panState.elevation > 90);
  const afterZenith = skyCameraBasis(42, panState.elevation);
  const dot = (a, b) => a.reduce((sum, value, index) => sum + value * b[index], 0);
  assert.ok(dot(beforeZenith.forward, afterZenith.forward) > 0.999999);
  assert.ok(dot(beforeZenith.right, afterZenith.right) > 0.999999);
  assert.ok(dot(beforeZenith.up, afterZenith.up) > 0.999999);

  panState.heading = 42;
  panState.pan(10, 0, 1);
  assert.equal(panState.heading, 38.5);
  panState.pan(-10, 0, 1);
  assert.equal(panState.heading, 42);

  const statusSource = source.match(/    function applyEncoderStatus\(status\) \{[\s\S]*?\n    \}/);
  assert.ok(statusSource);
  assert.doesNotMatch(statusSource[0], /touchHeading|touchElevation/);

  const before = skyCameraBasis(42, 0.01);
  const belowHorizon = skyCameraBasis(42, -0.01);

  assert.ok(normalizeSkyAngles(42, -0.01).altitude < 0);
  assert.ok(dot(before.forward, belowHorizon.forward) > 0.999999);
  assert.ok(dot(before.right, belowHorizon.right) > 0.999999);
  assert.ok(dot(before.up, belowHorizon.up) > 0.999999);
});

test("réécrit les liens entre pages sans transformer les appels API", () => {
  const html = makeMobileHtml(
    '<!doctype html><html lang="fr"><head></head><body><a href="/">Accueil</a><a href="/station">Station</a><a href="/cmd">API</a></body></html>'
  );
  assert.match(html, /href="\/index\.html"/);
  assert.match(html, /href="\/station\.html"/);
  assert.match(html, /href="\/cmd"/);
  assert.match(html, /window\.__dobsonMobileApp = true/);
});
