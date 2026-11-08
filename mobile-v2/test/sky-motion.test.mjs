import assert from "node:assert/strict";
import test from "node:test";
import { angularSeparationDegrees, hasSignificantSkyMotion } from "../src/sky-motion.js";

test("active le pointage après un mouvement franc en azimut", () => {
  assert.equal(hasSignificantSkyMotion({ azimuth: 20, altitude: 0 }, { azimuth: 30, altitude: 0 }), true);
});

test("active le pointage après un mouvement franc vers le haut ou vers le bas", () => {
  assert.equal(hasSignificantSkyMotion({ azimuth: 20, altitude: 0 }, { azimuth: 20, altitude: 10 }), true);
  assert.equal(hasSignificantSkyMotion({ azimuth: 20, altitude: 0 }, { azimuth: 20, altitude: -10 }), true);
});

test("calcule le mouvement au passage de 360 degrés sans faux grand déplacement", () => {
  assert.ok(angularSeparationDegrees(
    { azimuth: 359, altitude: 0 },
    { azimuth: 1, altitude: 0 }
  ) < 3);
  assert.equal(hasSignificantSkyMotion({ azimuth: 359, altitude: 0 }, { azimuth: 1, altitude: 0 }), false);
});

test("n’active pas le pointage sur un petit mouvement", () => {
  assert.equal(hasSignificantSkyMotion({ azimuth: 20, altitude: 0 }, { azimuth: 22, altitude: 1 }), false);
});
