#!/usr/bin/env bash
readonly PWD0="$(dirname "$(readlink -fn "$0")")"
readonly MYFRAME_ROOT="$(dirname ${PWD0})"

readonly IMAGE_NAME="myframe"

set -e

function main {
  cd "${PWD0}"

  local -a cmd=()
  if command -v docker; then
    cmd+=("docker" "build")
  else
    echo "No docker existed!"
    exit 1
  fi

  local arc="$(uname -m)"
  local cpu_arch="unknown"
  case "${arc}" in
  "aarch64" | "arm64")
    arc="aarch64"
    cpu_arch="arm"
    ;;
  "x86_64" | "amd64")
    arc="amd64"
    cpu_arch="x86"
    ;;
  *)
    echo "Unknown arch: ${arc}"
    exit 1
    ;;
  esac

  local tag="${arc}-$1"

  "${cmd[@]}" \
    --build-arg myframe_version=$1 \
    --build-arg cpu_arch=${cpu_arch} \
    --ulimit nofile=102400:102400 \
    -f "dev.dockerfile" \
    -t "${IMAGE_NAME}:${tag}" \
    "${MYFRAME_ROOT}"
}

if [ "$#" -ne 1 ]; then
    echo "Error: No arguments provided. Usage: $0 \"myframe_version\""
    exit 1
fi

main "$@"
