with import <nixpkgs> {
  crossSystem = rec {
    libc = "musl";
    system = "mipsel-linux-musl";
    openssl.system = "linux-generic32";
    withTLS = true;
    platform = {
      endian = "little";
      kernelArch = "mips";
      gcc = { abi = "32"; } ;
      bfdEmulation = "elf32ltsmip";
    };
  };
};
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
