# Jan Stat Editor
A Stat Editor for Grand Theft Auto V based on BigBaseV2 by pocakking and gir489
Strictly for educational purposes.

## Stat Editor Features
* Write Integer Stat
* Write Bool Stat
* Write Float Stat
* Write Date Stat
* Write String Stat
* Write Packed Bool Stat
* Write Packed Int Stat
* Increment Stat

* Read Integer Stat
* Read Bool Stat
* Read Float Stat
* Read Date Stat
* Read String Stat
* Read Packed Bool Stat
* Read Packed Int Stat

* Global Variable Editor
* Local Variable Editor
* Some Unlocker


## Base Features
* ImGui–based user interface
* Unloading at runtime
* Log console
* Script fibers
* Fiber pool
* Access to YSC script globals
* scrProgram–based native hook

## Building
To build Jan Stat Editor you need:
* Visual Studio 2019
* [Premake 5.0](https://premake.github.io/download.html) in your PATH

To set up the build environment, run the following commands in a terminal:
```dos
git clone https://github.com/skript023/Jan-Stat-Editor.git --recurse-submodules
cd BigBaseV2-fix
GenerateProjects.bat
```
Now, you will be able to open the solution, and simply build it in Visual Studio.
