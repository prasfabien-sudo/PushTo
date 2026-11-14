const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");

test("le diagnostic TCP est accessible depuis les paramètres et compare les compteurs", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../src/mobile.js"), "utf8");
  assert.match(source, /id="dobson-test-connection"/);
  assert.match(source, /technicalContent\.appendChild\(connectionPanel\)/);
  assert.match(source, /await DobsonNative\.getEncoderStatus\(\)/);
  assert.match(source, /rawAZ - previousEncoderTest\.rawAZ/);
  assert.match(source, /rawALT - previousEncoderTest\.rawALT/);
  assert.match(source, /if \(!state\.connected\) \{\s*previousEncoderTest = undefined/);
});

test("la navigation mobile garde trois piliers et marque la page active", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../src/mobile.js"), "utf8");
  const navigationFunction = source.match(/function installNativeNavigation\(\) \{[\s\S]*?\r?\n\}/);
  assert.ok(navigationFunction);

  const expectedPages = [
    ["/", "/beta_sky.html", "page"],
    ["/beta_sky.html", "/beta_sky.html", "page"],
    ["/index.html", "/index.html", "page"],
    ["/station.html", "/index.html", "page"],
    ["/calib_page.html", "/index.html", "page"],
    ["/simu.html", "/index.html", "page"],
    ["/config.html", "/config.html", "page"],
    ["/help.html", "/config.html", "page"],
    ["/releasenotes.html", "/config.html", "page"]
  ];

  for (const [pathname, expectedHref, expectedCurrent] of expectedPages) {
    const body = {
      appendChild(element) {
        element.parentNode = this;
      }
    };
    const navigation = {
      links: [],
      setAttribute() {},
      children: [],
      replaceChildren(...links) {
        this.children.forEach((element) => { element.parentNode = null; });
        this.children = links;
        links.forEach((element) => { element.parentNode = this; });
        this.links = links;
      }
    };
    const nightModeToggle = {
      id: "nightModeToggle",
      parentNode: navigation,
      setAttribute() {}
    };
    navigation.children.push(nightModeToggle);
    const document = {
      querySelector: () => navigation,
      createElement: () => ({
        attributes: {},
        children: [],
        setAttribute(name, value) { this.attributes[name] = value; },
        appendChild(element) { this.children.push(element); }
      }),
      getElementById: (id) => id === "nightModeToggle" && nightModeToggle.parentNode ? nightModeToggle : null,
      body
    };
    const window = { location: { pathname } };

    vm.runInNewContext(`(${navigationFunction[0]})()`, { document, window });

    assert.deepEqual(navigation.links.map((link) => link.href), [
      "/beta_sky.html",
      "/index.html",
      "/config.html"
    ]);
    assert.equal(navigation.links.find((link) => link.href === expectedHref).attributes["aria-current"] || null, expectedCurrent);
    assert.equal(nightModeToggle.parentNode, body);
  }
});

test("la caméra AR ne démarre et n'arrête pas la boussole ou les capteurs", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../src/mobile.js"), "utf8");
  const augmentedReality = source.match(/function installAugmentedReality\(\) \{([\s\S]*?)\r?\n\}\r?\n\r?\nfunction installPlateSolver/);

  assert.ok(augmentedReality);
  assert.doesNotMatch(augmentedReality[1], /dobson-sensors|startSensors|stopSensors/);
  assert.match(source, /Caméra AR active\. La boussole et les capteurs se commandent séparément\./);
});

