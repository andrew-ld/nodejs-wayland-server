import { createRequire } from 'module';

const require = createRequire(import.meta.url);
const nodejsWayland = require('./artifacts/nodejs_wayland.node');

nodejsWayland.initialize()
nodejsWayland.wetMain(["server", "--shell=kiosk", "--xwayland"])
