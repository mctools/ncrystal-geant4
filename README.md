ncrystal-geant4
===============

This package faciliates the usage of low energy neutron scattering physics
provided by NCrystal, in the Geant4 simulation framework. Specifically, it
allows users of HP physics models, to initialise selected G4Materials based on
NCrystal cfg-strings, and to let NCrystal take over the scattering physics of
low energy (<5eV) neutrons in those volumes (other particles and physics will
still be handled by the usual Geant4 physics list).

The current implementation assumes that the user is using one of Geant4's HP
physics lists, and unfortunately does not yet support multi-threaded Geant4
simulations.

To use the NCrystal-Geant4 bindings, one must:

1. Install NCrystal and Geant4. Although it is possible to build both projects
   manually, it should be noted that both are available on conda-forge. If not
   using conda, note that NCrystal is also available on PyPI,, so assuming
   Geant4 has already been installed in another manner, it might be possible to
   simply complete the setup by a "pip install NCrystal".
2. Install ncrystal-geant4. This can be done by "pip install ncrystal-geant4",
   or alternatively by cloning the repository at
   https://github.com/mctools/ncrystal-geant4 and setting the CMake
   variable `NCrystalGeant4_DIR` to point at the `src/ncrystal_geant4/cmake`
   subdir of the cloned repo.
3. Edit your projects CMakeLists.txt to add a `find_package(NCrystalGeant4)`
   statement, and adding the `NCrystalGeant4::NCrystalGeant4` target as a
   dependency of the Geant4 application you are building.
4. Edit your C++ code and include the header file
   `G4NCrystal/G4NCrystal.hh`. Then create NCrystal materials based on NCrystal
   cfg-strings like `"Al_sg225.ncmat;temp=80K"` using code such as:
   ```
   G4Material * myMat = G4NCrystal::createMaterial("Al_sg225.ncmat;temp=80K");
   ```
   Finally, one must inject the NCrystal physics process into the physics list
   by placing a statement like:
   ```
   G4NCrystal::install();
   ```
   Crucially, such a statement must be placed _after_ `runManager->Initialize()`
   and _before_ `runManager->BeamOn(..)` is called.

For an actual example using the recipes above, see the `example/` folder next to
this README file (or find it online at
https://github.com/mctools/ncrystal-geant4/tree/HEAD/example ).
