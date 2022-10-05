#include <cassert>

#include "GlobalCollectionNames.h"
#include "ParticleObjectHelpers.h"
#include "SamplesCore.h"
#include "JetMETHandler.h"
#include "AK4JetSelectionHelpers.h"


using namespace std;
using namespace IvyStreamHelpers;


#define VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS \
AK4JET_VARIABLE(float, pt, 0) \
AK4JET_VARIABLE(float, eta, 0) \
AK4JET_VARIABLE(float, phi, 0) \
AK4JET_VARIABLE(float, mass, 0) \
AK4JET_COMMON_VARIABLES
#define VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS_LOWPT \
AK4JET_LOWPT_VARIABLE(float, rawPt, 0) \
AK4JET_LOWPT_VARIABLE(float, eta, 0) \
AK4JET_LOWPT_VARIABLE(float, phi, 0) \
AK4JET_LOWPT_EXTRA_INPUT_VARIABLES
#define JETMET_METXY_VERTEX_VARIABLES \
JETMET_METXY_VERTEX_VARIABLE(int, npvs)

const std::string JetMETHandler::colName_ak4jets = GlobalCollectionNames::colName_ak4jets;
const std::string JetMETHandler::colName_ak4jets_lowpt = GlobalCollectionNames::colName_ak4jets_lowpt;
const std::string JetMETHandler::colName_pfmet = GlobalCollectionNames::colName_pfmet;


JetMETHandler::JetMETHandler() :
  IvyBase(),

  pfmet(nullptr),
  pfmet_XYcorr_xCoeffA(0),
  pfmet_XYcorr_xCoeffB(0),
  pfmet_XYcorr_yCoeffA(0),
  pfmet_XYcorr_yCoeffB(0)
{
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", JetMETHandler::colName_ak4jets.data()));
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) this->addConsumed<TYPE* const>(JetMETHandler::colName_ak4jets + "_" + #NAME);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS;
#undef AK4JET_VARIABLE

  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", JetMETHandler::colName_ak4jets_lowpt.data()));
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) this->addConsumed<TYPE* const>(JetMETHandler::colName_ak4jets_lowpt + "_" + #NAME);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS_LOWPT;
#undef AK4JET_LOWPT_VARIABLE

#define MET_VARIABLE(TYPE, NAME) this->addConsumed<TYPE>(JetMETHandler::colName_pfmet + "_" + #NAME);
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE

#define JETMET_METXY_VERTEX_VARIABLE(TYPE, NAME) this->addConsumed<TYPE>(GlobalCollectionNames::colName_pv + "_" + #NAME);
  JETMET_METXY_VERTEX_VARIABLES;
#undef JETMET_METXY_VERTEX_VARIABLE
}

void JetMETHandler::clear(){
  this->resetCache();

  for (auto*& prod:ak4jets) delete prod;
  for (auto*& prod:ak4jets_masked) delete prod;
  ak4jets.clear();
  ak4jets_masked.clear();
  delete pfmet; pfmet=nullptr;
}

bool JetMETHandler::constructJetMET(
  SimEventHandler const* simEventHandler
){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;

  bool res = (
    constructAK4Jets() && constructAK4Jets_LowPt() && constructMET()
    &&
    assignMETXYShifts()
    );

  if (res) this->cacheEvent();
  return res;
}

bool JetMETHandler::constructAK4Jets(){
  bool const isData = SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier);

  GlobalCollectionNames::collsize_t nProducts;
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) TYPE* const* arr_##NAME = nullptr;
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS;
  AK4JET_GENINFO_VARIABLES;
#undef AK4JET_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", JetMETHandler::colName_ak4jets.data()), nProducts);
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed<TYPE* const>(JetMETHandler::colName_ak4jets + "_" + #NAME, arr_##NAME);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS;
  if (!isData){
    AK4JET_GENINFO_VARIABLES;
  }
#undef AK4JET_VARIABLE
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "JetMETHandler::constructAK4Jets: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "JetMETHandler::constructAK4Jets: All variables are set up!" << endl;

  /************/
  /* ak4 jets */
  /************/
  ak4jets.reserve(nProducts);
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) TYPE* it_##NAME = nullptr; if (arr_##NAME) it_##NAME = &((*arr_##NAME)[0]);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS;
  AK4JET_GENINFO_VARIABLES;
#undef AK4JET_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "JetMETHandler::constructAK4Jets: Attempting ak4 jet " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass); // Yes you have to do this on a separate line...
      ak4jets.push_back(new AK4JetObject(momentum));
      AK4JetObject*& obj = ak4jets.back();

      // Set extras
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) obj->extras.NAME = *it_##NAME;
      AK4JET_COMMON_VARIABLES;
      if (!isData){
        AK4JET_GENINFO_VARIABLES;
      }
