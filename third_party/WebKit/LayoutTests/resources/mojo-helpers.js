/*
 * mojo-helpers contains extensions to testharness.js useful for consuming
 * and mocking Mojo services directly within test code.
 */
'use strict';

// Fix up the global window.define, since all baked-in Mojo modules expect to
// find it there. This define() also returns a promise to the module.
function define(name, deps, factory) {
  return new Promise(resolve => {
    mojo.define(name, deps, (...modules) => {
      let result = factory(...modules);
      resolve(result);
      return result;
    });
  });
}

// Returns a promise to an object that exposes common Mojo module interfaces.
// Additional modules to load can be specified in the |modules| parameter. The
// result will contain them, in the same order, in the |modules| field.
function loadMojoModules(name, modules = []) {
  return define('Mojo layout test module: ' + name, [
      'mojo/public/js/core',
      'mojo/public/js/router',
      'mojo/public/js/support',
      'content/public/renderer/service_provider',
  ].concat(modules), (core, router, support, serviceProvider, ...rest) => {
    return {
      core: core,
      router: router,
      support: support,

      // |serviceProvider| is a bit of a misnomer. It should probably be
      // called |serviceRegistry|, so let's call it that here.
      serviceRegistry: serviceProvider,

      modules: rest,
    };
  });
}

function mojoTestCleanUp(mojo) {
  mojo.serviceRegistry.clearServiceOverridesForTesting();
}

// Runs a promise_test which depends on the Mojo system API modules available to
// all layout tests. The test implementation function is called with an Object
// that exposes common Mojo module interfaces.
function mojo_test(func, name, properties) {
  promise_test(() => loadMojoModules(name).then(mojo => {
    let result = Promise.resolve(func(mojo));
    let cleanUp = () => mojoTestCleanUp(mojo);
    result.then(cleanUp, cleanUp);
    return result;
  }), name, properties);
}

// Waits for a message to become available on a pipe.
function mojo_wait_for_incoming_message(mojo, pipe) {
  return new Promise((resolve, reject) => {
    mojo.support.asyncWait(pipe, mojo.core.HANDLE_SIGNAL_READABLE, result => {
      if (result != mojo.core.RESULT_OK) {
        reject(result);
        return;
      }
      let buffer, handles;
      ({ result, buffer, handles } = mojo.core.readMessage(pipe, 0));
      if (result !== mojo.core.RESULT_OK) {
        reject(result);
        return;
      }
      resolve({ buffer, handles });
    });
  });
};
