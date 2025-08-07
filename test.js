const server = require("./index.js");
const { spawn } = require("node:child_process");

server
  .launchWaylandServer(() => {
    spawn("glxgears");
  })
  .then((code) => console.log("exit:", code))
  .catch((error) => console.error("error:", error));
