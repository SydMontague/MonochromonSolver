# Monochromon Solver

A tool designed to calculate the fastest possible solution to Monochromon's Shop minigame for a given initial RNG state.

It's primarily aimed at speedrunners and as such isn't necessarily user friendly.

# Usage
Run the tool in a command line like this:

```MonochromonSolver <seed> [options]```

You can abort the execution by pressing Ctrl+C. It will print you the best found result so far.
This can be useful if you only care for *a* solution, not the best one.

Depending on the seed the tool might run for a few minutes or even hours. 

There are several command line flags that can be used to configure the execution:

```
  -h [ --help ]                 This text.
  --seed arg                    The initial seed for the shop, taken when talking to Monochromon.
  -m [ --mode ] arg (=combined) The solver mode used. Valid: combined|deep|heuristic
                                combined -> use heuristic and deep solver in parallel, finds lowest score
                                            heuristic is to find a quick base value, to speed up the deep solve
                                            might take several minutes, depending on the seed!
                                deep -> use deep solver exclusively, finds lowest score
                                        not recommended over combined, unless you use the --score option
                                        might take several minutes, depending on the seed!
                                heuristic -> use heuristic solver, doesn't find lowest score
                                             not recommended, unless you want a result quickly
                                             fast, unless you turn heuristic_attempts very high
  -s [ --score ] arg (=99999)   Initial "best" score, ignores any result worse than that.
                                Setting this can allow deep search to faster rule out slow paths,
                                but might yield no result at all when there is no better path.
                                Useful when comparing multiple seeds.
  -a [ --advances ] arg (=4)    The maximum number of advances from the base seed to check.
                                Time loss from advancing is taken into account.
                                Spawns up to 2 threads per advance and thus increases CPU load.
                                Recommended to use, it can reduce execution time significantly.
  --attempts arg (=5000000)     Number of attempts when using heuristic or combined solver.
                                Rarely finds anything better after 10000000.
  -d [ --depth ] arg (=30)      Maximum number of inputs when using deep or combined solver.
                                Higher values might find solutions with plenty CANCELs, that should be faster.
                                On the flip side, it might increase run time significantly.
```

When you abort the execution the currently best result gets printed.


# Building

This project uses CMake in combination CPM.cmake for dependency management.

Building the project should be a simple

```
$ git clone git@github.com:Operation-Decoded/DW1ModelConverter.git
$ cd <project dir>
$ cmake . -DCMAKE_BUILD_TYPE=Release
$ cmake --build . --config Release
```

Or you just open the folder with a CMake enabled IDE like VS Code.

# Contact

* Discord: SydMontague, or in either the [Digimon Modding Community](https://discord.gg/cb5AuxU6su) or [Digimon Discord Community](https://discord.gg/0VODO3ww0zghqOCO)
* directly on GitHub
* E-Mail: sydmontague@web.de
* Reddit: [/u/Sydmontague](https://reddit.com/u/sydmontague)
* if you find a SydMontague somewhere else chances are high that's me, too. ;)