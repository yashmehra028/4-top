#include <exception>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include "JetResolutionObject.h"


const JetCorrectionUtilities::bijection_map<JESRHelpers::JERBinning, std::string> JetParameters::binning_to_string ={
  { JESRHelpers::JERBinning::JetPt, "JetPt" },
  { JESRHelpers::JERBinning::JetEta, "JetEta" },
  { JESRHelpers::JERBinning::JetAbsEta, "JetAbsEta" },
  { JESRHelpers::JERBinning::JetE, "JetE" },
  { JESRHelpers::JERBinning::JetArea, "JetArea" },
  { JESRHelpers::JERBinning::Mu, "Mu" },
  { JESRHelpers::JERBinning::Rho, "Rho" },
  { JESRHelpers::JERBinning::NPV, "NPV" }
};

JetParameters::JetParameters(JetParameters&& rhs) :
  m_values(std::move(rhs.m_values))
{}

JetParameters::JetParameters(std::initializer_list<typename value_type::value_type> const& init){
  for (auto& i : init) set(i.first, i.second);
}

JetParameters& JetParameters::setJetPt(float pt){
  m_values[JESRHelpers::JERBinning::JetPt] = pt;
  return *this;
}

JetParameters& JetParameters::setJetEta(float eta){
  m_values[JESRHelpers::JERBinning::JetEta] = eta;
  m_values[JESRHelpers::JERBinning::JetAbsEta] = std::abs(eta);
  return *this;
}

JetParameters& JetParameters::setJetE(float e){
  m_values[JESRHelpers::JERBinning::JetE] = e;
  return *this;
}

JetParameters& JetParameters::setJetArea(float area){
  m_values[JESRHelpers::JERBinning::JetArea] = area;
  return *this;
}

JetParameters& JetParameters::setMu(float mu){
  m_values[JESRHelpers::JERBinning::Mu] = mu;
  return *this;
}

JetParameters& JetParameters::setNPV(float npv){
  m_values[JESRHelpers::JERBinning::NPV] = npv;
  return *this;
}

JetParameters& JetParameters::setRho(float rho){
  m_values[JESRHelpers::JERBinning::Rho] = rho;
  return *this;
}

JetParameters& JetParameters::set(const JESRHelpers::JERBinning& bin, float value){
  m_values.emplace(bin, value);
  if (bin == JESRHelpers::JERBinning::JetEta) m_values.emplace(JESRHelpers::JERBinning::JetAbsEta, std::abs(value));
  return *this;
}

JetParameters& JetParameters::set(const typename value_type::value_type& value){
  set(value.first, value.second);
  return *this;
}

std::vector<float> JetParameters::createVector(const std::vector<JESRHelpers::JERBinning>& binning) const{
  std::vector<float> values;
  for (auto const& bin : binning){
    auto const& it = m_values.find(bin);
    if (it == m_values.cend()){
      JetCorrectionUtilities::handleError("JetParameters::createVector",
                      "JER parametrisation depends on '" + JetParameters::binning_to_string.findByFirst(bin)->second +
                      "' but no value for this parameter has been specified. Please call the appropriate 'set' "
                      "function of the JetParameters object");
    }

    values.push_back(it->second);
  }

  return values;
}

JetResolutionObject::Definition::Definition(const std::string& definition){
  std::vector<std::string> tokens = JetCorrectionUtilities::getTokens(definition);

  // We need at least 3 tokens
  if (tokens.size() < 3){
    JetCorrectionUtilities::handleError("JetResolutionObject::Definition::Definition",
                    "Definition line needs at least three tokens. Please check file format.");
  }

  size_t n_bins = std::stoul(tokens[0]);

  if (tokens.size() < (n_bins + 2)){
    JetCorrectionUtilities::handleError("JetResolutionObject::Definition::Definition", "Invalid file format. Please check.");
  }

  for (size_t i = 0; i < n_bins; i++){
    m_bins_name.push_back(tokens[i + 1]);
  }

  size_t n_variables = std::stoul(tokens[n_bins + 1]);

  if (tokens.size() < (1 + n_bins + 1 + n_variables + 1)){
    JetCorrectionUtilities::handleError("JetResolutionObject::Definition::Definition", "Invalid file format. Please check.");
  }

  for (size_t i = 0; i < n_variables; i++){
    m_variables_name.push_back(tokens[n_bins + 2 + i]);
  }

  m_formula_str = tokens[n_bins + n_variables + 2];

  std::string formula_str_lower = m_formula_str;
  std::transform(formula_str_lower.begin(), formula_str_lower.end(), formula_str_lower.begin(), ::tolower);

  if (formula_str_lower == "none"){
    m_formula_str = "";

    if ((tokens.size() > n_bins + n_variables + 3) && (std::atoi(tokens[n_bins + n_variables + 3].c_str()))){
      size_t n_parameters = std::stoul(tokens[n_bins + n_variables + 3]);

      if (tokens.size() < (1 + n_bins + 1 + n_variables + 1 + 1 + n_parameters)){
        JetCorrectionUtilities::handleError("JetResolutionObject::Definition::Definition", "Invalid file format. Please check.");
      }

      for (size_t i = 0; i < n_parameters; i++){
        m_formula_str += tokens[n_bins + n_variables + 4 + i] + " ";
      }
    }
  }

  init();
}

