#!/usr/bin/env bash
# Default is read-only. --install is explicit authorization to apt dependencies.
set -eo pipefail
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="${1:---check}"
if [[ "$mode" != --check && "$mode" != --install ]]; then
  printf 'Pemakaian: bash scripts/install_dependencies.sh [--check|--install]\n'; exit 2
fi
source /etc/os-release
case "$VERSION_ID" in
  22.04) distro=humble; integration=ros-humble-ros-gzharmonic ;;
  24.04) distro=jazzy; integration=ros-jazzy-ros-gz ;;
  *) printf 'Target modul: Ubuntu 22.04 atau 24.04, ditemukan %s.\n' "$VERSION_ID"; exit 1 ;;
esac
if [[ ! -f "/opt/ros/$distro/setup.bash" ]]; then
  printf 'Pasang ROS 2 %s terlebih dahulu dari panduan resmi ROS.\n' "$distro"; exit 1
fi
packages=("$integration" "ros-$distro-rviz2" "ros-$distro-robot-state-publisher"
          "ros-$distro-teleop-twist-keyboard" "ros-$distro-tf2-ros"
          python3-colcon-common-extensions python3-yaml)
if [[ "$mode" == --install ]]; then
  if [[ "$distro" == humble ]] && ! apt-cache show "$integration" >/dev/null 2>&1; then
    command -v curl >/dev/null || { printf 'Pasang curl terlebih dahulu.\n'; exit 1; }
    tmp_key="$(mktemp)"
    trap 'rm -f "$tmp_key"' EXIT
    curl --fail --location --proto '=https' --tlsv1.2 \
      https://packages.osrfoundation.org/gazebo.gpg --output "$tmp_key"
    sudo install -m 0644 "$tmp_key" /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
    printf 'deb [arch=%s signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] https://packages.osrfoundation.org/gazebo/ubuntu-stable %s main\n' \
      "$(dpkg --print-architecture)" "$VERSION_CODENAME" | sudo tee /etc/apt/sources.list.d/rival-gazebo-stable.list >/dev/null
  fi
  sudo apt-get update
  printf '\nRencana perubahan paket (termasuk paket yang akan diganti):\n'
  apt-get --simulate install "${packages[@]}"
  # apt asks for confirmation. Never remove packages through a wildcard/autoremove.
  sudo apt-get install "${packages[@]}"
fi
source "$root_dir/scripts/setup_env.bash"
missing=0
for pkg in ros_gz_bridge ros_gz_sim rviz2 robot_state_publisher teleop_twist_keyboard tf2_ros; do
  if ros2 pkg prefix "$pkg" >/dev/null 2>&1; then
    printf '[OK] %s\n' "$pkg"
  else
    printf '[BELUM] %s\n' "$pkg"; missing=1
  fi
done
if command -v gz >/dev/null && gz sim --versions 2>/dev/null | grep -Eq '(^|[[:space:]])8\.'; then
  printf '[OK] Gazebo Harmonic (gz-sim 8)\n'
else
  printf '[BELUM] Gazebo Harmonic\n'; missing=1
fi
if [[ "$distro" == humble ]] && ! dpkg-query -W -f='${db:Status-Abbrev}' ros-humble-ros-gzharmonic-bridge 2>/dev/null | grep -q '^ii'; then
  printf '[BELUM] Bridge khusus Humble-Harmonic; gunakan --install.\n'; missing=1
fi
if command -v colcon >/dev/null; then printf '[OK] colcon\n'; else printf '[BELUM] colcon\n'; missing=1; fi
printf 'ROS=%s | Ubuntu=%s | DISPLAY=%s\n' "$distro" "$VERSION_ID" "${DISPLAY:-belum tersedia}"
exit "$missing"
