// Version Component: ObjectsDB.h v5.1.0
// Version Global System: v1.2.8
#ifndef OBJECTSDB_H
#define OBJECTSDB_H

#include <pgmspace.h>

struct DSO {
  const char* name;        // Nom de l'étoile ou objet
  const char* commonName;  // Constellation ou nom usuel
  const char* type;        // Type spectral ou type d'objet
  float ra;                // Ascension Droite (heures)
  float dec;               // Déclinaison (degrés - Hémisphère Nord)
  float mag;               // Magnitude apparente
};

const DSO DSO_CATALOG[] PROGMEM = {
  // ==========================================
  // ÉTOILES DE RÉFÉRENCE - HÉMISPHÈRE NORD
  // ==========================================

  // Andromède
  {"Alpheratz", "α And", "Étoile B", 0.14, 29.09, 2.07},
  {"Mirach", "β And", "Étoile M", 1.10, 35.62, 2.07},
  {"Almach", "γ And", "Étoile K", 2.06, 42.33, 2.10},
  {"Adhil", "ξ And", "Étoile K", 1.25, 45.49, 4.87},

  // Verseau (Aquarius - partiellement nord)
  {"Sadalmelik", "α Aqr", "Étoile G", 22.06, -0.32, 2.95},
  {"Sadalsuud", "β Aqr", "Étoile G", 21.52, -5.57, 2.90},
  {"Sadachbia", "γ Aqr", "Étoile A", 22.36, -1.39, 3.86},

  // Aigle (Aquila)
  {"Altair", "α Aql", "Étoile A", 19.85, 8.87, 0.77},
  {"Alshain", "β Aql", "Étoile G", 19.93, 6.40, 3.71},
  {"Tarazed", "γ Aql", "Étoile K", 19.78, 10.61, 2.72},
  {"Deneb el Okab", "ζ Aql", "Étoile A", 19.09, 13.86, 2.99},

  // Bélier (Aries)
  {"Hamal", "α Ari", "Étoile K", 2.12, 23.46, 2.01},
  {"Sheratan", "β Ari", "Étoile A", 1.93, 20.79, 2.64},
  {"Mesarthim", "γ Ari", "Étoile A", 1.92, 19.29, 3.88},

  // Cocher (Auriga)
  {"Capella", "α Aur", "Étoile G", 5.28, 45.99, 0.08},
  {"Menkalinan", "β Aur", "Étoile A", 5.96, 44.95, 1.90},
  {"Mahasim", "θ Aur", "Étoile A", 5.99, 37.21, 2.65},
  {"Hassaleh", "ι Aur", "Étoile K", 4.95, 33.19, 2.69},

  // Bouvier (Boötes)
  {"Arcturus", "α Boo", "Étoile K", 14.26, 19.18, -0.05},
  {"Nekkar", "β Boo", "Étoile G", 15.02, 40.39, 3.49},
  {"Seginus", "γ Boo", "Étoile A", 14.54, 38.31, 3.04},
  {"Hozian", "δ Boo", "Étoile G", 15.25, 33.31, 3.46},
  {"Muphrid", "η Boo", "Étoile G", 13.91, 18.40, 2.68},

  // Girafe (Camelopardalis)
  {"Beta Camelopardalis", "β Cam", "Étoile G", 5.04, 60.44, 4.03},

  // Cancer
  {"Tarf", "β Cnc", "Étoile K", 8.27, 9.19, 3.53},
  {"Acubens", "α Cnc", "Étoile A", 8.96, 11.86, 4.26},

  // Chiens de Chasse (Canes Venatici)
  {"Cor Caroli", "α CVn", "Étoile A", 12.93, 38.32, 2.89},
  {"Chara", "β CVn", "Étoile G", 12.56, 41.36, 4.24},

  // Petit Chien (Canis Minor)
  {"Procyon", "α CMi", "Étoile F", 7.65, 5.22, 0.38},
  {"Gomeisa", "β CMi", "Étoile B", 7.44, 8.29, 2.89},

  // Cassiopée (Cassiopeia)
  {"Shedar", "α Cas", "Étoile K", 0.67, 56.54, 2.24},
  {"Caph", "β Cas", "Étoile F", 0.15, 59.15, 2.28},
  {"Navi", "γ Cas", "Étoile B", 0.94, 60.71, 2.15},
  {"Ruchbah", "δ Cas", "Étoile A", 1.43, 60.23, 2.68},
  {"Segin", "ε Cas", "Étoile B", 1.90, 63.67, 3.35},

  // Céphée (Cepheus)
  {"Alderamin", "α Cep", "Étoile A", 21.31, 62.58, 2.45},
  {"Alrai", "β Cep", "Étoile K", 21.48, 70.55, 3.23},
  {"Errai", "γ Cep", "Étoile K", 23.66, 77.63, 3.21},
  {"Kurhah", "ξ Cep", "Étoile A", 22.05, 64.63, 4.26},

  // Baleine (Cetus)
  {"Menkar", "α Cet", "Étoile M", 3.04, 4.08, 2.54},
  {"Kaffaljidhma", "γ Cet", "Étoile A", 2.43, 3.23, 3.47},

  // Chevelure de Berenice (Coma Berenices)
  {"Diadem", "α Com", "Étoile F", 13.16, 17.53, 4.32},

  // Couronne Boréale (Corona Borealis)
  {"Alphecca", "α CrB", "Étoile A", 15.58, 26.71, 2.22},
  {"Nusakan", "β CrB", "Étoile F", 15.46, 29.10, 3.68},

  // Cygne (Cygnus)
  {"Deneb", "α Cyg", "Étoile A", 20.69, 45.28, 1.25},
  {"Albireo", "β Cyg", "Étoile K", 19.51, 27.96, 3.05},
  {"Sadr", "γ Cyg", "Étoile F", 20.37, 40.26, 2.23},
  {"Fawaris", "δ Cyg", "Étoile B", 19.75, 45.13, 2.87},
  {"Gienah", "ε Cyg", "Étoile K", 20.77, 33.97, 2.48},

  // Dauphin (Delphinus)
  {"Rotanev", "β Del", "Étoile F", 20.62, 14.60, 3.64},
  {"Sualocin", "α Del", "Étoile B", 20.65, 15.90, 3.77},

  // Dragon (Draco)
  {"Eltanin", "γ Dra", "Étoile K", 17.94, 51.49, 2.23},
  {"Rastaban", "β Dra", "Étoile G", 17.50, 52.30, 2.79},
  {"Thuban", "α Dra", "Étoile A", 14.07, 64.38, 3.67},

  // Petit Cheval (Equuleus)
  {"Kitalpha", "α Equ", "Étoile G", 21.26, 5.24, 3.92},

  // Gémeaux (Gemini)
  {"Pollux", "β Gem", "Étoile K", 7.76, 28.03, 1.14},
  {"Castor", "α Gem", "Étoile A", 7.38, 31.89, 1.58},
  {"Alhena", "γ Gem", "Étoile A", 6.63, 16.40, 1.93},
  {"Mebsuta", "ε Gem", "Étoile G", 6.73, 25.13, 3.06},
  {"Tejat", "μ Gem", "Étoile M", 6.24, 22.51, 2.87},

  // Hercule (Hercules)
  {"Kornephoros", "β Her", "Étoile G", 16.51, 21.48, 2.78},
  {"Rasalgethi", "α Her", "Étoile M", 17.24, 14.39, 3.48},
  {"Sujata", "δ Her", "Étoile A", 17.25, 24.85, 3.12},
  {"Marfik", "λ Her", "Étoile A", 17.50, 26.10, 4.41},

  // Lézard (Lacerta)
  {"Alpha Lacertae", "α Lac", "Étoile A", 22.55, 50.29, 3.76},

  // Lion (Leo)
  {"Regulus", "α Leo", "Étoile B", 10.14, 11.97, 1.35},
  {"Denebola", "β Leo", "Étoile A", 11.82, 14.57, 2.14},
  {"Algieba", "γ Leo", "Étoile K", 10.33, 19.84, 2.01},
  {"Zosma", "δ Leo", "Étoile A", 11.24, 20.52, 2.56},
  {"Ras Elased Australis", "ε Leo", "Étoile G", 9.76, 23.77, 2.97},

  // Petit Lion (Leo Minor)
  {"Praecipua", "46 LMi", "Étoile K", 10.48, 34.20, 3.79},

  // Lynx
  {"Alpha Lyncis", "α Lyn", "Étoile K", 9.36, 34.33, 3.82},

  // Lyre (Lyra)
  {"Vega", "α Lyr", "Étoile A", 18.61, 38.78, 0.03},
  {"Sheliak", "β Lyr", "Étoile B", 18.85, 33.36, 3.52},
  {"Sulafat", "γ Lyr", "Étoile A", 18.96, 32.69, 3.25},

  // Ophiuchus
  {"Rasalhague", "α Oph", "Étoile A", 17.58, 12.56, 2.08},

  // Orion
  {"Rigel", "β Ori", "Étoile B", 5.24, -8.20, 0.12},
  {"Betelgeuse", "α Ori", "Étoile M", 5.92, 7.41, 0.50},
  {"Bellatrix", "γ Ori", "Étoile B", 5.42, 6.35, 1.64},
  {"Alnilam", "ε Ori", "Étoile B", 5.93, -1.20, 1.69},
  {"Alnitak", "ζ Ori", "Étoile O", 5.40, -1.94, 1.77},
  {"Mintaka", "δ Ori", "Étoile O", 5.53, -0.30, 2.23},
  {"Meissa", "λ Ori", "Étoile O", 5.58, 9.93, 3.39},

  // Pégase (Pegasus)
  {"Enif", "ε Peg", "Étoile K", 21.74, 9.87, 2.38},
  {"Scheat", "β Peg", "Étoile M", 23.06, 28.08, 2.44},
  {"Markab", "α Peg", "Étoile B", 23.08, 15.21, 2.49},
  {"Algenib", "γ Peg", "Étoile B", 0.22, 15.18, 2.84},

  // Persée (Perseus)
  {"Mirfak", "α Per", "Étoile F", 3.24, 49.86, 1.79},
  {"Algol", "β Per", "Étoile B", 3.13, 40.96, 2.12},
  {"Menkib", "ζ Per", "Étoile B", 4.23, 31.86, 2.84},
  {"Atik", "ο Per", "Étoile B", 3.74, 32.30, 3.84},

  // Poissons (Pisces)
  {"Alpherg", "η Psc", "Étoile G", 1.52, 15.35, 3.62},
  {"Gamma Piscium", "γ Psc", "Étoile G", 23.29, 3.29, 3.70},

  // Flèche (Sagitta)
  {"Sham", "α Sge", "Étoile G", 19.65, 18.49, 4.39},

  // Serpent (Serpens)
  {"Unukalhai", "α Ser", "Étoile K", 15.74, 6.42, 2.63},

  // Taureau (Taurus)
  {"Aldebaran", "α Tau", "Étoile K", 4.60, 16.51, 0.85},
  {"Elnath", "β Tau", "Étoile B", 5.44, 28.61, 1.65},
  {"Alcyone", "η Tau", "Étoile B", 3.79, 24.11, 2.85},
  {"Tianguis", "ζ Tau", "Étoile B", 5.62, 21.14, 3.01},

  // Triangle (Triangulum)
  {"Beta Trianguli", "β Tri", "Étoile A", 2.15, 34.98, 3.00},
  {"Mothallah", "α Tri", "Étoile F", 1.89, 29.58, 3.42},

  // Grande Ourse (Ursa Major)
  {"Dubhe", "α UMa", "Étoile K", 11.06, 61.75, 1.79},
  {"Merak", "β UMa", "Étoile A", 11.03, 56.38, 2.37},
  {"Phecda", "γ UMa", "Étoile A", 11.90, 53.69, 2.44},
  {"Megrez", "δ UMa", "Étoile A", 12.25, 57.03, 3.31},
  {"Alioth", "ε UMa", "Étoile A", 12.90, 55.96, 1.77},
  {"Mizar", "ζ UMa", "Étoile A", 13.40, 54.92, 2.23},
  {"Alkaid", "η UMa", "Étoile B", 13.79, 49.31, 1.86},

  // Petite Ourse (Ursa Minor)
  {"Polaris", "α UMi", "Étoile F", 2.53, 89.26, 2.00},
  {"Kochab", "β UMi", "Étoile K", 14.85, 74.16, 2.08},
  {"Pherkad", "γ UMi", "Étoile A", 15.35, 71.83, 3.05},
  {"Epsilon UMi", "ε UMi", "Étoile G", 16.76, 82.04, 4.21},
  {"Zeta UMi", "ζ UMi", "Étoile A", 15.73, 77.80, 4.29},
  {"Yildun", "δ UMi", "Étoile A", 17.54, 86.59, 4.35},
  {"Eta UMi", "η UMi", "Étoile F", 16.29, 75.75, 4.95},

  // Vierge (Virgo - partiellement nord)
  {"Vindemiatrix", "ε Vir", "Étoile K", 13.03, 10.96, 2.83},
  {"Porrima", "γ Vir", "Étoile F", 12.69, -1.45, 2.74},

  // Petit Renard (Vulpecula)
  {"Anser", "α Vul", "Étoile M", 19.48, 24.40, 4.44},

 // ==========================================
  // CATALOGUE MESSIER INTÉGRAL (M1 à M110)
  // ==========================================
  {"M1", "Nébuleuse du Crabe", "Nébuleuse", 5.58, 22.01, 8.4},
  {"M2", "Amas Globulaire", "Amas Globulaire", 21.56, -0.82, 6.5},
  {"M3", "Amas Globulaire", "Amas Globulaire", 13.72, 28.38, 6.2},
  {"M4", "Amas Globulaire", "Amas Globulaire", 16.39, -26.53, 5.6},
  {"M5", "Amas Globulaire", "Amas Globulaire", 15.31, 2.06, 5.6},
  {"M6", "Amas du Papillon", "Amas Ouvert", 17.67, -32.22, 4.2},
  {"M7", "Amas de Ptolémée", "Amas Ouvert", 17.90, -34.82, 3.3},
  {"M8", "Nébuleuse de la Lagoon", "Nébuleuse", 18.06, -24.38, 6.0},
  {"M9", "Amas Globulaire", "Amas Globulaire", 17.32, -18.51, 7.7},
  {"M10", "Amas Globulaire", "Amas Globulaire", 16.98, -4.10, 6.4},
  {"M11", "Amas du Canard Sauvage", "Amas Ouvert", 18.86, -6.27, 5.8},
  {"M12", "Amas Globulaire", "Amas Globulaire", 16.79, -1.95, 6.7},
  {"M13", "Grand Amas d'Hercule", "Amas Globulaire", 16.69, 36.46, 5.8},
  {"M14", "Amas Globulaire", "Amas Globulaire", 17.63, -3.25, 7.6},
  {"M15", "Amas Globulaire", "Amas Globulaire", 21.50, 12.16, 6.2},
  {"M16", "Nébuleuse de l'Aigle", "Nébuleuse", 18.31, -13.79, 6.0},
  {"M17", "Nébuleuse Oméga", "Nébuleuse", 18.35, -16.18, 6.0},
  {"M18", "Amas Ouvert", "Amas Ouvert", 18.33, -17.05, 7.5},
  {"M19", "Amas Globulaire", "Amas Globulaire", 17.03, -26.27, 6.8},
  {"M20", "Nébuleuse Trifide", "Nébuleuse", 18.04, -23.03, 6.3},
  {"M21", "Amas Ouvert", "Amas Ouvert", 18.08, -22.50, 5.9},
  {"M22", "Amas Globulaire", "Amas Globulaire", 18.61, -23.90, 5.1},
  {"M23", "Amas Ouvert", "Amas Ouvert", 17.95, -19.01, 5.5},
  {"M24", "Nuage Stellaire", "Amas Ouvert", 18.28, -18.48, 4.6},
  {"M25", "Amas Ouvert", "Amas Ouvert", 18.53, -19.12, 4.6},
  {"M26", "Amas Ouvert", "Amas Ouvert", 18.76, -9.38, 8.0},
  {"M27", "Nébuleuse Dumbbell", "Nébuleuse", 19.96, 22.72, 7.4},
  {"M28", "Amas Globulaire", "Amas Globulaire", 18.41, -24.87, 6.8},
  {"M29", "Amas Ouvert", "Amas Ouvert", 20.40, 38.53, 7.1},
  {"M30", "Amas Globulaire", "Amas Globulaire", 21.67, -23.18, 7.2},
  {"M31", "Galaxie d'Andromède", "Galaxie", 0.71, 41.27, 3.4},
  {"M32", "Galaxie Satellite M31", "Galaxie", 0.71, 40.87, 8.1},
  {"M33", "Galaxie du Triangle", "Galaxie", 1.57, 30.66, 5.7},
  {"M34", "Amas Ouvert", "Amas Ouvert", 2.70, 42.78, 5.2},
  {"M35", "Amas Ouvert", "Amas Ouvert", 6.15, 24.33, 5.3},
  {"M36", "Amas Ouvert", "Amas Ouvert", 5.61, 34.14, 6.0},
  {"M37", "Amas Ouvert", "Amas Ouvert", 5.89, 32.55, 6.2},
  {"M38", "Amas Ouvert", "Amas Ouvert", 5.48, 35.84, 6.4},
  {"M39", "Amas Ouvert", "Amas Ouvert", 21.54, 48.43, 5.5},
  {"M40", "Étoile Double", "Étoile A", 12.38, 58.08, 8.4},
  {"M41", "Amas Ouvert", "Amas Ouvert", 6.78, -20.73, 4.5},
  {"M42", "Nébuleuse d'Orion", "Nébuleuse", 5.59, -5.39, 4.0},
  {"M43", "Nébuleuse de de Mairan", "Nébuleuse", 5.59, -5.26, 9.0},
  {"M44", "La Ruche / Praesepe", "Amas Ouvert", 8.67, 19.68, 3.7},
  {"M45", "Les Pléiades", "Amas Ouvert", 3.78, 24.12, 1.6},
  {"M46", "Amas Ouvert", "Amas Ouvert", 7.69, -14.83, 6.1},
  {"M47", "Amas Ouvert", "Amas Ouvert", 7.61, -14.50, 4.4},
  {"M48", "Amas Ouvert", "Amas Ouvert", 8.23, -5.80, 5.5},
  {"M49", "Galaxie Elliptique", "Galaxie", 12.50, 8.00, 8.4},
  {"M50", "Amas Ouvert", "Amas Ouvert", 7.05, -8.33, 5.9},
  {"M51", "Galaxie du Tourbillon", "Galaxie", 13.50, 47.20, 8.4},
  {"M52", "Amas Ouvert", "Amas Ouvert", 23.40, 61.58, 7.3},
  {"M53", "Amas Globulaire", "Amas Globulaire", 13.21, 18.17, 7.6},
  {"M54", "Amas Globulaire", "Amas Globulaire", 18.95, -30.48, 7.6},
  {"M55", "Amas Globulaire", "Amas Globulaire", 19.67, -30.96, 6.3},
  {"M56", "Amas Globulaire", "Amas Globulaire", 19.28, 30.18, 8.3},
  {"M57", "Nébuleuse de la Lyre", "Nébuleuse", 18.89, 33.03, 8.8},
  {"M58", "Galaxie Spirale", "Galaxie", 12.63, 11.82, 9.7},
  {"M59", "Galaxie Elliptique", "Galaxie", 12.70, 11.65, 9.6},
  {"M60", "Galaxie Elliptique", "Galaxie", 12.75, 11.55, 8.8},
  {"M61", "Galaxie Spirale", "Galaxie", 12.36, 4.47, 9.7},
  {"M62", "Amas Globulaire", "Amas Globulaire", 17.02, -30.11, 6.5},
  {"M63", "Galaxie du Tournesol", "Galaxie", 13.26, 42.03, 8.6},
  {"M64", "Galaxie de l'Œil Noir", "Galaxie", 12.95, 21.68, 8.5},
  {"M65", "Galaxie Spirale", "Galaxie", 11.31, 13.00, 9.3},
  {"M66", "Galaxie Spirale", "Galaxie", 11.34, 12.99, 8.9},
  {"M67", "Amas Ouvert", "Amas Ouvert", 8.85, 11.82, 6.1},
  {"M68", "Amas Globulaire", "Amas Globulaire", 12.66, -26.75, 7.8},
  {"M69", "Amas Globulaire", "Amas Globulaire", 18.52, -32.35, 7.6},
  {"M70", "Amas Globulaire", "Amas Globulaire", 18.73, -32.32, 7.9},
  {"M71", "Amas Globulaire", "Amas Globulaire", 19.90, 18.78, 8.2},
  {"M72", "Amas Globulaire", "Amas Globulaire", 20.91, -12.53, 9.3},
  {"M73", "Astérisme", "Amas Ouvert", 20.98, -12.50, 9.0},
  {"M74", "Galaxie Spirale", "Galaxie", 1.90, 15.78, 9.4},
  {"M75", "Amas Globulaire", "Amas Globulaire", 20.11, -21.92, 8.5},
  {"M76", "Nébuleuse de la Petite Dumbbell", "Nébuleuse", 1.70, 51.57, 10.1},
  {"M77", "Galaxie Spirale", "Galaxie", 2.71, -0.01, 8.9},
  {"M78", "Nébuleuse par Réflexion", "Nébuleuse", 5.76, 0.05, 8.3},
  {"M79", "Amas Globulaire", "Amas Globulaire", 5.40, -24.52, 7.7},
  {"M80", "Amas Globulaire", "Amas Globulaire", 16.28, -22.98, 7.3},
  {"M81", "Galaxie de Bode", "Galaxie", 9.96, 69.07, 6.9},
  {"M82", "Galaxie du Cigare", "Galaxie", 9.96, 69.68, 8.4},
  {"M83", "Galaxie du Moulinet Austral", "Galaxie", 13.62, -29.87, 7.5},
  {"M84", "Galaxie Lenticulaire", "Galaxie", 12.42, 12.97, 9.1},
  {"M85", "Galaxie Lenticulaire", "Galaxie", 12.44, 18.19, 9.1},
  {"M86", "Galaxie Lenticulaire", "Galaxie", 12.44, 12.95, 8.9},
  {"M87", "Galaxie Virgo A", "Galaxie", 12.51, 12.39, 8.6},
  {"M88", "Galaxie Spirale", "Galaxie", 12.53, 14.42, 9.6},
  {"M89", "Galaxie Elliptique", "Galaxie", 12.59, 12.56, 9.8},
  {"M90", "Galaxie Spirale", "Galaxie", 12.61, 13.24, 9.5},
  {"M91", "Galaxie Spirale", "Galaxie", 12.60, 14.50, 10.2},
  {"M92", "Amas Globulaire", "Amas Globulaire", 17.29, 43.14, 6.4},
  {"M93", "Amas Ouvert", "Amas Ouvert", 7.74, -23.87, 6.0},
  {"M94", "Galaxie Spirale", "Galaxie", 12.85, 41.14, 8.2},
  {"M95", "Galaxie Spirale", "Galaxie", 10.74, 11.70, 9.7},
  {"M96", "Galaxie Spirale", "Galaxie", 10.79, 11.82, 9.2},
  {"M97", "Nébuleuse du Hibou", "Nébuleuse", 11.25, 55.02, 9.9},
  {"M98", "Galaxie Spirale", "Galaxie", 12.23, 14.90, 10.1},
  {"M99", "Galaxie Spirale", "Galaxie", 12.31, 14.42, 9.9},
  {"M100", "Galaxie Spirale", "Galaxie", 12.38, 15.82, 9.3},
  {"M101", "Galaxie du Pinwheel", "Galaxie", 14.05, 54.35, 7.9},
  {"M102", "Galaxie du Fuseau", "Galaxie", 15.11, 55.75, 9.9},
  {"M103", "Amas Ouvert", "Amas Ouvert", 1.55, 60.70, 7.4},
  {"M104", "Galaxie du Sombrero", "Galaxie", 12.67, -11.62, 8.0},
  {"M105", "Galaxie Elliptique", "Galaxie", 10.79, 12.58, 9.3},
  {"M106", "Galaxie Spirale", "Galaxie", 12.31, 47.30, 8.4},
  {"M107", "Amas Globulaire", "Amas Globulaire", 16.55, -13.06, 7.9},
  {"M108", "Galaxie Spirale", "Galaxie", 11.19, 55.67, 10.0},
  {"M109", "Galaxie Spirale", "Galaxie", 11.96, 53.37, 9.8},
  {"M110", "Galaxie Satellite M31", "Galaxie", 0.67, 41.68, 8.5},


  // ==========================================
  // OBJETS NGC CÉLÈBRES
  // ==========================================
  {"NGC 869", "Double Amas de Persée (h)", "Amas Ouvert", 2.32, 57.14, 5.3},
  {"NGC 884", "Double Amas de Persée (χ)", "Amas Ouvert", 2.37, 57.15, 6.1},
  {"NGC 2237", "Nébuleuse de la Rosette", "Nébuleuse", 6.53, 5.03, 9.0},
  {"NGC 3372", "Nébuleuse de la Carène", "Nébuleuse", 10.75, -59.87, 1.0},
  {"NGC 5139", "Omega Centauri", "Amas Globulaire", 13.44, -47.48, 3.9},
  {"NGC 7293", "Nébuleuse de l'Hélice", "Nébuleuse", 22.48, -20.83, 7.3},
  {"NGC 6543", "Nébuleuse de l'Œil de Chat", "Nébuleuse", 17.98, 66.63, 8.1},
  {"NGC 6960", "Les Dentelles du Cygne", "Nébuleuse", 20.77, 30.70, 7.0},
  {"NGC 2403", "Galaxie Spirale", "Galaxie", 7.60, 65.60, 8.4},
  {"NGC 4565", "Galaxie de l'Aiguille", "Galaxie", 12.60, 25.98, 9.6},
  {"NGC 6888", "Nébuleuse du Croissant", "Nébuleuse", 20.20, 38.35, 7.4},
  {"Large Magellanic Cloud", "Grand Nuage de Magellan", "Galaxie", 5.40, -69.75, 0.9},
  {"Small Magellanic Cloud", "Petit Nuage de Magellan", "Galaxie", 0.85, -72.80, 2.7},
  {"NGC 253", "Galaxie du Sculpteur", "Galaxie", 0.79, -25.29, 7.1},
  {"NGC 457", "Amas de la Chouette (E.T.)", "Amas Ouvert", 1.48, 58.35, 6.4},
  {"NGC 1499", "Nébuleuse California", "Nébuleuse", 4.05, 36.43, 5.0},
  {"NGC 7000", "Nébuleuse de l'Amérique du Nord", "Nébuleuse", 20.99, 44.32, 4.0},
  {"NGC 7331", "Galaxie de Deerlick", "Galaxie", 22.62, 34.42, 9.5},
  {"NGC 7789", "Amas de la Rose Caroline", "Amas Ouvert", 23.95, 56.72, 6.7}


};

const size_t DSO_COUNT = sizeof(DSO_CATALOG) / sizeof(DSO_CATALOG[0]);

#endif