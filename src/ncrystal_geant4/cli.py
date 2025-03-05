def parse_args():
    from argparse import ArgumentParser, RawTextHelpFormatter
    import textwrap
    def wrap(t,w=59):
        return textwrap.fill( ' '.join(t.split()), width=w )

    descr = """Get information about NCrystalGeant4 installation."""
    parser = ArgumentParser( description=wrap(descr,79),
                             formatter_class = RawTextHelpFormatter )
    from . import __version__ as progversion
    parser.add_argument('--version', action='version', version=progversion)

    parser.add_argument('--cmakedir', action='store_true',
                        help=wrap(
                            """Print the directory in which
                            NCrystalGeant4Config.cmake resides. To make a CMake
                            project with find_package(NCrystalGeant4) work, the
                            printed directory must either be added to the
                            CMAKE_PREFIX_PATH, or the variable
                            NCrystalGeant4_DIR can be set to the value.""" )
                        )
    parser.add_argument('--includedir', action='store_true',
                        help=wrap(
                            """Print the directory in which NCrystalGeant4
                            header files reside (for advanced users wishing to
                            modify include paths manually).""" )
                        )
    parser.add_argument('--srcdir', action='store_true',
                        help=wrap(
                            """Print the directory in which NCrystalGeant4
                            source files reside (for advanced users wishing to
                            process them manually).""" )
                        )

    args = parser.parse_args()

    nselect = sum( (1 if e else 0)
                   for e in (args.cmakedir,args.includedir,args.srcdir) )
    if nselect == 0:
        parser.error('Invalid usage. Run with -h/--help for instructions.')
    if nselect > 1:
        parser.error('Conflicting options')
    return args

def main():
    import pathlib
    cmakedir = pathlib.Path(__file__).parent.joinpath('cmake').absolute()
    args = parse_args()
    if args.cmakedir:
        print( cmakedir )
    elif args.includedir:
        print( cmakedir.joinpath('include') )
    elif args.srcdir:
        print( cmakedir.joinpath('src') )
    else:
        assert False, "Implementation error"
