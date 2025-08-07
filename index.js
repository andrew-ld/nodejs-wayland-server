import { createRequire } from "module";

const require = createRequire(import.meta.url);
const nodejsWayland = require("./artifacts/nodejs_wayland");

nodejsWayland.initialize();

export default nodejsWayland;
