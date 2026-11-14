const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");

function createLabelRenderer() {
  const source = fs.readFileSync(path.resolve(__dirname, "../../include/BetaSkyPage.h"), "utf8");
  const start = source.indexOf("    const objectLabelStates = new Map();");
  const end = source.indexOf("    // --- MOTEUR DE RENDU", start);
  assert.ok(start >= 0 && end > start);
  const controls = {
    labelGal: { checked: true },
    labelNeb: { checked: true },
    labelAmas: { checked: true },
    labelStars: { checked: true }
  };
  const children = [];
  const document = {
    getElementById: (id) => controls[id],
    createElement: () => ({
      style: {},
      remove() {
        children.splice(children.indexOf(this), 1);
      }
    })
  };
  const context = vm.createContext({
    document,
    labelsElement: { appendChild: (element) => children.push(element) },
    window: { __dobsonMobileApp: true },
    projectionMode: "stéréo",
    fieldOfView: 34,
    showSkyNames: true,
    getObjectType: (object) => ({
      isGal: object.type === "galaxie",
      isNeb: object.type === "nébuleuse",
      isAmas: object.type === "amas",
      isStar: object.type === "étoile"
    })
  });
  vm.runInContext(source.slice(start, end), context);
  return { context, controls, children, update: context.updateObjectLabels };
}

test("les étiquettes suivent les objets à chaque image sans réutiliser leurs éléments", () => {
  const { update, children } = createLabelRenderer();
  const a = { name: "A", type: "étoile", mag: 1 };
  const b = { name: "B", type: "étoile", mag: 2 };
  const frame = (ax, bx) => [
    { object: a, screenX: ax, screenY: 0, depth: 0.9 },
    { object: b, screenX: bx, screenY: 0, depth: 0.8 }
  ];

  assert.equal(update(frame(-0.5, 0.5), 1000, 500, 0), true);
  const [labelA, labelB] = children;
  assert.equal(update(frame(-0.4, 0.4).reverse(), 1000, 500, 16), false);
  assert.equal(labelA.textContent, "A");
  assert.equal(labelB.textContent, "B");
  assert.equal(labelA.style.transform, "translate3d(300px, 234px, 0)");
  assert.equal(labelB.style.transform, "translate3d(700px, 234px, 0)");
});

test("une étiquette visible reste prioritaire malgré le changement de profondeur", () => {
  const { update, children } = createLabelRenderer();
  const a = { name: "A", type: "étoile", mag: 1 };
  const b = { name: "B", type: "étoile", mag: 2 };
  const frame = (depthA, depthB, xB) => [
    { object: a, screenX: 0, screenY: 0, depth: depthA },
    { object: b, screenX: xB, screenY: 0, depth: depthB }
  ];

  update(frame(1, 0.5, 0.05), 1000, 500, 0);
  assert.deepEqual(children.map((label) => label.textContent), ["A"]);
  update(frame(0.5, 1, 0.05), 1000, 500, 80);
  assert.deepEqual(children.map((label) => label.textContent), ["A"]);
  update(frame(0.5, 1, 0.1), 1000, 500, 160);
  assert.deepEqual(children.map((label) => label.textContent), ["A", "B"]);
});

test("un filtre désactive immédiatement les noms et libère les éléments après la transition", () => {
  const { update, controls, children } = createLabelRenderer();
  const object = { name: "A", type: "étoile", mag: 1 };
  const frame = [{ object, screenX: 0, screenY: 0, depth: 1 }];

  update(frame, 1000, 500, 0);
  assert.equal(children[0].style.opacity, "1");
  controls.labelStars.checked = false;
  assert.equal(update(frame, 1000, 500, 16), true);
  assert.equal(children[0].style.opacity, "0");
  update(frame, 1000, 500, 166);
  assert.equal(children.length, 0);
});

test("le nombre de noms mobiles reste limité à 250", () => {
  const { context, update, children } = createLabelRenderer();
  context.fieldOfView = 1;
  const frame = Array.from({ length: 300 }, (_, index) => ({
    object: { name: `S${index}`, type: "étoile", mag: 1 },
    screenX: -0.95 + (index % 20) * 0.1,
    screenY: -0.9 + Math.floor(index / 20) * 0.12,
    depth: 1
  }));
  update(frame, 1000, 500, 0);
  assert.equal(children.length, 250);
});

test("les planètes conservent leur nom sans suffixe de catalogue", () => {
  const { update, children } = createLabelRenderer();
  const mars = { name: "Mars", commonName: "Mars", type: "Planète", mag: 1 };
  const messier = { name: "M42", commonName: "Orion", type: "nébuleuse", mag: 1 };
  update([
    { object: mars, screenX: -0.5, screenY: 0, depth: 1 },
    { object: messier, screenX: 0.5, screenY: 0, depth: 1 }
  ], 1000, 500, 0);
  assert.deepEqual(children.map((label) => label.textContent), ["Mars", "M42 (Orion)"]);
});

test("les nouvelles étoiles affichent leur désignation au lieu de leur identifiant de catalogue", () => {
  const { update, children } = createLabelRenderer();
  const star = { name: "HIP 1473", commonName: "σ And", type: "étoile", mag: 2 };

  update([{ object: star, screenX: 0, screenY: 0, depth: 1 }], 1000, 500, 0);

  assert.equal(children[0].textContent, "σ And");
});
