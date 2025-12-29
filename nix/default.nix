{
  lib,
  nix-gitignore,
  stdenv,

  cmake,
  pkg-config,
  aquamarine,
  cairo,
  glaze-hyprsysteminfo,
  hyprgraphics,
  hyprtoolkit,
  hyprutils,
  libdrm,
  pciutils,
  pixman,
  version ? "0",
}:

stdenv.mkDerivation {
  pname = "hyprsysteminfo";
  inherit version;

  src = nix-gitignore.gitignoreSource [] ./..;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    aquamarine
    cairo
    glaze-hyprsysteminfo
    hyprgraphics
    hyprtoolkit
    hyprutils
    libdrm
    pciutils
    pixman
  ];

  meta = {
    description = "System info utility for Hyprland";
    homepage = "https://github.com/hyprwm/hyprsysteminfo";
    license = lib.licenses.bsd3;
    maintainers = [lib.maintainers.fufexan];
    mainProgram = "hyprsysteminfo";
    platforms = lib.platforms.linux;
  };
}
