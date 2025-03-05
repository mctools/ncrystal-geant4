
def parse_args():
    from argparse import ArgumentParser, RawTextHelpFormatter
    import textwrap
    def wrap(t,w=59):
        return textwrap.fill( ' '.join(t.split()), width=w )

    descr = """Developer utils for NCrystalGeant4 installation."""
    parser = ArgumentParser( description=wrap(descr,79),
                             formatter_class = RawTextHelpFormatter )
    parser.add_argument('--check', action='store_true',
                        help=wrap("Perform various code checks"))
    args = parser.parse_args()
    nselect = sum( (1 if e else 0)
                   for e in [args.check,] )
    if nselect == 0:
        parser.error('Invalid usage. Run with -h/--help for instructions.')
    if nselect > 1:
        parser.error('Conflicting options')
    return args

def main():
    args = parse_args()
    import pathlib
    reporoot = pathlib.Path(__file__).parent.parent.parent.parent.absolute()
    if args.check:
        from . import check_ruff
        check_ruff.main(reporoot)
        from . import check_version
        check_version.main(reporoot)
    else:
        assert False, "should not happen"
