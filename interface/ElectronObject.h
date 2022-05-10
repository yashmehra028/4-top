#ifndef ELECTRONOBJECT_H
#define ELECTRONOBJECT_H

#define ELECTRON_EXTRA_VARIABLES \ 
ELECTRON_VARIABLE(float, mvaFall17V2noIso)\
ELECTRON_VARIABLE(bool, mvaFall17V2noIso_WP80)\
ELECTRON_VARIABLE(bool, mvaFall17V2noIso_WP90)\
ELECTRON_VARIABLE(bool, mvaFall17V2noIso_WPL)\ 
ELECTRON_VARIABLE(float, deltaEtaSC)

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
