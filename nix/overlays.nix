{
  inputs,
  self,
  lib,
}:
let
  mkDate =
    longDate:
    (lib.concatStringsSep "-" [
      (builtins.substring 0 4 longDate)
      (builtins.substring 4 2 longDate)
      (builtins.substring 6 2 longDate)
    ]);
  date = mkDate (self.lastModifiedDate or "19700101");
  version = lib.removeSuffix "\n" (builtins.readFile ../VERSION);
in
{
  default = self.overlays.hyprsysteminfo;

  hyprsysteminfo-with-deps = lib.composeManyExtensions [
    inputs.aquamarine.overlays.default
    inputs.hyprgraphics.overlays.default
    inputs.hyprlang.overlays.default
    inputs.hyprutils.overlays.default
    inputs.hyprwayland-scanner.overlays.default
    inputs.hyprtoolkit.overlays.default
    inputs.hyprwire.overlays.default
    self.overlays.glaze
    self.overlays.hyprsysteminfo
  ];

  hyprsysteminfo = final: prev: {
    hyprsysteminfo = final.callPackage ./. {
      version = "${version}+date=${date}_${self.shortRev or "dirty"}";
      stdenv = final.gcc15Stdenv;
    };
  };

  # Even though glaze itself disables it by default, nixpkgs sets ENABLE_SSL set to true.
  # Since we don't include openssl, the build failes without the `enableSSL = false;` override
  glaze = final: prev: {
    glaze-hyprsysteminfo = prev.glaze.override {
      enableSSL = false;
      enableInterop = false;
    };
  };
}