test("la vue du ciel pilote séparément le suivi automatique et la navigation tactile", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../src/mobile.js"), "utf8");
  const styles = fs.readFileSync(path.resolve(__dirname, "../src/mobile.css"), "utf8");
  const skyPage = fs.readFileSync(path.resolve(__dirname, "../../include/BetaSkyPage.h"), "utf8");
  const configPage = fs.readFileSync(path.resolve(__dirname, "../../include/ConfigPage.h"), "utf8");

  assert.match(source, /function installNativeNavigation\(\)/);
  assert.match(source, /const sceneMenu = document\.createElement\("details"\)/);
  assert.match(source, /alignmentContent\.appendChild\(offsetReadout\)/);
  assert.match(source, /Identifier le centre et l’échelle d’une image du ciel avec Astrometry\.net/);
  assert.match(source, /label: "Aide", icon: "icon-book-open"/);
  assert.match(source, /nightModeToggle\.className = "dobson-night-toggle"/);
  assert.match(source, /icon-gamepad-2/);
  assert.match(source, /Offset AZ \/ ALT/);
  assert.match(skyPage, /aria-label="Réaligner la vue" title="Réaligner la vue"/);
  assert.match(source, /dobson-sky-control-mode/);
  assert.match(source, /pointage automatique par capteurs/i);
  assert.match(source, /Navigation tactile/);
  assert.match(source, /dobson-sky-control-mode-v3/);
  assert.match(source, /hasSignificantSkyMotion\(autoFollowBaseline, currentOrientation\)/);
  assert.match(source, /let autoFollow = localStorage\.getItem\("dobson-sky-control-mode-v3"\) !== "touch"/);
  assert.match(source, /Déplacez le téléphone pour activer le pointage/);
  assert.match(source, /cameraAltitude/);
  assert.match(source, /cameraAzimuth/);
  assert.match(source, /window\.__loadSkyObjectImage = async/);
  assert.match(skyPage, /window\.__loadSkyObjectImage\(closestObj\)/);
  assert.match(source, /hasSignificantSkyMotion\(autoFollowBaseline, currentOrientation\)/);
  assert.match(source, /magnitude\.disabled = false/);
  assert.match(configPage, /projectionModeSelect\.addEventListener\('change'/);
  assert.match(configPage, /window\.__saveMobileSkyPrefs\(\{ projMode: projectionModeSelect\.value \}\)/);
  assert.match(configPage, /d\.lat !== undefined \? d\.lat : 45\.89/);
  assert.match(configPage, /d\.lon !== undefined \? d\.lon : 6\.05/);
  assert.match(source, /dobson-technical-options/);
  assert.match(source, /dobson-location-options/);
  assert.match(source, /Configuration enregistrée sur ce téléphone/);
  assert.match(source, /configurationFields\.has\(event\.target\.id\)/);
  assert.doesNotMatch(source, /dobson-sky-pointing-axis|Direction de pointage/);
  assert.match(source, /azimuth: orientation\.cameraAzimuth,\s+altitude: orientation\.cameraAltitude/);
  assert.match(source, /altitude >= 2 \|\| hasSignificantSkyMotion/);
  const sensorHandler = source.match(/sensorListener = await DobsonNative\.addListener\("orientation", \(orientation\) => \{([\s\S]*?)\r?\n    \}\);/);
  assert.ok(sensorHandler);
  assert.doesNotMatch(sensorHandler[1], /skyTrackingActive = false/);
  assert.match(source, /const enterTouchNavigation = \(\) => \{\s+if \(!autoFollow\) return;\s+autoFollow = false;\s+skyTrackingActive = false;/);
  assert.match(source, /gestureStarts\.set\(event\.pointerId/);
  assert.match(source, /Math\.hypot\(event\.clientX - start\.x, event\.clientY - start\.y\) >= 8/);
  assert.match(source, /enterTouchNavigation\(\)/);
  assert.match(source, /for \(const id of \["openSearchTargetModalBtn", "centerButton", "manualFullscreenBtn"\]\)/);
  assert.match(source, /dobson-scene-alignment/);
  assert.match(source, /dobson-config-sky-options/);
  assert.match(source, /data-sky-pref="showGrid"/);
  assert.match(source, /data-sky-pref="labelGal"/);
  assert.match(source, /dobson-sky-magnitude/);
  assert.match(source, /connectionPanel\.classList\.add\("dobson-config-connection-panel"\)/);
  assert.match(source, /\.appendChild\(connectionPanel\)/);
  assert.match(styles, /background: #080a10 !important/);
  assert.match(styles, /width: min\(100%, 520px\)/);
  assert.match(styles, /dobson-night-toggle/);
  assert.match(styles, /dobson-scene-menu-content/);
  assert.match(styles, /body\.landscape-fullscreen \.dobson-night-toggle/);
});

test("l’application ne charge le gros catalogue que sur ses trois vues de catalogue", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../src/mobile.js"), "utf8");

  assert.match(source, /loadMobileCatalog\(window\.location\.pathname, window\.fetch\.bind\(window\)\)/);
  assert.doesNotMatch(source, /initializeNativeCatalog|DobsonNative\.initializeCatalog/);
});

test("l’icône fournie est configurée pour Android et iOS", () => {
  const root = path.resolve(__dirname, "..");
  const manifest = fs.readFileSync(path.join(root, "android/app/src/main/AndroidManifest.xml"), "utf8");
  const iosIcon = fs.readFileSync(path.join(root, "ios/App/App/Assets.xcassets/AppIcon.appiconset/Contents.json"), "utf8");
  const iconSvg = fs.readFileSync(path.join(root, "resources/icon.svg"), "utf8");
  const adaptiveBackground = fs.readFileSync(path.join(root, "android/app/src/main/res/values/ic_launcher_background.xml"), "utf8");

  assert.match(manifest, /android:icon="@mipmap\/ic_launcher"/);
  assert.match(iosIcon, /AppIcon-512@2x\.png/);
  assert.match(iconSvg, /fill="#0B1026"/);
  assert.match(iconSvg, /stroke="#5EEAD4"/);
  assert.match(adaptiveBackground, /#0B1026/);
  const pngDimensions = (file) => {
    const data = fs.readFileSync(file);
    assert.equal(data.toString("hex", 0, 8), "89504e470d0a1a0a");
    return [data.readUInt32BE(16), data.readUInt32BE(20)];
  };
  assert.deepEqual(pngDimensions(path.join(root, "resources/icon.png")), [1024, 1024]);
  assert.deepEqual(pngDimensions(path.join(root, "ios/App/App/Assets.xcassets/AppIcon.appiconset/AppIcon-512@2x.png")), [1024, 1024]);

  const androidDensities = [
    ["mdpi", 48, 108],
    ["hdpi", 72, 162],
    ["xhdpi", 96, 216],
    ["xxhdpi", 144, 324],
    ["xxxhdpi", 192, 432]
  ];
  for (const [density, launcherSize, foregroundSize] of androidDensities) {
    const mipmap = path.join(root, `android/app/src/main/res/mipmap-${density}`);
    assert.deepEqual(pngDimensions(path.join(mipmap, "ic_launcher.png")), [launcherSize, launcherSize]);
    assert.deepEqual(pngDimensions(path.join(mipmap, "ic_launcher_round.png")), [launcherSize, launcherSize]);
    assert.deepEqual(pngDimensions(path.join(mipmap, "ic_launcher_foreground.png")), [foregroundSize, foregroundSize]);
  }
});

test("l’application harmonise ses barres système Android avec son thème", () => {
  const root = path.resolve(__dirname, "..");
  const activity = fs.readFileSync(path.join(root, "android/app/src/main/java/fr/dobsonpushto/mobile/MainActivity.java"), "utf8");
  const styles = fs.readFileSync(path.join(root, "android/app/src/main/res/values/styles.xml"), "utf8");

  assert.match(activity, /setStatusBarColor\(SYSTEM_BAR_COLOR\)/);
  assert.match(activity, /setNavigationBarColor\(SYSTEM_BAR_COLOR\)/);
  assert.match(activity, /Color\.rgb\(8, 10, 16\)/);
  assert.match(activity, /setAppearanceLightStatusBars\(false\)/);
  assert.match(activity, /setAppearanceLightNavigationBars\(false\)/);
  assert.match(activity, /SYSTEM_UI_FLAG_LIGHT_STATUS_BAR \| View\.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR/);
  assert.match(activity, /onWindowFocusChanged\(boolean hasFocus\)/);
  assert.match(styles, /postSplashScreenTheme/);
  assert.match(styles, /android:windowBackground">#080a10/);
  assert.match(styles, /android:statusBarColor">#080a10/);
  assert.match(styles, /android:navigationBarColor">#080a10/);
});
