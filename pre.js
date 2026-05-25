if (typeof self !== 'undefined') {
    self.alert = function(msg) { console.warn("[WASM Alert Blocked]:", msg); };
    self.confirm = function(msg) { console.warn("[WASM Confirm Blocked]:", msg); return false; };
    self.prompt = function(msg, def) { console.warn("[WASM Prompt Blocked]:", msg); return def || ""; };
}

if (typeof window !== 'undefined') {
    window.__originalErrorHandler = window.onerror;
    window.onerror = function(msg, url, line, col, error) {
        console.warn("[WASM Error Suppressed]:", msg, "at", url, ":", line);
        return true;
    };
}

if (typeof window !== 'undefined') {
    const originalWorker = window.Worker;
    const BASE_CHUNK_SIZE = 256 * 1024;
    const MAX_CHUNK_SIZE = 4 * 1024 * 1024;
    const MAX_CACHE_BYTES = 64 * 1024 * 1024;
    const lruCache = new Map();
    let lruCacheBytes = 0;
    const lruCacheStrategyBytes = [0, 0, 0, 0];
    const lruCacheUrlBytes = new Map();
    const krkrTraceRangeIO = (() => {
        try {
            const params = new URLSearchParams(window.location.search || "");
            const level = params.get("loglevel") || params.get("krkr_loglevel") || "1";
            return level === "3";
        } catch (e) {
            return false;
        }
    })();
    const krkrReadClassName = (strategy) => {
        switch (normalizeStrategy(strategy)) {
        case 1: return "psb-full";
        case 2: return "audio-stream";
        case 3: return "video-stream";
        default: return "small-random";
        }
    };
    const normalizeStrategy = (strategy) => {
        const s = strategy | 0;
        return s >= 0 && s <= 3 ? s : 0;
    };
    const clampChunkSize = (value) => {
        let size = Number(value) || BASE_CHUNK_SIZE;
        if (size < BASE_CHUNK_SIZE) size = BASE_CHUNK_SIZE;
        if (size > MAX_CHUNK_SIZE) size = MAX_CHUNK_SIZE;
        return size >>> 0;
    };
    const cacheBudgetForStrategy = (strategy) => {
        switch (normalizeStrategy(strategy)) {
        case 1: return 64 * 1024 * 1024; // PSB/raw full reads.
        case 2: return 32 * 1024 * 1024; // Audio stream working set.
        case 3: return 32 * 1024 * 1024; // Active video stream working set.
        default: return 8 * 1024 * 1024; // Fonts, index and sparse reads.
        }
    };
    const cachePerUrlBudgetForStrategy = (strategy) => {
        switch (normalizeStrategy(strategy)) {
        case 1: return 32 * 1024 * 1024; // Single PSB/raw logical item cap.
        case 2: return 8 * 1024 * 1024; // Per audio stream cap.
        case 3: return 32 * 1024 * 1024; // Current active video archive cap.
        default: return 8 * 1024 * 1024;
        }
    };
    const cacheItemKey = (strategy, url, itemKey) => {
        return normalizeStrategy(strategy) + "|" + String(url || "") + ">" + String(itemKey || "");
    };
    const cacheEntryBytes = (entry) => entry && entry.data ? entry.data.byteLength : 0;
    const addCacheCounters = (entry, sign) => {
        const bytes = cacheEntryBytes(entry) * sign;
        if (!bytes) return;
        const strategy = normalizeStrategy(entry.strategy);
        lruCacheBytes += bytes;
        lruCacheStrategyBytes[strategy] = Math.max(0, (lruCacheStrategyBytes[strategy] || 0) + bytes);
        const itemKey = cacheItemKey(strategy, entry.url, entry.itemKey);
        const nextItemBytes = (lruCacheUrlBytes.get(itemKey) || 0) + bytes;
        if (nextItemBytes > 0) lruCacheUrlBytes.set(itemKey, nextItemBytes);
        else lruCacheUrlBytes.delete(itemKey);
    };
    const deleteCacheEntry = (key, entry) => {
        if (!entry) entry = lruCache.get(key);
        if (!entry) return;
        lruCache.delete(key);
        addCacheCounters(entry, -1);
    };
    const countCacheBytes = (predicate) => {
        let total = 0;
        for (const entry of lruCache.values()) {
            if (predicate(entry)) total += cacheEntryBytes(entry);
        }
        return total;
    };
    const touchCacheEntry = (key, entry) => {
        lruCache.delete(key);
        lruCache.set(key, entry);
    };
    const evictOldestMatching = (predicate, bytesNow, target) => {
        let guard = lruCache.size + 1;
        while (bytesNow() > target && guard-- > 0) {
            let removed = false;
            for (const [key, entry] of lruCache) {
                if (!predicate(entry)) continue;
                deleteCacheEntry(key, entry);
                removed = true;
                break;
            }
            if (!removed) break;
        }
    };
    const evictRangeCache = (strategy, url, itemKey) => {
        const s = normalizeStrategy(strategy);
        const keyForItem = cacheItemKey(s, url, itemKey);
        if (s === 3) {
            evictOldestMatching(
                (entry) => normalizeStrategy(entry.strategy) === 3 && entry.itemKey !== itemKey,
                () => countCacheBytes((entry) => normalizeStrategy(entry.strategy) === 3 && entry.itemKey !== itemKey),
                0
            );
        }
        evictOldestMatching(
            (entry) => normalizeStrategy(entry.strategy) === s && entry.url === url && entry.itemKey === itemKey,
            () => lruCacheUrlBytes.get(keyForItem) || 0,
            cachePerUrlBudgetForStrategy(s)
        );
        evictOldestMatching(
            (entry) => normalizeStrategy(entry.strategy) === s,
            () => lruCacheStrategyBytes[s] || 0,
            cacheBudgetForStrategy(s)
        );
        evictOldestMatching(
            () => true,
            () => lruCacheBytes,
            MAX_CACHE_BYTES
        );
    };
    const addRangeCache = (key, data, strategy, chunkSize, url, itemKey) => {
        if (!data || data.byteLength <= 0) return;
        const entry = {
            data,
            strategy: normalizeStrategy(strategy),
            chunkSize: chunkSize >>> 0,
            url: String(url || ""),
            itemKey: String(itemKey || "")
        };
        if (lruCache.has(key)) {
            deleteCacheEntry(key);
        }
        lruCache.set(key, entry);
        addCacheCounters(entry, 1);
        evictRangeCache(entry.strategy, entry.url, entry.itemKey);
    };

    window.Worker = function(url, options) {
        const w = new originalWorker(url, options);
        w.addEventListener('message', (e) => {
            if (e.data && e.data.cmd === 'krkr_io_size') {
                e.stopImmediatePropagation();
                const sab = e.data.sab;
                const int32 = new Int32Array(sab);
                const float64 = new Float64Array(sab);
                fetch(e.data.url, { method: 'HEAD' }).then(res => {
                    if (res.ok) {
                        float64[1] = Number(res.headers.get('Content-Length'));
                        int32[0] = 1;
                    } else { int32[0] = -1; }
                    Atomics.notify(int32, 0, 1);
                }).catch(() => {
                    int32[0] = -1;
                    Atomics.notify(int32, 0, 1);
                });
            } else if (e.data && e.data.cmd === 'krkr_io_exist') {
                e.stopImmediatePropagation();
                const sab = e.data.sab;
                const int32 = new Int32Array(sab);
                fetch(e.data.url, { method: 'HEAD' }).then(res => {
                    if (res.ok) {
                        int32[0] = 1;
                    } else {
                        int32[0] = -1;
                    }
                    Atomics.notify(int32, 0, 1);
                }).catch(() => {
                    int32[0] = -1;
                    Atomics.notify(int32, 0, 1);
                });
            } else if (e.data && e.data.cmd === 'krkr_io_list') {
                e.stopImmediatePropagation();
                const sab = e.data.sab;
                const int32 = new Int32Array(sab);
                const uint8 = new Uint8Array(sab, 16);
                fetch(e.data.url)
                .then(res => {
                    if (res.ok) return res.text();
                    throw new Error("HTTP " + res.status);
                })
                .then(text => {
                    const encoder = new TextEncoder();
                    const encoded = encoder.encode(text);
                    const copyLen = Math.min(encoded.length, uint8.length - 1);
                    uint8.set(encoded.slice(0, copyLen));
                    uint8[copyLen] = 0;
                    int32[1] = copyLen;
                    int32[0] = 1;
                    Atomics.notify(int32, 0, 1);
                }).catch(() => {
                    int32[0] = -1;
                    Atomics.notify(int32, 0, 1);
                });
            } else if (e.data && e.data.cmd === 'krkr_io_read') {
                e.stopImmediatePropagation();
                const sab = e.data.sab;
                const int32 = new Int32Array(sab);
                const uint8 = new Uint8Array(sab, 16);
                const { offset, size, url } = e.data;
                const itemKey = String(e.data.itemKey || "");
                const strategy = e.data.strategy | 0;
                const chunkSize = clampChunkSize(e.data.chunkSize);
                const chunkIndex = Math.floor(offset / chunkSize);
                const offsetInChunk = offset % chunkSize;
                const isWithinChunk = (offset + size) <= (chunkIndex + 1) * chunkSize;

                if (isWithinChunk && size <= chunkSize) {
                    const key = url + '|' + chunkSize + '|' + chunkIndex;
                    if (lruCache.has(key)) {
                        const entry = lruCache.get(key);
                        touchCacheEntry(key, entry);
                        const chunkData = entry.data;
                        if (krkrTraceRangeIO) {
                            console.log(`[XP3-RANGE] cache=hit class=${krkrReadClassName(strategy)} offset=${offset} size=${size} chunkSize=${chunkSize} chunk=${chunkIndex} cacheBytes=${lruCacheBytes}`);
                        }
                        
                        const maxRead = Math.min(size, Math.max(0, chunkData.byteLength - offsetInChunk));
                        if (maxRead > 0) {
                            uint8.set(chunkData.subarray(offsetInChunk, offsetInChunk + maxRead));
                        }
                        
                        int32[1] = maxRead;
                        int32[0] = 1;
                        Atomics.notify(int32, 0, 1);
                    } else {
                        if (krkrTraceRangeIO) {
                            console.log(`[XP3-RANGE] cache=miss class=${krkrReadClassName(strategy)} offset=${offset} size=${size} chunkSize=${chunkSize} chunk=${chunkIndex}`);
                        }
                        const start = chunkIndex * chunkSize;
                        const end = (chunkIndex + 1) * chunkSize - 1;
                        fetch(url, { headers: { Range: `bytes=${start}-${end}` } })
                        .then(res => {
                            if (res.status === 206 || res.status === 200) return res.arrayBuffer();
                            throw new Error("HTTP " + res.status);
                        })
                        .then(buf => {
                            const chunkData = new Uint8Array(buf);
                            addRangeCache(key, chunkData, strategy, chunkSize, url, itemKey);
                            
                            const maxRead = Math.min(size, Math.max(0, chunkData.byteLength - offsetInChunk));
                            if (maxRead > 0) {
                                uint8.set(chunkData.subarray(offsetInChunk, offsetInChunk + maxRead));
                            }
                            
                            int32[1] = maxRead;
                            int32[0] = 1;
                            Atomics.notify(int32, 0, 1);
                        })
                        .catch(err => {
                            console.error("[Network Fetch Error]", err);
                            int32[0] = -1;
                            Atomics.notify(int32, 0, 1);
                        });
                    }
                } else {
                    if (krkrTraceRangeIO) {
                        console.log(`[XP3-RANGE] cache=raw class=${krkrReadClassName(strategy)} offset=${offset} size=${size} chunkSize=${chunkSize}`);
                    }
                    fetch(url, { headers: { Range: 'bytes=' + offset + '-' + (offset + size - 1) } })
                    .then(res => {
                        if (res.status === 206 || res.status === 200) return res.arrayBuffer();
                        throw new Error("HTTP " + res.status);
                    })
                    .then(buf => {
                        const data = new Uint8Array(buf);
                        const copyLen = Math.min(data.byteLength, uint8.length, size);
                        if (copyLen > 0) {
                            uint8.set(data.subarray(0, copyLen));
                        }
                        int32[1] = copyLen;
                        int32[0] = 1;
                        Atomics.notify(int32, 0, 1);
                    }).catch((err) => {
                        console.error("[Network Fetch Error]", err);
                        int32[0] = -1;
                        Atomics.notify(int32, 0, 1);
                    });
                }
            }
        });
        return w;
    };
}
