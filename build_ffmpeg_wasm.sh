#!/usr/bin/env bash
# FFmpeg WASM build script for krkrsdl2
# This script builds FFmpeg as a static library for Emscripten/WASM

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FFMPEG_DIR="${SCRIPT_DIR}/external/ffmpeg"
FFMPEG_SRC="${FFMPEG_DIR}/ffmpeg-6.1"
FFMPEG_BUILD="${FFMPEG_DIR}/build"
FFMPEG_INSTALL="${FFMPEG_DIR}/install"
ENABLE_GPL_CODECS="${KRKRSDL2_FFMPEG_ENABLE_GPL_CODECS:-0}"

# FFmpeg version
FFMPEG_VERSION="6.1"

echo "=== FFmpeg WASM Build Script ==="
echo "FFmpeg directory: ${FFMPEG_DIR}"
echo "FFmpeg source: ${FFMPEG_SRC}"

# Create directories
mkdir -p "${FFMPEG_DIR}"
mkdir -p "${FFMPEG_BUILD}"
mkdir -p "${FFMPEG_INSTALL}"

# Download FFmpeg if not exists
if [ ! -d "${FFMPEG_SRC}" ]; then
    echo "Downloading FFmpeg ${FFMPEG_VERSION}..."
    cd "${FFMPEG_DIR}"
    curl -L "https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.xz" -o "ffmpeg-${FFMPEG_VERSION}.tar.xz"
    tar xf "ffmpeg-${FFMPEG_VERSION}.tar.xz"
    rm "ffmpeg-${FFMPEG_VERSION}.tar.xz"
    echo "FFmpeg downloaded and extracted"
else
    echo "FFmpeg source already exists"
fi

# Configure FFmpeg for WASM
echo "Configuring FFmpeg for WASM..."
cd "${FFMPEG_SRC}"

# Clean previous build
make clean 2>/dev/null || true

if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
else
    JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
fi

# Configure with Emscripten. Default to LGPL-only. Some commercial KRKR
# packages use WMV/VC-1; enable those only with explicit confirmation:
# KRKRSDL2_FFMPEG_ENABLE_GPL_CODECS=1 ./build_ffmpeg_wasm.sh
COMMON_CONFIG=(
    --prefix="${FFMPEG_INSTALL}" \
    --enable-cross-compile \
    --target-os=none \
    --arch=x86 \
    --disable-runtime-cpudetect \
    --disable-asm \
    --disable-x86asm \
    --disable-inline-asm \
    --disable-stripping \
    --disable-programs \
    --disable-doc \
    --disable-debug \
    --disable-network \
    --disable-autodetect \
    --disable-everything \
    --enable-static \
    --disable-shared \
    --enable-avformat \
    --enable-avcodec \
    --enable-avutil \
    --enable-swresample \
    --enable-swscale \
    --enable-protocol=file \
    --enable-demuxer=ogg \
    --enable-demuxer=avi \
    --enable-demuxer=mov \
    --enable-demuxer=matroska \
    --enable-demuxer=wav \
    --enable-demuxer=mp3 \
    --enable-demuxer=flac \
    --enable-demuxer=aac \
    --enable-decoder=vorbis \
    --enable-decoder=opus \
    --enable-decoder=mp3 \
    --enable-decoder=flac \
    --enable-decoder=aac \
    --enable-decoder=pcm_s16le \
    --enable-decoder=pcm_s16be \
    --enable-decoder=pcm_f32le \
    --enable-decoder=pcm_f32be \
    --enable-parser=vorbis \
    --enable-parser=opus \
    --enable-parser=mpegaudio \
    --enable-parser=flac \
    --enable-parser=aac \
    --extra-cflags="-D__EMSCRIPTEN__" \
    --extra-cxxflags="-D__EMSCRIPTEN__" \
    --ar=emar \
    --cc=emcc \
    --cxx=em++ \
    --objcc=emcc \
    --dep-cc=emcc \
    --nm=emnm \
    --ranlib=emranlib \
    --strip=emstrip
)

GPL_CODEC_CONFIG=(
    --enable-gpl
    --enable-demuxer=asf
    --enable-decoder=wmav2
    --enable-decoder=wmav1
    --enable-decoder=wmapro
    --enable-decoder=wmalossless
    --enable-decoder=wmavoice
    --enable-decoder=wmv1
    --enable-decoder=wmv2
    --enable-decoder=wmv3
    --enable-decoder=wmv3image
    --enable-decoder=vc1
    --enable-decoder=msmpeg4v1
    --enable-decoder=msmpeg4v2
    --enable-decoder=msmpeg4v3
    --enable-parser=wmav2
    --enable-parser=wmv3
    --enable-parser=vc1
)

if [ "${ENABLE_GPL_CODECS}" = "1" ]; then
    echo "GPL codec mode: enabled for WMV/WMA/VC-1 experiments"
    emconfigure ./configure "${COMMON_CONFIG[@]}" "${GPL_CODEC_CONFIG[@]}"
else
    echo "GPL codec mode: disabled; using LGPL-first default"
    emconfigure ./configure "${COMMON_CONFIG[@]}"
fi

echo "FFmpeg configured"

# Build FFmpeg
echo "Building FFmpeg..."
cd "${FFMPEG_SRC}"
make -j"${JOBS}"

# Install FFmpeg
echo "Installing FFmpeg..."
make install

echo "=== FFmpeg WASM Build Complete ==="
echo "FFmpeg installed to: ${FFMPEG_INSTALL}"
echo ""
echo "Libraries built:"
ls -la "${FFMPEG_INSTALL}/lib/"
echo ""
echo "Include directories:"
ls -la "${FFMPEG_INSTALL}/include/"
