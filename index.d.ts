/**
 * A callback function that is executed when the Weston server
 * is initialized and ready to accept client connections.
 * This function takes no arguments and returns nothing.
 */
type ReadyCallback = () => void;

/**
 * Describes the object exported by the module.
 */
declare const WaylandServerInterface: {
  /**
   * Launches and runs the Weston Wayland compositor in the background.
   *
   * This function returns a promise that resolves with the exit code when the compositor
   * process eventually shuts down.
   *
   * @param readyCallback An optional callback function that is executed when the Weston
   *                      server is initialized and ready to accept clients.
   * @returns A promise that resolves with the Weston process's integer exit code upon termination.
   * @example
   * const wayland = require('./my-wayland-module');
   *
   * console.log('Starting server...');
   * wayland.launchWaylandServer(() => {
   *   console.log('✅ Server is ready!');
   * }).then(exitCode => {
   *   console.log(`Server exited with code: ${exitCode}`);
   * }).catch(err => {
   *   console.error('Server failed to start or an error occurred:', err);
   * });
   */
  launchWaylandServer(readyCallback: ReadyCallback): Promise<number>;
};


export = WaylandServerInterface;