void JetResolutionObject::Definition::init(){
  if (!m_formula_str.empty()){
    if (m_formula_str.find(' ') == std::string::npos)
      m_formula = std::make_shared<TFormula>("jet_resolution_formula", m_formula_str.c_str());
    else
      m_parameters_name = JetCorrectionUtilities::getTokens(m_formula_str);
  }
  for (auto const& bin : m_bins_name){
    auto b = JetParameters::binning_to_string.findBySecond(bin);
    if (b == JetParameters::binning_to_string.pairs.cend()){
      JetCorrectionUtilities::handleError("JetResolutionObject::Definition::init", "Bin name not supported: '" + bin + "'");
    }
    m_bins.push_back(b->first);
  }

  for (auto const& v : m_variables_name){
    auto var = JetParameters::binning_to_string.findBySecond(v);
    if (var == JetParameters::binning_to_string.pairs.cend()){
      JetCorrectionUtilities::handleError("JetResolutionObject::Definition::init", "Variable name not supported: '" + v + "'");
    }
    m_variables.push_back(var->first);
  }
}

JetResolutionObject::Record::Record(const std::string& line, const Definition& def){
  std::vector<std::string> tokens = JetCorrectionUtilities::getTokens(line);

  if (tokens.size() < (def.nBins() * 2 + def.nVariables() * 2 + 1)){
    JetCorrectionUtilities::handleError("JetResolutionObject::Record::Record", "Invalid record. Please check file format. Record: " + line);
  }

  size_t pos = 0;

  for (size_t i = 0; i < def.nBins(); i++){
    Range r(std::stof(tokens[pos]), std::stof(tokens[pos + 1]));
    pos += 2;
    m_bins_range.push_back(r);
  }

  size_t n_parameters = std::stoul(tokens[pos++]);

  if (tokens.size() < (def.nBins() * 2 + def.nVariables() * 2 + 1 + (n_parameters - def.nVariables() * 2))){
    JetCorrectionUtilities::handleError("JetResolutionObject::Record::Record", "Invalid record. Please check file format. Record: " + line);
  }

  for (size_t i = 0; i < def.nVariables(); i++){
    Range r(std::stof(tokens[pos]), std::stof(tokens[pos + 1]));
    pos += 2;
    m_variables_range.push_back(r);
    n_parameters -= 2;
  }

  for (size_t i = 0; i < n_parameters; i++){
    m_parameters_values.push_back(std::stof(tokens[pos++]));
  }
}

JetResolutionObject::JetResolutionObject(const std::string& filename) : m_valid(false){
  std::ifstream f(filename);

  if (!f.good()){
    JetCorrectionUtilities::handleError("JetResolutionObject::JetResolutionObject", "Can't read input file '" + filename + "'");
  }

  for (std::string line; std::getline(f, line);){
    if ((line.empty()) || (line[0] == '#'))
      continue;

    std::string definition = JetCorrectionUtilities::getDefinitions(line);

    if (!definition.empty()){
      m_definition = Definition(definition);
    }
    else {
      m_records.push_back(Record(line, m_definition));
    }
  }

  m_valid = true;
}

JetResolutionObject::JetResolutionObject(const JetResolutionObject& object){
  m_definition = object.m_definition;
  m_records = object.m_records;
  m_valid = object.m_valid;

  m_definition.init();
}

