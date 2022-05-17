#ifndef ELECTRONOBJECT_H
#define ELECTRONOBJECT_H

#define ELECTRON_EXTRA_VARIABLES \ 
ELECTRON_VARIABLE(float, mvaFall17V2noIso)\
ELECTRON_VARIABLE(float, miniPFRelIso_all)\
ELECTRON_VARIABLE(float, deltaEtaSC)\ 

// TODO: should these be electron extra variables?
// or should they be implemented as functions in selection_tools.h?
ELECTRON_VARIABLE(float, ptRel)\ 
ELECTRON_VARIABLE(float, ptRatio) 




class ElectronVariables{
public:
  #define ELECTRON_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
    ELECTRON_VARIABLES;
  #undef ELECTRON_VARIABLE

  ElectronVariables();
  ElectronVariables(ElectronVariables const& other);
  ElectronVariables& operator=(const ElectronVariables& other);

  void swap(ElectronVariables& other);
};

class ElectronObject : public ParticleObject{
public:
  ElectronVariables extras;

  ElectronObject();
  ElectronObject(int id_);
  ElectronObject(int id_, LorentzVector_t const& mom_);
  ElectronObject(const ElectronObject& other);
  ElectronObject& operator=(const ElectronObject& other);
  ~ElectronObject();

  void swap(ElectronObject& other);

  float const& etaSC() const{ return extras.deltaEtaSC + this->eta(); }

};

#endif
