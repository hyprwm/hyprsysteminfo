{
  lib,
  stdenv,
  cmake,
  pkg-config,
  hyprutils,
  kdePackages,
  pciutils,
  qt6,
  version ? "0",
}: let
  inherit (lib.sources) cleanSource cleanSourceWith;
  inherit (lib.strings) hasSuffix makeBinPath;
in
  stdenv.mkDerivation {
    pname = "hyprsysteminfo";
    inherit version;

    src = cleanSourceWith {
      filter = name: _type: let
        baseName = baseNameOf (toString name);
      in
        ! (hasSuffix ".nix" baseName);
      src = cleanSource ../.;
    };

    nativeBuildInputs = [
      cmake
      pkg-config
      qt6.wrapQtAppsHook
    ];

    buildInputs = [
      hyprutils
      kdePackages.kirigami-addons
      qt6.qtbase
      qt6.qtsvg
      qt6.qtwayland
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
