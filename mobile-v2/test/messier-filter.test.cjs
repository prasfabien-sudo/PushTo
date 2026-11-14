const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");

test("l’option Messier ignore la magnitude sans désactiver les catégories", () => {
  const source = fs.readFileSync(path.resolve(__dirname, "../../include/BetaSkyPage.h"), "utf8");
  const getObjectType = source.match(/    function getObjectType\(object\) \{[\s\S]*?\n    \}/);
  const isMessierObject = source.match(/    function isMessierObject\(object\) \{[\s\S]*?\n    \}/);
  const matchesFilter = source.match(/    function matchesFilter\(object\) \{[\s\S]*?\n    \}/);
  assert.ok(getObjectType && isMessierObject && matchesFilter);

  const controls = {
    showMessierAll: { checked: true },
    showGal: { checked: true },
    showNeb: { checked: true },
    showAmas: { checked: true },
    showStars: { checked: true }
  };
  const context = vm.createContext({
    document: { getElementById: (id) => controls[id] },
    getSelectedMagnitude: () => 5
  });
  const filter = vm.runInContext(
    `${getObjectType[0]}\n${isMessierObject[0]}\n${matchesFilter[0]}\n({ getObjectType, isMessierObject, matchesFilter })`,
    context
  );
  const messierNebula = { name: "M42", type: "Nébuleuse", mag: 8.4 };

  assert.deepEqual({ ...filter.getObjectType(messierNebula) }, {
    isGal: false,
    isNeb: true,
    isAmas: false,
    isStar: false
  });
  assert.equal(filter.matchesFilter(messierNebula), true);
  assert.equal(filter.isMessierObject({ name: "M110" }), true);
  assert.equal(filter.isMessierObject({ name: "M111" }), false);

  controls.showMessierAll.checked = false;
  assert.equal(filter.matchesFilter(messierNebula), false);
  controls.showMessierAll.checked = true;
  controls.showNeb.checked = false;
  assert.equal(filter.matchesFilter(messierNebula), false);
});
