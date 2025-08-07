import { createRequire } from "module";

const require = createRequire(import.meta.url);
const nodejsWayland = require("./artifacts/nodejs_wayland.node");

nodejsWayland.initialize();

async function test() {
  await nodejsWayland.wetMain(["server", "--shell=kiosk", "--xwayland"]);
  console.log(1)
}

test().catch(console.error);
