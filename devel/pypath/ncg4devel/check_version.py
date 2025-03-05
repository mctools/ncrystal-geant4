def discard_whitespace( s ):
    return ''.join(s.strip().split())

def extract_bruteforce(f,linebegin,lineend):
    linebegin = discard_whitespace(linebegin)
    lineend = discard_whitespace(lineend)
    for line in f.read_text().splitlines():
        s = discard_whitespace(line)
        if not s.startswith(linebegin):
            continue
        assert s.endswith(lineend)
        return s[len(linebegin):-len(lineend)].strip()
    assert False

def extract_cmake(reporoot):
    f = reporoot.joinpath('src','ncrystal_geant4',
                          'cmake','NCrystalGeant4ConfigVersion.cmake')
    return extract_bruteforce(f,'set(PACKAGE_VERSION',')')

def main( reporoot ):
    print("Checking versions...")
    versions = [
        ('VERSION','',''),
        ('src/ncrystal_geant4/cmake/NCrystalGeant4ConfigVersion.cmake',
         'set(PACKAGE_VERSION "','")'),
        ('pyproject.toml','version = "','"'),
        ('src/ncrystal_geant4/__init__.py',"__version__ = '","'")
    ]
    actual_version = None
    for frel, linebegin, lineend in versions:
        print(f"Extracting version from {frel}")
        f = reporoot.joinpath(frel)
        if not linebegin and not lineend:
            v = f.read_text().strip()
        else:
            v = extract_bruteforce(f,linebegin,lineend)
        print(f"  .. got {repr(v)}")
        assert v
        if not actual_version:
            actual_version = v
        elif actual_version != v:
            raise SystemExit('ERROR: Version mismatch!')
    print("Checking versions... DONE")
