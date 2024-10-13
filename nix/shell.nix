{
  pkgs ? import <nixpkgs> {},
  hyprsysteminfo ? pkgs.callPackage ./default.nix {},
  ...
}: pkgs.mkShell {
  inputsFrom = [ hyprsysteminfo ];
  nativeBuildInputs = [ pkgs.clang-tools pkgs.pciutils ];

  shellHook = let
    inherit (pkgs.lib.strings) concatMapStringsSep;
    qtLibPath = f: concatMapStringsSep ":" f (with pkgs.qt6; [
      qtbase
      qtdeclarative
      qtwayland
      pkgs.kdePackages.qqc2-desktop-style
      # see https://github.com/NixOS/nixpkgs/blob/e186dd1a34be7a76c6e2c038ab4b759faec32d5d/pkgs/kde/frameworks/kirigami/default.nix#L9-L11
      pkgs.kdePackages.kirigami.unwrapped
    ]);
  in ''
    # Add Qt-related environment variables.
    export QT_PLUGIN_PATH=${qtLibPath (p: "${p}/lib/qt-6/plugins")}
    export QML2_IMPORT_PATH=${qtLibPath (p: "${p}/lib/qt-6/qml")}

    # Generate compile_commands.json
    CMAKE_EXPORT_COMPILE_COMMANDS=1 cmake -S . -B ./build
    ln -s build/compile_commands.json .
  '';
}
