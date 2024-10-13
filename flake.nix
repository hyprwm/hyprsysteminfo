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
      default = pkgsFor.${system}.mkShell {
        inputsFrom = [self.packages.${system}.hyprsysteminfo];

        shellHook = ''
          # Generate compile_commands.json
          CMAKE_EXPORT_COMPILE_COMMANDS=1 cmake -S . -B ./build
          ln -s build/compile_commands.json .
        '';
      };
    });
  };
}
