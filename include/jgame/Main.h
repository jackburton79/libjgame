/*
 * Main.h
 *
 * For the source file that defines main(). On some systems (Windows, Android)
 * SDL renames it to SDL_main() and wants it declared with C linkage; this
 * header brings that in, so a game doesn't have to include SDL itself for it.
 *
 * It is not in <jgame/jgame.h>: a file that is not the program's main() has
 * no use for it, and a test with a plain main() (built with -Umain) must not
 * get it renamed.
 */
#pragma once

#include <SDL_main.h>
