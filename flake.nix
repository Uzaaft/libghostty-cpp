{
  description = "C++17 CMake scaffold for libghostty-cpp";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/release-25.11";
    flake-utils.url = "github:numtide/flake-utils";
    zig = {
      url = "github:mitchellh/zig-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      zig,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;
        zigPackage = zig.packages.${system}."0.15.2";
        ghosttySource = pkgs.fetchFromGitHub {
          owner = "ghostty-org";
          repo = "ghostty";
          rev = "cb28160b5a2fd32d2e1cfeefb01d4297dbca8b18";
          hash = "sha256-z7SOqGqEv0waIDLtVITjpkO2ypkgKhus+V6uIlvSo68=";
        };
        ghosttyDeps = pkgs.callPackage "${ghosttySource}/build.zig.zon.nix" {
          name = "libghostty-vt-cache-cb28160b5a2fd32d2e1cfeefb01d4297dbca8b18";
          linkFarm =
            name: entries:
            pkgs.runCommand name { } ''
              mkdir -p $out
              ${lib.concatMapStringsSep "\n" (entry: ''
                cp -rL ${entry.path} $out/${entry.name}
              '') entries}
            '';
        };
        linuxBuildInputs = lib.optionals pkgs.stdenv.hostPlatform.isLinux [
          pkgs.xorg.libX11
          pkgs.xorg.libXcursor
          pkgs.xorg.libXrandr
          pkgs.xorg.libXinerama
          pkgs.xorg.libXi
          pkgs.libGL
          pkgs.libxkbcommon
          pkgs.wayland
        ];
        libghosttyCppPackage =
          {
            buildGhostling,
          }:
          pkgs.stdenv.mkDerivation {
            pname = if buildGhostling then "ghostling-cpp" else "libghostty-cpp";
            version = "0-unstable-2026-05-25";
            src = self;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.pkg-config
              zigPackage
            ]
            ++ lib.optionals buildGhostling [ pkgs.qt6.wrapQtAppsHook ];

            buildInputs =
              linuxBuildInputs
              ++ lib.optionals buildGhostling [
                pkgs.qt6.qtbase
                pkgs.qt6.qtwayland
              ];

            preConfigure = ''
              export ZIG_GLOBAL_CACHE_DIR=$TMPDIR/zig-global-cache
              export ZIG_LOCAL_CACHE_DIR=$TMPDIR/zig-local-cache
              mkdir -p "$ZIG_GLOBAL_CACHE_DIR" "$ZIG_LOCAL_CACHE_DIR"

              cp -rL ${ghosttySource} ghostty-src
              chmod -R u+w ghostty-src
              cmakeFlagsArray+=("-DFETCHCONTENT_SOURCE_DIR_GHOSTTY=$PWD/ghostty-src")
            '';

            cmakeFlags = [
              "-DCMAKE_INSTALL_BINDIR=bin"
              "-DCMAKE_INSTALL_INCLUDEDIR=include"
              "-DCMAKE_INSTALL_LIBDIR=lib"
              "-DGHOSTTY_ZIG_BUILD_FLAGS=--system;${ghosttyDeps};-Dcpu=baseline;-Dapp-runtime=none;-Dsimd=false"
              "-DLIBGHOSTTY_CPP_BUILD_GHOSTLING_CPP=${if buildGhostling then "ON" else "OFF"}"
              "-DBUILD_TESTING=OFF"
            ];

            meta = {
              homepage = "https://github.com/uzaaft/libghostty-cpp";
              platforms = lib.platforms.unix;
            }
            // lib.optionalAttrs buildGhostling {
              mainProgram = "ghostling_cpp";
            };
          };
      in
      {
        formatter = pkgs.nixfmt-tree;

        packages = rec {
          libghostty-cpp = libghosttyCppPackage { buildGhostling = false; };
          ghostling = libghosttyCppPackage { buildGhostling = true; };
          default = libghostty-cpp;
        };

        apps = rec {
          ghostling = flake-utils.lib.mkApp {
            drv = self.packages.${system}.ghostling;
            exePath = "/bin/ghostling_cpp";
          };
          default = ghostling;
        };

        devShells.default = pkgs.mkShell {
          packages = [
            zigPackage
            pkgs.clang-tools
            pkgs.cmake
            pkgs.ninja
            pkgs.pinact
            pkgs.pkg-config
            pkgs.qt6.qtbase
            pkgs.qt6.qtwayland
            pkgs.scc
          ]
          ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isLinux [
            pkgs.xorg.libX11
            pkgs.xorg.libXcursor
            pkgs.xorg.libXrandr
            pkgs.xorg.libXinerama
            pkgs.xorg.libXi
            pkgs.libGL
            pkgs.libxkbcommon
            pkgs.wayland
          ];

          # Unset Nix Darwin SDK env vars and remove xcbuild
          # xcrun wrapper so Zig's SDK detection uses the real
          # system xcrun/xcode-select
          shellHook = ''
            export CMAKE_GENERATOR=Ninja
            unset SDKROOT
            unset DEVELOPER_DIR
            export PATH=$(echo "$PATH" | tr ':' '\n' | grep -v -e xcbuild -e apple-sdk | tr '\n' ':')
          '';
        };
      }
    );
}
