{
  description = "C++17 CMake scaffold for libghostty-cpp";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";

  outputs =
    { self, nixpkgs }:
    let
      lib = nixpkgs.lib;
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forEachSupportedSystem =
        f:
        lib.genAttrs supportedSystems (
          system:
          f {
            inherit system;
            pkgs = import nixpkgs { inherit system; };
          }
        );
    in
    {
      formatter = forEachSupportedSystem ({ pkgs, ... }: pkgs.nixfmt-tree);

      packages = forEachSupportedSystem (
        { pkgs, ... }:
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "libghostty-cpp";
            version = "0.1.0";
            src = lib.cleanSource ./.;

            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              pkg-config
            ];

            cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];
          };
        }
      );

      devShells = forEachSupportedSystem (
        { pkgs, system, ... }:
        {
          default = pkgs.mkShell {
            packages =
              (with pkgs; [
                clang-tools
                cmake
                ninja
                pkg-config
                stdenv.cc
                self.formatter.${system}
              ])
              ++ lib.optionals pkgs.stdenv.isLinux [ pkgs.gdb ]
              ++ lib.optionals pkgs.stdenv.isDarwin [ pkgs.lldb ];

            env = {
              CMAKE_BUILD_PARALLEL_LEVEL = "8";
              CTEST_OUTPUT_ON_FAILURE = "1";
            };

            shellHook = ''
              export CMAKE_GENERATOR=Ninja
            '';
          };
        }
      );

      checks = forEachSupportedSystem (
        { system, ... }:
        {
          default = self.packages.${system}.default;
        }
      );
    };
}
