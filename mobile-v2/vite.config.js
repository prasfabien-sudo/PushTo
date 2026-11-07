import { readdirSync } from "node:fs";
import { resolve } from "node:path";
import { defineConfig } from "vite";

const pagesDirectory = resolve("src");
const inputs = Object.fromEntries(
  readdirSync(pagesDirectory)
    .filter((file) => file.endsWith(".html"))
    .map((file) => [file.replace(/\.html$/, ""), resolve(pagesDirectory, file)])
);

export default defineConfig({
  root: "src",
  publicDir: resolve("public"),
  build: {
    outDir: resolve("www"),
    emptyOutDir: true,
    rollupOptions: {
      input: inputs,
      output: {
        entryFileNames: "assets/[name].js",
        chunkFileNames: "assets/[name]-[hash].js",
        assetFileNames: "assets/[name]-[hash][extname]"
      }
    }
  }
});
