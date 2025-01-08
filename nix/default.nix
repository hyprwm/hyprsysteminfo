{
  lib,
  nix-gitignore,
  stdenv,

  cmake,
  qt6,
  pkg-config,
  hyprutils,
  pciutils,
  hyprland-qt-support,
  version ? "0",
}: let
  inherit (lib.strings) makeBinPath;
in
  stdenv.mkDerivation {
    pname = "hyprsysteminfo";
    inherit version;

    src = nix-gitignore.gitignoreSource [] ./..;

    nativeBuildInputs = [
      cmake
      pkg-config
      qt6.wrapQtAppsHook
    ];

    buildInputs = [
      qt6.qtbase
      qt6.qtdeclarative
      qt6.qtsvg
      qt6.qtwayland
      hyprutils
      hyprland-qt-support
    ];

    preFixup = ''
      qtWrapperArgs+=(--prefix PATH : "${makeBinPath [pciutils]}")
    '';

    meta = {
      description = "A tiny qt6/qml application to display information about the running system";
      homepage = "https://github.com/hyprwm/hyprsysteminfo";
      license = lib.licenses.bsd3;
      maintainers = [lib.maintainers.fufexan];
      mainProgram = "hyprsysteminfo";
      platforms = lib.platforms.linux;
    };
  }
