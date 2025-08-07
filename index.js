const nodejsWayland = require("./artifacts/nodejs_wayland");

nodejsWayland.initialize();

async function launchWaylandServer(readyCallback) {
    const args = ["weston", "--shell=kiosk", "--xwayland"];
    return await nodejsWayland.wetMain(args, readyCallback);
}

module.exports = {
    launchWaylandServer: launchWaylandServer,
};
