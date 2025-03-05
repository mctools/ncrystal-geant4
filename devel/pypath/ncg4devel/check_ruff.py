
def main( reporoot ):
    print(reporoot)
    import subprocess
    import shutil
    print("Checking ruff...")
    ruff = shutil.which('ruff')
    if not ruff:
        raise SystemExit('ERROR: ruff command not available')
    files = sorted(reporoot.rglob('*.py'))
    rv = subprocess.run(['ruff','check'] + [str(f) for f in files] )
    if rv.returncode!=0:
        raise SystemExit(1)
    print("Checking ruff... DONE")
