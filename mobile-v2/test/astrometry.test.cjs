const assert = require("node:assert/strict");
const test = require("node:test");
const { solveWithAstrometry } = require("../src/astrometry.js");

function jsonResponse(body) {
  return new Response(JSON.stringify(body), {
    status: 200,
    headers: { "Content-Type": "application/json" }
  });
}

test("envoie une image privée et retourne la calibration Astrometry.net", async () => {
  const calls = [];
  const responses = [
    { status: "success", session: "session-token" },
    { status: "success", subid: 42 },
    { jobs: [314] },
    { status: "success" },
    { ra: 180, dec: -20, pixscale: 1.25 }
  ];
  const request = async (url, options = {}) => {
    calls.push({ url, options });
    return jsonResponse(responses.shift());
  };
  const messages = [];
  let calibration;
  const result = await solveWithAstrometry(
    new Blob(["image-data"], { type: "image/jpeg" }),
    "secret-api-key",
    (message) => messages.push(message),
    { fetch: request, delay: async () => {}, onCalibration: (value) => { calibration = value; } }
  );

  assert.match(result, /AD 12\.0000 h · Dec -20\.0000°/);
  assert.deepEqual(calibration, { ra: 12, dec: -20, pixelScale: 1.25 });
  assert.equal(calls.length, 5);
  assert.equal(calls[0].url, "https://nova.astrometry.net/api/login");
  assert.deepEqual(JSON.parse(calls[0].options.body.get("request-json")), { apikey: "secret-api-key" });
  assert.equal(calls[1].url, "https://nova.astrometry.net/api/upload");
  assert.equal(JSON.parse(calls[1].options.body.get("request-json")).publicly_visible, "n");
  assert.equal(calls[1].options.body.get("file").name, "dobson-sky-image");
  assert.ok(messages.some((message) => message.includes("privé")));
});

test("signale explicitement un échec de résolution", async () => {
  const responses = [
    { status: "success", session: "session-token" },
    { status: "success", subid: 42 },
    { jobs: [314] },
    { status: "failure" }
  ];
  await assert.rejects(
    solveWithAstrometry(new Blob(["image"]), "key", () => {}, {
      fetch: async () => jsonResponse(responses.shift()),
      delay: async () => {}
    }),
    /n’a pas pu identifier les étoiles/
  );
});
