{ targetBoard ? "mt300n_v2" }:
let nixwrt = (import ../nixwrt/nixwrt/default.nix) { inherit targetBoard; }; in
with nixwrt.nixpkgs;
{
  kexectools = pkgs.kexectools.overrideAttrs (o: {
    nativeBuildInputs = [pkgs.autoreconfHook pkgs.autoconf] ++ o.nativeBuildInputs;
    preConfigure = [ ''
        aclocal -I config
        autoheader
        autoconf
    ''];
    src = ./.;
    buildInputs = o.buildInputs ++ [pkgs.xz];
  });
}
