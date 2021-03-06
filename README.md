# LEO_Mobility_NS-3

This project is to support my academic papers.

## function
Because ns-3 itself lacks the corresponding LEO satellite mobile module, it is necessary to develop a simple and easy-to-use LEO satellite mobile module in ns-3.

Based on it, a new segment routing protocol SWS is implemented. In the simulation, the implementation of SWS is to use Nix routing and add some mechanisms to find characteristic waypoints. See src/mobility/model/leo-satellite-mobility-model.cc(h) and leo-satellite-helper.cc(h) for details.

This module is based on ns-3.29.

## document introduction

There are four directories:
One is scratch/, which contains test cases of various LEO satellite constellations, including iridium, oneweb, startlink, etc., as well as those using the new segmented routing algorithm.For iridium constellation, three test cases of different routing algorithms are written. It includes the best route of ipv4globalroutinghelper (see iridium-topology2-updV3.cc), improved Nix segmented route (see iridium-nix-segment-routing.cc and iridium-SWS.cc) and OLSR (see iridium-OLSR.cc).

Second directory is src/mobility/, which only adds a few files to the source directory.

The third directory is helper/, which contains some scripts. Among them, analysisNs3TranceFile.py is used to analyze the trace file obtained from the running of test cases in scratch; generate_satellite_init_position.py is used to generate initial constellation data; the other three files are used to visualize the orbit of iridium, oneweb, startlink and other constellations.

The fourth folder is nix-vector-routing/, it makes some modifications to nix vector routing to adapt to SWS algorithm.


## use
To use it, simply replace the file directory with src/mobility/ and src/nix-vector-routing/. Then, recompile：
```
./waf configure
./waf build
```
Then copy the file in scratch to the corresponding file, modify and run it.


## supplement
The detailed implementation principle of the source code is shown in my master's thesis.