void JetResolutionObject::dump() const{
  std::cout << "Definition: " << std::endl;
  std::cout << "    Number of binning variables: " << m_definition.nBins() << std::endl;
  std::cout << "        ";
  for (auto const& bin : m_definition.getBinsName()){
    std::cout << bin << ", ";
  }
  std::cout << std::endl;
  std::cout << "    Number of variables: " << m_definition.nVariables() << std::endl;
  std::cout << "        ";
  for (auto const& bin : m_definition.getVariablesName()){
    std::cout << bin << ", ";
  }
  std::cout << std::endl;
  std::cout << "    Formula: " << m_definition.getFormulaString() << std::endl;

  std::cout << std::endl << "Bin contents" << std::endl;

  for (auto const& record : m_records){
    std::cout << "    Bins" << std::endl;
    size_t index = 0;
    for (auto const& bin : record.getBinsRange()){
      std::cout << "        " << m_definition.getBinName(index) << " [" << bin.min << " - " << bin.max << "]"
        << std::endl;
      index++;
    }

    std::cout << "    Variables" << std::endl;
    index = 0;
    for (auto const& r : record.getVariablesRange()){
      std::cout << "        " << m_definition.getVariableName(index) << " [" << r.min << " - " << r.max << "] "
        << std::endl;
      index++;
    }

    std::cout << "    Parameters" << std::endl;
    index = 0;
    for (auto const& par : record.getParametersValues()){
      std::cout << "        Parameter #" << index << " = " << par << std::endl;
      index++;
    }
  }
}

void JetResolutionObject::saveToFile(const std::string& file) const{
  std::ofstream fout(file);
  fout.setf(std::ios::right);

  fout << "{" << m_definition.nBins();
  for (auto& bin : m_definition.getBinsName()) fout << "    " << bin;
  fout << "    " << m_definition.nVariables();
  for (auto& var : m_definition.getVariablesName()) fout << "    " << var;
  fout << "    " << (m_definition.getFormulaString().empty() ? "None" : m_definition.getFormulaString()) << "    Resolution}" << std::endl;
  for (auto& record : m_records){
    for (auto& r : record.getBinsRange()) fout << std::left << std::setw(15) << r.min << std::setw(15) << r.max << std::setw(15);
    fout << (record.nVariables() * 2 + record.nParameters()) << std::setw(15);
    for (auto& r : record.getVariablesRange()) fout << r.min << std::setw(15) << r.max << std::setw(15);
    for (auto& p : record.getParametersValues()) fout << p << std::setw(15);
    fout << std::endl << std::setw(0);
  }
}

const JetResolutionObject::Record* JetResolutionObject::getRecord(const JetParameters& bins_parameters) const{
  if (!m_valid) return nullptr;

  // Create vector of bins value. Throw if some values are missing
  std::vector<float> bins = bins_parameters.createVector(m_definition.getBins());

  // Iterate over all records, and find the one for which all bins are valid
  const Record* good_record = nullptr;
  for (auto const& record : m_records){
    // Iterate over bins
    size_t valid_bins = 0;
    size_t current_bin = 0;
    for (auto const& bin : record.getBinsRange()){
      if (bin.is_inside(bins[current_bin]))
        valid_bins++;

      current_bin++;
    }

    if (valid_bins == m_definition.nBins()){
      good_record = &record;
      break;
    }
  }

  return good_record;
}

float JetResolutionObject::evaluateFormula(const JetResolutionObject::Record& record, const JetParameters& variables_parameters) const{
  if (!m_valid)
    return 1;

  // Set parameters
  auto const* pFormula = m_definition.getFormula();
  if (!pFormula)
    return 1;
  auto formula = *pFormula;

  // Create vector of variables value. Throw if some values are missing
  std::vector<float> variables = variables_parameters.createVector(m_definition.getVariables());

  double variables_[4] ={ 0 };
  for (size_t index = 0; index < m_definition.nVariables(); index++){
    variables_[index] = JetCorrectionUtilities::clip(variables[index], record.getVariablesRange()[index].min, record.getVariablesRange()[index].max);
  }
  const std::vector<float>& parameters = record.getParametersValues();

  for (size_t index = 0; index < parameters.size(); index++){
    formula.SetParameter(index, parameters[index]);
  }

  return formula.EvalPar(variables_);
}
