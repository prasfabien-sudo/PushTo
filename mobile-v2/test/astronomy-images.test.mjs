import assert from "node:assert/strict";
import test from "node:test";
import { findAstronomyImage } from "../src/astronomy-images.js";

test("charge l’image mise en avant de la fiche du même type d’objet", async () => {
  const requests = [];
  const fetchMock = async (input) => {
    const url = new URL(input);
    requests.push(url);
    if (url.pathname.endsWith("/search")) {
      return Response.json([
        {
          title: "NGC 2264 (amas ouvert)",
          subtype: "amas",
          _links: { self: [{ href: "https://journaldelastronomie.fr/wp-json/wp/v2/amas/1006" }] }
        },
        {
          title: "Nébuleuse du Cône (NGC 2264)",
          subtype: "nebuleuse",
          _links: { self: [{ href: "https://journaldelastronomie.fr/wp-json/wp/v2/nebuleuse/706" }] }
        }
      ]);
    }

    return Response.json({
      link: "https://journaldelastronomie.fr/nebuleuse/ngc-2264/",
      _embedded: {
        "wp:featuredmedia": [{
          source_url: "https://journaldelastronomie.fr/wp-content/uploads/cone.jpg",
          alt_text: "",
          media_details: {
            sizes: {
              "jda-thumb": {
                source_url: "https://journaldelastronomie.fr/wp-content/uploads/cone-thumb.jpg"
              }
            }
          }
        }]
      }
    });
  };

  const image = await findAstronomyImage({
    name: "NGC 2264",
    commonName: "Nébuleuse du Cône",
    type: "Nébuleuse"
  }, fetchMock);

  assert.deepEqual(image, {
    imageUrl: "https://journaldelastronomie.fr/wp-content/uploads/cone-thumb.jpg",
    articleUrl: "https://journaldelastronomie.fr/nebuleuse/ngc-2264/",
    alt: "Nébuleuse du Cône — Le Journal de l’Astronomie"
  });
  assert.equal(requests.length, 2);
  assert.equal(requests[1].pathname, "/wp-json/wp/v2/nebuleuse/706");
});

test("ignore les résultats sans image hébergée sur le site source", async () => {
  const fetchMock = async (input) => {
    const url = new URL(input);
    if (url.pathname.endsWith("/search")) {
      return Response.json([{
        title: "M31 Andromède",
        subtype: "galaxie",
        _links: { self: [{ href: "https://journaldelastronomie.fr/wp-json/wp/v2/galaxie/503" }] }
      }]);
    }
    return Response.json({
      link: "https://journaldelastronomie.fr/galaxie/m31/",
      _embedded: {
        "wp:featuredmedia": [{
          source_url: "https://images.example.org/andromede.jpg",
          media_details: { sizes: {} }
        }]
      }
    });
  };

  const image = await findAstronomyImage({
    name: "M31",
    commonName: "Andromède",
    type: "Galaxie"
  }, fetchMock);

  assert.equal(image, null);
});

test("signale explicitement les erreurs du service d’images", async () => {
  await assert.rejects(
    findAstronomyImage({ name: "NGC 999999", type: "Galaxie" }, async () => new Response(null, { status: 503 })),
    /Recherche d’image impossible \(HTTP 503\)/
  );
});
