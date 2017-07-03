//============================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009-2017 by Stephen Anthony <sa666666@gmail.com>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

/**
  Bankswitching types as defined by Armin (Krokodile Cart designer).

  @author  Stephen Anthony
*/

#ifndef BSTYPE_HXX
#define BSTYPE_HXX

#include "bspf.hxx"

enum BSType
{
  BS_4K   = 0,
  BS_F8   = 1,
  BS_F6   = 2,
  BS_F4   = 3,
  BS_MCF6 = 4,
  BS_FA   = 5,
  BS_3F   = 6,
  BS_MCF8 = 7,
  BS_F8SC = 8,
  BS_F6SC = 9,
  BS_F4SC = 10,
  BS_MCF4 = 11,
  BS_MC4K = 12,
  BS_EF   = 13,
  BS_CV   = 14,
  BS_3E   = 15,
  BS_UA   = 16,
  BS_F0   = 17,   // not supported at all
  BS_E0   = 18,   // not supported at all
  BS_E7   = 19,
  BS_FE   = 20,   // not supported at all
  BS_AR   = 21,   // not supported at all
  BS_EFSC = 22,
  BS_0840 = 23,

  BS_DPC  = 100,  // items from this point on will probably
  BS_4A50 = 101,  // never be implemented
  BS_X07  = 102,
  BS_SB   = 103,
  BS_MC   = 104,
  BS_DPCP = 105,

  BS_NONE = 1000,
  BS_AUTO = 1001,
};

class Bankswitch
{
  public:
    static string typeToName(BSType type)
    {
      switch(type)
      {
        case BS_4K:   return "4K";
        case BS_F8:   return "F8";
        case BS_F6:   return "F6";
        case BS_F4:   return "F4";
        case BS_MCF6: return "MCF6";
        case BS_FA:   return "FA";
        case BS_3F:   return "3F";
        case BS_MCF8: return "MCF8";
        case BS_F8SC: return "F8SC";
        case BS_F6SC: return "F6SC";
        case BS_F4SC: return "F4SC";
        case BS_MCF4: return "MCF4";
        case BS_MC4K: return "MC4K";
        case BS_EF:   return "EF";
        case BS_CV:   return "CV";
        case BS_3E:   return "3E";
        case BS_UA:   return "UA";
        case BS_F0:   return "F0";
        case BS_E0:   return "E0";
        case BS_E7:   return "E7";
        case BS_FE:   return "FE";
        case BS_AR:   return "AR";
        case BS_EFSC: return "EFSC";
        case BS_0840: return "0840";

        case BS_DPC:  return "DPC";
        case BS_4A50: return "4A50";
        case BS_X07:  return "X07";
        case BS_SB:   return "SB";
        case BS_MC:   return "MC";
        case BS_DPCP: return "DPC+";
        case BS_NONE: return "NONE/UNKNOWN";
        case BS_AUTO: return "AUTO";
      }
      return "NONE";
    }

    static BSType nameToType(const string& name)
    {
      if(BSPF::equalsIgnoreCase(name, "4K"))         return BS_4K;
      else if(BSPF::equalsIgnoreCase(name, "F8"))    return BS_F8;
      else if(BSPF::equalsIgnoreCase(name, "F6"))    return BS_F6;
      else if(BSPF::equalsIgnoreCase(name, "F4"))    return BS_F4;
      else if(BSPF::equalsIgnoreCase(name, "MCF6"))  return BS_MCF6;
      else if(BSPF::equalsIgnoreCase(name, "FA"))    return BS_FA;
      else if(BSPF::equalsIgnoreCase(name, "3F"))    return BS_3F;
      else if(BSPF::equalsIgnoreCase(name, "MCF8"))  return BS_MCF8;
      else if(BSPF::equalsIgnoreCase(name, "F8SC"))  return BS_F8SC;
      else if(BSPF::equalsIgnoreCase(name, "F6SC"))  return BS_F6SC;
      else if(BSPF::equalsIgnoreCase(name, "F4SC"))  return BS_F4SC;
      else if(BSPF::equalsIgnoreCase(name, "MCF4"))  return BS_MCF4;
      else if(BSPF::equalsIgnoreCase(name, "MC4K"))  return BS_MC4K;
      else if(BSPF::equalsIgnoreCase(name, "EF"))    return BS_EF;
      else if(BSPF::equalsIgnoreCase(name, "CV"))    return BS_CV;
      else if(BSPF::equalsIgnoreCase(name, "3E"))    return BS_3E;
      else if(BSPF::equalsIgnoreCase(name, "UA"))    return BS_UA;
      else if(BSPF::equalsIgnoreCase(name, "F0"))    return BS_F0;
      else if(BSPF::equalsIgnoreCase(name, "E0"))    return BS_E0;
      else if(BSPF::equalsIgnoreCase(name, "E7"))    return BS_E7;
      else if(BSPF::equalsIgnoreCase(name, "FE"))    return BS_FE;
      else if(BSPF::equalsIgnoreCase(name, "AR"))    return BS_AR;
      else if(BSPF::equalsIgnoreCase(name, "EFSC"))  return BS_EFSC;
      else if(BSPF::equalsIgnoreCase(name, "0840"))  return BS_0840;

      else if(BSPF::equalsIgnoreCase(name, "DPC"))   return BS_DPC;
      else if(BSPF::equalsIgnoreCase(name, "4A50"))  return BS_4A50;
      else if(BSPF::equalsIgnoreCase(name, "X07"))   return BS_X07;
      else if(BSPF::equalsIgnoreCase(name, "SB"))    return BS_SB;
      else if(BSPF::equalsIgnoreCase(name, "MC"))    return BS_MC;
      else if(BSPF::equalsIgnoreCase(name, "DPC+"))  return BS_DPCP;
      else if(BSPF::equalsIgnoreCase(name, "AUTO"))  return BS_AUTO;
      else                                           return BS_NONE;
    }
};

#endif