#undef AK4JET_VARIABLE

      // Set particle index as its unique identifier
      obj->setUniqueIdentifier(ip);

      // Set the selection bits
      AK4JetSelectionHelpers::setSelectionBits(*obj);

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) if (it_##NAME) it_##NAME++;
      VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS;
      AK4JET_GENINFO_VARIABLES;
#undef AK4JET_VARIABLE
    }
  }
  // Sort particles
  ParticleObjectHelpers::sortByGreaterPt(ak4jets);

  return true;
}
bool JetMETHandler::constructAK4Jets_LowPt(){
  bool const isData = SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier);

  GlobalCollectionNames::collsize_t nProducts;
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) TYPE* const* arr_##NAME = nullptr;
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS_LOWPT;
#undef AK4JET_LOWPT_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", JetMETHandler::colName_ak4jets_lowpt.data()), nProducts);
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed<TYPE* const>(JetMETHandler::colName_ak4jets_lowpt + "_" + #NAME, arr_##NAME);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS_LOWPT;
#undef AK4JET_LOWPT_VARIABLE
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "JetMETHandler::constructAK4Jets_LowPt: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "JetMETHandler::constructAK4Jets_LowPt: All variables are set up!" << endl;

  ak4jets_masked.reserve(nProducts);
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) TYPE* it_##NAME = nullptr; if (arr_##NAME) it_##NAME = &((*arr_##NAME)[0]);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS_LOWPT;
#undef AK4JET_LOWPT_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "JetMETHandler::constructAK4Jets_LowPt: Attempting ak4 jet " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_rawPt, *it_eta, *it_phi, 0); // Yes you have to do this on a separate line...
      ak4jets_masked.push_back(new AK4JetObject(momentum));
      AK4JetObject*& obj = ak4jets_masked.back();

      // Set extras
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) obj->extras.NAME = *it_##NAME;
      AK4JET_LOWPT_EXTRA_INPUT_VARIABLES;
#undef AK4JET_LOWPT_VARIABLE

      // Set particle index as its unique identifier
      obj->setUniqueIdentifier(ip);

      // These jets always fail selection, so there is no need to set bits

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) if (it_##NAME) it_##NAME++;
      VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS_LOWPT;
#undef AK4JET_LOWPT_VARIABLE
    }
  }
  // Sort particles
  ParticleObjectHelpers::sortByGreaterPt(ak4jets_masked);

  return true;
}

bool JetMETHandler::constructMET(){
  bool const isData = SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier);

#define MET_VARIABLE(TYPE, NAME) TYPE const* pfmet_##NAME = nullptr;
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = true;
#define MET_VARIABLE(TYPE, NAME) allVariablesPresent &= this->getConsumed(JetMETHandler::colName_pfmet + "_" + #NAME, pfmet_##NAME);
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "JetMETHandler::constructMET: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "JetMETHandler::constructMET: All variables are set up!" << endl;

  /**********/
  /* PF MET */
  /**********/
  pfmet = new METObject();
#define MET_VARIABLE(TYPE, NAME) pfmet->extras.NAME = *pfmet_##NAME;
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE

  return true;
}
bool JetMETHandler::assignMETXYShifts(){
#define JETMET_METXY_VERTEX_VARIABLE(TYPE, NAME) TYPE const* NAME = nullptr;
  JETMET_METXY_VERTEX_VARIABLES;
#undef JETMET_METXY_VERTEX_VARIABLE

  bool allVariablesPresent = true;
#define JETMET_METXY_VERTEX_VARIABLE(TYPE, NAME) allVariablesPresent &= this->getConsumed(GlobalCollectionNames::colName_pv + "_" + #NAME, NAME);
  JETMET_METXY_VERTEX_VARIABLES;
#undef JETMET_METXY_VERTEX_VARIABLE
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "JetMETHandler::assignMETXYShifts: Not all variables are consumed properly!" << endl;
    assert(0);
    return false;
  }

  float const npv = std::min(*npvs, (int) 100); // Effective number of primary vertices
  float METxcorr = -(pfmet_XYcorr_xCoeffA*npv + pfmet_XYcorr_xCoeffB);
  float METycorr = -(pfmet_XYcorr_yCoeffA*npv + pfmet_XYcorr_yCoeffB);
  pfmet->setXYShift(METxcorr, METycorr);
  if (this->verbosity>=MiscUtils::DEBUG) IVYerr << "JetMETHandler::assignMETXYShifts: Applying an additional XY shift of ( " << METxcorr << ", " << METycorr << " )" << endl;

  return true;
}

