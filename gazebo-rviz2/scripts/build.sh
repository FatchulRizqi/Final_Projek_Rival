#!/usr/bin/env bash
set -eo pipefail
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$root_dir/scripts/setup_env.bash"
cd "$root_dir/template_ws"
colcon build --symlink-install --cmake-args -DBUILD_TESTING=OFF
printf '\nBuild selesai. Di setiap terminal praktik (Bash/Zsh), jalankan:\nsource "%s/scripts/setup_env.bash"\n' "$root_dir"
