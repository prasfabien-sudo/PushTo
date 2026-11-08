const journalOrigin = "https://journaldelastronomie.fr";
const imageCache = new Map();

function normalizeText(value) {
  return String(value || "")
    .normalize("NFD")
    .replace(/\p{Diacritic}/gu, "")
    .toLowerCase()
    .replace(/[^a-z0-9]/g, "");
}

function matchesObjectType(result, objectType) {
  const type = normalizeText(objectType);
  const subtype = normalizeText(result.subtype);
  if (type.includes("neb") && subtype.includes("neb")) return true;
  if (type.includes("gal") && subtype.includes("gal")) return true;
  if ((type.includes("amas") || type.includes("cluster")) && subtype.includes("amas")) return true;
  return false;
}

function findMatchingPost(results, query, objectType) {
  const normalizedQuery = normalizeText(query);
  return results
    .filter((result) => normalizeText(result.title).includes(normalizedQuery))
    .sort((left, right) => Number(matchesObjectType(right, objectType)) - Number(matchesObjectType(left, objectType)))[0];
}

function trustedUrl(value, allowApi = false) {
  try {
    const url = new URL(value);
    if (url.protocol !== "https:" || url.hostname !== "journaldelastronomie.fr") return null;
    if (allowApi && !url.pathname.startsWith("/wp-json/wp/v2/")) return null;
    return url;
  } catch {
    return null;
  }
}

async function lookupAstronomyImage(object, fetchImpl) {
  const queries = [...new Set([object.name, object.commonName].filter(Boolean))];
  for (const query of queries) {
    const searchUrl = new URL("/wp-json/wp/v2/search", journalOrigin);
    searchUrl.searchParams.set("search", query);
    searchUrl.searchParams.set("per_page", "5");
    const searchResponse = await fetchImpl(searchUrl);
    if (!searchResponse.ok) {
      throw new Error(`Recherche d’image impossible (HTTP ${searchResponse.status}).`);
    }

    const results = await searchResponse.json();
    if (!Array.isArray(results)) throw new Error("Réponse de recherche d’images invalide.");
    const post = findMatchingPost(results, query, object.type);
    if (!post) continue;

    const apiUrl = trustedUrl(post._links?.self?.[0]?.href, true);
    if (!apiUrl) continue;
    apiUrl.searchParams.set("_embed", "wp:featuredmedia");
    apiUrl.searchParams.set("_fields", "link,title,_embedded");
    const postResponse = await fetchImpl(apiUrl);
    if (!postResponse.ok) {
      throw new Error(`Fiche astronomique indisponible (HTTP ${postResponse.status}).`);
    }

    const article = await postResponse.json();
    const media = article._embedded?.["wp:featuredmedia"]?.[0];
    const imageUrl = trustedUrl(
      media?.media_details?.sizes?.["jda-thumb"]?.source_url
        || media?.media_details?.sizes?.medium_large?.source_url
        || media?.source_url
    );
    const articleUrl = trustedUrl(article.link);
    if (!imageUrl || !articleUrl) continue;

    return {
      imageUrl: imageUrl.href,
      articleUrl: articleUrl.href,
      alt: media.alt_text || `${object.commonName || object.name} — Le Journal de l’Astronomie`
    };
  }

  return null;
}

export function findAstronomyImage(object, fetchImpl = fetch) {
  const cacheKey = normalizeText(`${object.name || ""}|${object.commonName || ""}|${object.type || ""}`);
  if (!imageCache.has(cacheKey)) {
    const imageRequest = lookupAstronomyImage(object, fetchImpl).catch((error) => {
      imageCache.delete(cacheKey);
      throw error;
    });
    imageCache.set(cacheKey, imageRequest);
  }
  return imageCache.get(cacheKey);
}