bool JetMETHandler::wrapTree(BaseTree* tree){
  if (!tree) return false;

  static bool printWarnings = true;

  // 200314: The following is taken from https://lathomas.web.cern.ch/lathomas/METStuff/XYCorrections/XYMETCorrection.h
  // The formula is corr = -(A*npv + B).
  pfmet_XYcorr_xCoeffA = pfmet_XYcorr_xCoeffB = pfmet_XYcorr_yCoeffA = pfmet_XYcorr_yCoeffB = 0;

  TString theDP = SampleHelpers::getDataPeriod();
  auto const theDY = SampleHelpers::getDataYear();
  bool const isData = SampleHelpers::checkSampleIsData(tree->sampleIdentifier, &theDP);
  if (isData){
    if (theDP == "2016B"){
      pfmet_XYcorr_xCoeffA = -0.0214894; pfmet_XYcorr_xCoeffB = -0.188255;
      pfmet_XYcorr_yCoeffA = 0.0876624; pfmet_XYcorr_yCoeffB = 0.812885;
    }
    else if (theDP == "2016C"){
      pfmet_XYcorr_xCoeffA = -0.032209; pfmet_XYcorr_xCoeffB = 0.067288;
      pfmet_XYcorr_yCoeffA = 0.113917; pfmet_XYcorr_yCoeffB = 0.743906;
    }
    else if (theDP == "2016D"){
      pfmet_XYcorr_xCoeffA = -0.0293663; pfmet_XYcorr_xCoeffB = 0.21106;
      pfmet_XYcorr_yCoeffA = 0.11331; pfmet_XYcorr_yCoeffB = 0.815787;
    }
    else if (theDP == "2016E"){
      pfmet_XYcorr_xCoeffA = -0.0132046; pfmet_XYcorr_xCoeffB = 0.20073;
      pfmet_XYcorr_yCoeffA = 0.134809; pfmet_XYcorr_yCoeffB = 0.679068;
    }
    else if (theDP == "2016F_APV"){
      pfmet_XYcorr_xCoeffA = -0.0543566; pfmet_XYcorr_xCoeffB = 0.816597;
      pfmet_XYcorr_yCoeffA = 0.114225; pfmet_XYcorr_yCoeffB = 1.17266;
    }
    else if (theDP == "2016F_NonAPV"){
      pfmet_XYcorr_xCoeffA = 0.134616; pfmet_XYcorr_xCoeffB = -0.89965;
      pfmet_XYcorr_yCoeffA = 0.0397736; pfmet_XYcorr_yCoeffB = 1.0385;
    }
    else if (theDP == "2016G"){
      pfmet_XYcorr_xCoeffA = 0.121809; pfmet_XYcorr_xCoeffB = -0.584893;
      pfmet_XYcorr_yCoeffA = 0.0558974; pfmet_XYcorr_yCoeffB = 0.891234;
    }
    else if (theDP == "2016H"){
      pfmet_XYcorr_xCoeffA = 0.0868828; pfmet_XYcorr_xCoeffB = -0.703489;
      pfmet_XYcorr_yCoeffA = 0.0888774; pfmet_XYcorr_yCoeffB = 0.902632;
    }
    else if (theDP == "2017B"){
      pfmet_XYcorr_xCoeffA = -0.211161; pfmet_XYcorr_xCoeffB = 0.419333;
      pfmet_XYcorr_yCoeffA = 0.251789; pfmet_XYcorr_yCoeffB = -1.28089;
    }
    else if (theDP == "2017C"){
      pfmet_XYcorr_xCoeffA = -0.185184; pfmet_XYcorr_xCoeffB = -0.164009;
      pfmet_XYcorr_yCoeffA = 0.200941; pfmet_XYcorr_yCoeffB = -0.56853;
    }
    else if (theDP == "2017D"){
      pfmet_XYcorr_xCoeffA = -0.201606; pfmet_XYcorr_xCoeffB = 0.426502;
      pfmet_XYcorr_yCoeffA = 0.188208; pfmet_XYcorr_yCoeffB = -0.58313;
    }
    else if (theDP == "2017E"){
      pfmet_XYcorr_xCoeffA = -0.162472; pfmet_XYcorr_xCoeffB = 0.176329;
      pfmet_XYcorr_yCoeffA = 0.138076; pfmet_XYcorr_yCoeffB = -0.250239;
    }
    else if (theDP == "2017F"){
      pfmet_XYcorr_xCoeffA = -0.210639; pfmet_XYcorr_xCoeffB = 0.72934;
      pfmet_XYcorr_yCoeffA = 0.198626; pfmet_XYcorr_yCoeffB = 1.028;
    }
    else if (theDP == "2018A"){
      pfmet_XYcorr_xCoeffA = 0.263733; pfmet_XYcorr_xCoeffB = -1.91115;
      pfmet_XYcorr_yCoeffA = 0.0431304; pfmet_XYcorr_yCoeffB = 0.112043;
    }
    else if (theDP == "2018B"){
      pfmet_XYcorr_xCoeffA = 0.400466; pfmet_XYcorr_xCoeffB = -3.05914;
      pfmet_XYcorr_yCoeffA = 0.146125; pfmet_XYcorr_yCoeffB = -0.533233;
    }
    else if (theDP == "2018C"){
      pfmet_XYcorr_xCoeffA = 0.430911; pfmet_XYcorr_xCoeffB = -1.42865;
      pfmet_XYcorr_yCoeffA = 0.0620083; pfmet_XYcorr_yCoeffB = -1.46021;
    }
    else if (theDP == "2018D"){
      pfmet_XYcorr_xCoeffA = 0.457327; pfmet_XYcorr_xCoeffB = -1.56856;
      pfmet_XYcorr_yCoeffA = 0.0684071; pfmet_XYcorr_yCoeffB = -0.928372;
    }
    else if (theDP.Contains("2022")){
      if (printWarnings) IVYout << "JetMETHandler::wrapTree: WARNING! Data period " << theDP << " does not currently feature the data MET corrections yet." << endl;
    }
    else{
      IVYerr << "JetMETHandler::wrapTree: Data period " << theDP << " is undefined for the data MET corrections." << endl;
      return false;
    }
  }
  else{
    switch (theDY){
    case 2016:
    {
      if (!SampleHelpers::isAPV2016Affected(theDP)){
        pfmet_XYcorr_xCoeffA = -0.153497; pfmet_XYcorr_xCoeffB = -0.231751;
        pfmet_XYcorr_yCoeffA = 0.00731978; pfmet_XYcorr_yCoeffB = 0.243323;
      }
      else{
        pfmet_XYcorr_xCoeffA = -0.188743; pfmet_XYcorr_xCoeffB = 0.136539;
        pfmet_XYcorr_yCoeffA = 0.0127927; pfmet_XYcorr_yCoeffB = 0.117747;
      }
      break;
    }
    case 2017:
      pfmet_XYcorr_xCoeffA = -0.300155; pfmet_XYcorr_xCoeffB = 1.90608;
      pfmet_XYcorr_yCoeffA = 0.300213; pfmet_XYcorr_yCoeffB = -2.02232;
      break;
    case 2018:
      pfmet_XYcorr_xCoeffA = 0.183518; pfmet_XYcorr_xCoeffB = 0.546754;
      pfmet_XYcorr_yCoeffA = 0.192263; pfmet_XYcorr_yCoeffB = -0.42121;
      break;
    case 2022:
      if (printWarnings) IVYout << "JetMETHandler::wrapTree: WARNING! Data year " << theDY << " does not feature the MC MET corrections yet." << endl;
      break;
    default:
      IVYerr << "JetMETHandler::wrapTree: Year " << SampleHelpers::getDataYear() << " is undefined for the MC MET corrections." << endl;
      return false;
      break;
    }
  }

  printWarnings = false;

  return IvyBase::wrapTree(tree);
}

void JetMETHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", JetMETHandler::colName_ak4jets.data()), 0);
  tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", JetMETHandler::colName_ak4jets_lowpt.data()), 0);

  bool const isData = SampleHelpers::checkSampleIsData(tree->sampleIdentifier);
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) \
this->addConsumed<TYPE* const>(JetMETHandler::colName_ak4jets + "_" + #NAME); \
this->defineConsumedSloppy(JetMETHandler::colName_ak4jets + "_" + #NAME); \
tree->bookArrayBranch<TYPE>(JetMETHandler::colName_ak4jets + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_ak4jets);
  if (!isData){
    AK4JET_GENINFO_VARIABLES;
  }
#undef AK4JET_VARIABLE

#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) tree->bookArrayBranch<TYPE>(JetMETHandler::colName_ak4jets + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_ak4jets);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS;
#undef AK4JET_VARIABLE

#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) tree->bookArrayBranch<TYPE>(JetMETHandler::colName_ak4jets_lowpt + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_ak4jets_lowpt);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES_AK4JETS_LOWPT;
#undef AK4JET_LOWPT_VARIABLE

#define MET_VARIABLE(TYPE, NAME) tree->bookBranch<TYPE>(JetMETHandler::colName_pfmet + "_" + #NAME, 0);
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE

  // Vertex variables
#define JETMET_METXY_VERTEX_VARIABLE(TYPE, NAME) tree->bookBranch<TYPE>(GlobalCollectionNames::colName_pv + "_" + #NAME, 0);
  JETMET_METXY_VERTEX_VARIABLES;
#undef JETMET_METXY_VERTEX_VARIABLE
}


#undef VECTOR_ITERATOR_HANDLER_DIRECTIVES
