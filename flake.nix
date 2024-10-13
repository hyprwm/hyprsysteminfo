{
  description = "A tiny qt6/qml application to display information about the running system";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default-linux";

    hyprutils = {
      url = "github:hyprwm/hyprutils";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.systems.follows = "systems";
    };
  };

  outputs = {
    self,
    nixpkgs,
    systems,
    ...
  } @ inputs: let
    inherit (nixpkgs) lib;
    eachSystem = lib.genAttrs (import systems);
    pkgsFor = eachSystem (
      system:
        import nixpkgs {
          localSystem = system;
          overlays = [self.overlays.default];
        }
    );
  in {
    overlays = import ./nix/overlays.nix {inherit inputs self lib;};

    packages = eachSystem (system: {
      default = self.packages.${system}.hyprsysteminfo;
      inherit (pkgsFor.${system}) hyprsysteminfo;
    });

    devShells = eachSystem (system: {
      default = import ./nix/shell.nix {
        pkgs = pkgsFor.${system};
        inherit (pkgsFor.${system}) hyprsysteminfo;
      };
    });
  };
}
