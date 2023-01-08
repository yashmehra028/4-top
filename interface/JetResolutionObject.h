#ifndef JETRESOLUTIONOBJECT_H
#define JETRESOLUTIONOBJECT_H

#include <unordered_map>
#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <initializer_list>
#include "TFormula.h"
#include "JESRHelpers.h"
#include "JetCorrectionUtilities.h"


class JetParameters{
public:
  typedef std::unordered_map<JESRHelpers::JERBinning, float> value_type;
  static const JetCorrectionUtilities::bijection_map<JESRHelpers::JERBinning, std::string> binning_to_string;

private:
  value_type m_values;

public:
  JetParameters() = default;
  JetParameters(JetParameters&& rhs);
  JetParameters(std::initializer_list<typename value_type::value_type> const& init);

  JetParameters& setJetPt(float pt);
  JetParameters& setJetEta(float eta);
  JetParameters& setJetE(float e);
  JetParameters& setJetArea(float area);
  JetParameters& setMu(float mu);
  JetParameters& setRho(float rho);
  JetParameters& setNPV(float npv);
  JetParameters& set(const JESRHelpers::JERBinning& bin, float value);
  JetParameters& set(const typename value_type::value_type& value);

  std::vector<float> createVector(const std::vector<JESRHelpers::JERBinning>& binning) const;

};

class JetResolutionObject{
public:
  struct Range{
    float min;
    float max;

    Range(){}

    Range(float min, float max){
      this->min = min;
      this->max = max;
    }

    bool is_inside(float value) const { return (value >= min) && (value < max); }
  };

  class Definition{
  private:
    std::vector<std::string> m_bins_name;
    std::vector<std::string> m_variables_name;
    std::string m_formula_str;

    std::shared_ptr<TFormula> m_formula;

    std::vector<JESRHelpers::JERBinning> m_bins;
    std::vector<JESRHelpers::JERBinning> m_variables;
    std::vector<std::string> m_parameters_name;

  public:
    Definition(){}

    Definition(const std::string& definition);

    const std::vector<std::string>& getBinsName() const { return m_bins_name; }

    const std::vector<JESRHelpers::JERBinning>& getBins() const { return m_bins; }

    std::string getBinName(size_t bin) const { return m_bins_name[bin]; }

    size_t nBins() const { return m_bins_name.size(); }

    const std::vector<std::string>& getVariablesName() const { return m_variables_name; }

    const std::vector<JESRHelpers::JERBinning>& getVariables() const { return m_variables; }

    std::string getVariableName(size_t variable) const { return m_variables_name[variable]; }

    size_t nVariables() const { return m_variables.size(); }

    const std::vector<std::string>& getParametersName() const { return m_parameters_name; }

    size_t nParameters() const { return m_parameters_name.size(); }

    std::string getFormulaString() const { return m_formula_str; }

    TFormula const* getFormula() const { return m_formula.get(); }

    void init();
  };

  class Record{
  private:
    std::vector<Range> m_bins_range;
    std::vector<Range> m_variables_range;
    std::vector<float> m_parameters_values;

  public:
    Record(){}

    Record(const std::string& record, const Definition& def);

    const std::vector<Range>& getBinsRange() const { return m_bins_range; }

    const std::vector<Range>& getVariablesRange() const { return m_variables_range; }

    const std::vector<float>& getParametersValues() const { return m_parameters_values; }

    size_t nVariables() const { return m_variables_range.size(); }

    size_t nParameters() const { return m_parameters_values.size(); }
  };

private:
  Definition m_definition;
  std::vector<Record> m_records;

  bool m_valid;

public:
  JetResolutionObject(const std::string& filename);
  JetResolutionObject(const JetResolutionObject& filename);
  JetResolutionObject() : m_valid(false){}

  void dump() const;
  void saveToFile(const std::string& file) const;

  const Record* getRecord(const JetParameters& bins) const;
  float evaluateFormula(const Record& record, const JetParameters& variables) const;

  const std::vector<Record>& getRecords() const { return m_records; }

  const Definition& getDefinition() const { return m_definition; }
};


#endif
