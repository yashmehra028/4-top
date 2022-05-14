#ifndef MUONOBJECT_H
#define MUONOBJECT_H

#define MUON_EXTRA_VARIABLES \ 
MUON_VARIABLE(float, mvaFall17V2noIso)\
MUON_VARIABLE(float, Muon_miniPFRelIso_all)\
MUON_VARIABLE(float, deltaEtaSC)\ 

// TODO: should these be muon extra variables?
// or should they be implemented as functions in selection_tools.h?
MUON_VARIABLE(float, deltaEtaSC)\ 
MUON_VARIABLE(float, ptRel)\ 
MUON_VARIABLE(float, ptRatio) 




class MuonVariables{
public:
  #define MUON_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
    MUON_VARIABLES;
  #undef MUON_VARIABLE

  MuonVariables();
  MuonVariables(MuonVariables const& other);
  MuonVariables& operator=(const MuonVariables& other);

  void swap(MuonVariables& other);
};

class MuonObject : public ParticleObject{
public:
  MuonVariables extras;

  MuonObject();
  MuonObject(int id_);
  MuonObject(int id_, LorentzVector_t const& mom_);
  MuonObject(const MuonObject& other);
  MuonObject& operator=(const MuonObject& other);
  ~MuonObject();

  void swap(MuonObject& other);

  float const& etaSC() const{ return extras.deltaEtaSC + this->eta(); }

};

#endif
