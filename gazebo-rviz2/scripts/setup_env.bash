# Source this file from Bash or Zsh; it also works before the first build.
if [ -n "${ZSH_VERSION:-}" ]; then
  _rival_shell=zsh
  _rival_script="${(%):-%N}"
elif [ -n "${BASH_VERSION:-}" ]; then
  _rival_shell=bash
  _rival_script="${BASH_SOURCE[0]}"
else
  printf 'Gunakan Bash atau Zsh untuk source skrip ini.\n'
  return 1
fi
_rival_root="$(cd "$(dirname "$_rival_script")/.." && pwd)"
_rival_ubuntu="$(. /etc/os-release; printf '%s' "$VERSION_ID")"
case "$_rival_ubuntu" in
  22.04) _rival_ros=humble ;;
  24.04) _rival_ros=jazzy ;;
  *) printf 'Ubuntu %s belum menjadi target modul. Gunakan Ubuntu 22.04 atau 24.04.\n' "$_rival_ubuntu"; return 1 ;;
esac
if [ -n "${ROS_DISTRO:-}" ] && [ "$ROS_DISTRO" != "$_rival_ros" ]; then
  printf 'Terminal memuat ROS %s. Buka terminal Bash baru untuk ROS %s.\n' "$ROS_DISTRO" "$_rival_ros"
  return 1
fi
if [ ! -f "/opt/ros/$_rival_ros/setup.$_rival_shell" ]; then
  printf 'ROS 2 %s belum terpasang. Ikuti lampiran persiapan.\n' "$_rival_ros"
  return 1
fi
source "/opt/ros/$_rival_ros/setup.$_rival_shell" || return 1
export ROS_LOCALHOST_ONLY=1
export ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-42}"
export GZ_VERSION=harmonic
if [ -f "$_rival_root/template_ws/install/setup.$_rival_shell" ]; then
  source "$_rival_root/template_ws/install/setup.$_rival_shell" || return 1
fi
unset _rival_root _rival_ubuntu _rival_ros _rival_shell _rival_script
