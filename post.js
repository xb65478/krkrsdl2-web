// post.js: Patch all invoke_* functions to catch WebAssembly.RuntimeError
// This prevents WASM memory access traps from killing the worker thread.
// The engine can survive use-after-free crashes in the rendering pipeline.
(function() {
    var _originals = {};
    function patchInvoke(name) {
        if (typeof globalThis[name] !== 'function') return;
        _originals[name] = globalThis[name];
        globalThis[name] = function() {
            try {
                return _originals[name].apply(this, arguments);
            } catch(e) {
                if (e instanceof WebAssembly.RuntimeError) {
                    console.warn('[WASM Trap Caught] ' + name + ': ' + e.message);
                    stackRestore(stackSave());
                    if (typeof _setThrew === 'function') _setThrew(1, 0);
                    return 0;
                }
                throw e;
            }
        };
    }
    // Patch after module initialization when invoke_* are in global scope
    var _origOnReady = Module['onRuntimeInitialized'];
    Module['onRuntimeInitialized'] = function() {
        if (_origOnReady) _origOnReady();
        var keys = Object.getOwnPropertyNames(globalThis);
        for (var i = 0; i < keys.length; i++) {
            if (keys[i].indexOf('invoke_') === 0) {
                patchInvoke(keys[i]);
            }
        }
    };
})();
