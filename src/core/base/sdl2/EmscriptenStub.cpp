// Stub implementations for Emscripten HTTP functions when building for non-Emscripten targets
#ifndef __EMSCRIPTEN__

extern "C" {

int emscripten_sync_http_check_exist(const char* url_ptr) {
    // HTTP not available, return 0 (does not exist)
    return 0;
}

int emscripten_sync_http_list(const char* url_ptr, char* out_buf, int max_len) {
    // Return empty list
    if (max_len > 0) out_buf[0] = '\0';
    return 0;
}

int emscripten_sync_http_read(const char* url_ptr, uint32_t offset, uint32_t size, void* buffer_ptr, uint32_t chunk_size, int strategy, const char* item_key_ptr) {
    // Cannot read, return 0 bytes read
    return 0;
}

uint64_t emscripten_sync_http_get_size(const char* url_ptr) {
    // Unknown size
    return 0;
}

} // extern "C"

#endif // !__EMSCRIPTEN__
