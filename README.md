#setup, build, and run simulation
Requires Geant4 and ROOT. I use Geant4 10.02.p02 but the specific version shouldn't matter too much. The G4NDL data files must be installed.

If you want visualization, Geant4 must be installed with QT, example:

```cmake ~/src/geant4.10.02.p02/ -DCMAKE_INSTALL_PREFIX="/home/meeg/software/geant4.10.02.p02" -DGEANT4_USE_QT=ON```

to build:
```source ~/software/geant4.10.02.p02/bin/geant4.sh```
```cd neutron_detector```
```mkdir build```
```cmake ..```
```make```

Run in the `build` directory.
* For a console with visualization just run `./exampleB4c` (this runs the macro `vis.mac`).
* To run a batch of 1000 events run `./exampleB4c -m run2.mac`.

Whichever way you run the simulation, it will make the output file `B4.root`.

#build and run analysis
Requires ROOT. I use ROOT 6 but ROOT 5 should be fine.

to build:
```cd neutron_detector/analysis```
```make```

to run:
```./plotNtuple ../build/B4.root```
