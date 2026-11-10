{
  lib,
  nix-gitignore,
  stdenv,

  cmake,
  pkg-config,
  glaze-hyprsysteminfo,
  hyprtoolkit,
  pciutils,
  version ? "0",
}:

stdenv.mkDerivation {
  pname = "hyprsysteminfo";
  inherit version;

  src = nix-gitignore.gitignoreSource [ ] ./..;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    glaze-hyprsysteminfo
    hyprtoolkit
    pciutils
  ];

  meta = {
    description = "System info utility for Hyprland";
    homepage = "https://github.com/hyprwm/hyprsysteminfo";
    license = lib.licenses.bsd3;
    maintainers = [ lib.maintainers.fufexan ];
    mainProgram = "hyprsysteminfo";
    platforms = lib.platforms.linux;
  };
}
