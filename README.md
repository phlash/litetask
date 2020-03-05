# Litetask

My learning experience of writing an x86 real-mode kernel, bootloader & some hardware drivers
from way back in 1994-95. If you want to build/run anything you'll need Dosbox.

My thanks to many people who didn't dissuede me at the time, and Sam for encouraging me to
publish after 25 years of languishing in the dark on tape backup.

## Does it build?

Yes! I have included the required tools along with the source code (it took ages to find 'em!)
I have been unable to source the required version of Polytron VCS (PVCS) or Polytron Make, so
all the \*.??v files are useless (and I should remove from git at some point) and I have had to
port each makefile to GNU make (thankfully I didn't do anything clever!)

 * Clone this repo (eg: git clone https://github.com/phlash/litetask.git)
 * Start Dosbox
 * Mount the repo root folder as drive C: (eg: mount c /home/phlash/litetask)
 * NB: you can do both from the command line, eg: dosbox /home/phlash/litetask
 * C:
 * SETUP.BAT - this just configures a few env vars
 * CD LITETASK
 * LITEMAKE - to build the libraries
 * LITEMAKE clean - to clean up built objects
 * You may also CD to any sub-folder and run PMAKE [clean]

## Does it run?

Not sure yet, I got as far as tooling and build/clean fixes for the core libraries before going
public. There are many test folders with binaries(!) in from 1995, these are not built with the
libraries although there are makefiles for most of them which still need porting to GNUmake.

## Dude the compiler warnings are gross!

Yes I know - I was young and in a hurry, please don't base anything serious on this hackery!

## Will you support it/accept PRs/licence it?

There's probably a suitable meme for this.. in short, no sorry.


## Can I ask questions about how/why you did certain things?

Sure, drop me an email mailto:phil.github@ashbysoft.com or post an issue here for a public reply.

Phlash, Feb 2020.
