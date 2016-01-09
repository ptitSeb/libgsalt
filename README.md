# libgsalt
Grain of SALT, the Geometry Simplification At Load Time library


Introduction
============

GSalt is a library tobe used to simplifiy geometry at load time.

That means:
 1. Simplify geometry: remove vertex/triangle from a model, while still trying to maintain visual aspect
 2. A load time: because it may takes time, so don't expect realtime "LOD" algorythms (LOD = Level Of Detail)

The library use the algorithm by Mickael Garland "Surface simplification using quadric metrics"

Build
======

Simply
```
cmake
make
sudo make install
```

You'll need a C++ compiler for it.

Use
===

*TODO*
