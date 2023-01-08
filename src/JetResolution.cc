#include <sstream>
#include <algorithm>
#include "JetResolution.h"


JetResolution::JetResolution(const std::string& filename){ m_object = std::make_shared<JetResolutionObject>(filename); }
JetResolution::JetResolution(const JetResolutionObject& object){ m_object = std::make_shared<JetResolutionObject>(object); }

float JetResolution::getResolution(const JetParameters& parameters) const {
  const JetResolutionObject::Record* record = m_object->getRecord(parameters);
  if (!record) return 1;
  return m_object->evaluateFormula(*record, parameters);
}

JetResolutionScaleFactor::JetResolutionScaleFactor(const std::string& filename){ m_object = std::make_shared<JetResolutionObject>(filename); }
JetResolutionScaleFactor::JetResolutionScaleFactor(const JetResolutionObject& object){ m_object = std::make_shared<JetResolutionObject>(object); }

float JetResolutionScaleFactor::getScaleFactor(
  const JetParameters& parameters,
  JESRHelpers::JERVariation variation,
  std::string uncertaintySource
) const{
  const JetResolutionObject::Record* record = m_object->getRecord(parameters);
  if (!record) return 1;

  std::vector<float> const& parameters_values = record->getParametersValues();
  std::vector<std::string> const& parameter_names = m_object->getDefinition().getParametersName();
  size_t parameter = static_cast<size_t>(variation);
  if (!uncertaintySource.empty()){
    std::string uncname;
    if (variation == JESRHelpers::kJERDown) uncname = uncertaintySource + "Down";
    else if (variation == JESRHelpers::kJERUp) uncname = uncertaintySource + "Up";
    parameter = parameter_names.size();
    if (uncname != "") parameter = std::distance(
      parameter_names.cbegin(),
      std::find(parameter_names.cbegin(), parameter_names.cend(), uncname)
    );
    if (parameter >= parameter_names.size()){
      std::stringstream ss;
      ss << "Invalid input uncertaintySource='" << uncertaintySource << "'. Only {";
      for (const auto& piece : parameter_names) ss << " '" << piece << "'";
      ss << " } are supported.";
      JetCorrectionUtilities::handleError("JetResolutionScaleFactor::getScaleFactor", ss.str());
    }
  }
  return parameters_values.at(parameter);
}
