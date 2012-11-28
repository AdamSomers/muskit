muskit
======

A C++ toolkit for music applications

The Music Programming Toolkit, or "MusKit", is an SDK to for audio software development in C++.  The vision behind MusKit is to provide a sandbox for people who want to implement DSP algorithms in C++ while also providing a lightweight, yet effective architecture for application design.

To compile on MacOS with XCode, the following are required:

- Project must search for headers in MusKit and ThirdParty
- -D__MACOSX_CORE__ complier flag must be set (see 'other C flags')
- Must link to CoreFoundation.framework, CoreMIDI.framework, and CoreAudio.framework
- Compile all files in MusKit/src and ThirdParty/ 

The easiest way to start a stand-alone MusKit project is to copy the Examples
project and modify one of the targets (or better yet, create a new target).
However, it is probably not a good idea to put your own projects within the MusKit
directory.  So the best way to create your own project is to copy the Examples 
project to directory which contains MusKit, and set the source and header search
locations accordingly.