import assert from "node:assert/strict";
import test from "node:test";
import { loadMobileCatalog } from "../src/mobile-catalog.js";

test("ne télécharge pas le catalogue sur les pages qui ne l’affichent pas", async () => {
  let fetchCount = 0;
  const catalog = await loadMobileCatalog("/config.html", async () => {
    fetchCount += 1;
    throw new Error("Le catalogue ne devrait pas être demandé.");
  });

  assert.equal(catalog, null);
  assert.equal(fetchCount, 0);
});

test("charge directement le catalogue local pour les pages qui en ont besoin", async () => {
  const expectedCatalog = [{ name: "Polaris" }];
  const requestedUrls = [];
  const catalogPages = ["/", "/index.html", "/beta_sky.html", "/station.html"];

  for (const pathname of catalogPages) {
    const catalog = await loadMobileCatalog(pathname, async (url) => {
      requestedUrls.push(url);
      return {
        ok: true,
        json: async () => ({ catalog: expectedCatalog })
      };
    });
    assert.deepEqual(catalog, expectedCatalog);
  }

  assert.deepEqual(requestedUrls, catalogPages.map(() => "/mobile-catalog.json"));
});

test("signale un fichier de catalogue local invalide", async () => {
  await assert.rejects(
    loadMobileCatalog("/index.html", async () => ({
      ok: true,
      json: async () => ({ catalog: null })
    })),
    /fichier du catalogue mobile est invalide/
  );
});
