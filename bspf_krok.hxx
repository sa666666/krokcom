//===========================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009 by Stephen Anthony <stephena@users.sourceforge.net>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id$
//============================================================================

/**
  The bspf.hxx file comes from Stella, and was originally written by
  Brad Mott.  KrokCom needs to add stuff to it, so we wrap access and
  use 'bspf_krok.hxx' instead.

  This makes it easier to integrate new changes from Stella, since
  bspf.hxx is just dropped in here and doesn't have to be modified.

  @author  Stephen Anthony
*/

#ifndef BSPF_KROK_HXX
#define BSPF_KROK_HXX

#include <vector>

/** What system are we using? */
#if defined(WIN32) || defined(_WIN32)
  #define BSPF_WIN32
#elif defined(__APPLE__) || defined(MAC_OS_X)
  #define BSPF_MAC_OSX
#else
  #define BSPF_UNIX
#endif
#include "bspf.hxx"

typedef vector<string> StringList;

#endif // BSPF_HARMONY
