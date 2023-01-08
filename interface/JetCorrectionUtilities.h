#ifndef JETCORRECTIONUTILITIES_H
#define JETCORRECTIONUTILITIES_H

#include <string>
#include <cmath>
#include <vector>
#include <utility>
#include <initializer_list>


namespace JetCorrectionUtilities{
  void handleError(const std::string& fClass, const std::string& fMessage);
  float getFloat(const std::string& token);
  unsigned getUnsigned(const std::string& token);
  long int getSigned(const std::string& token);
  std::string getSection(const std::string& token);
  std::vector<std::string> getTokens(const std::string& fLine);
  std::string getDefinitions(const std::string& token);
  float quadraticInterpolation(float fZ, const float fX[3], const float fY[3]);

  template<typename T> T clip(T const& n, T const& lower, T const& upper){ return std::max(lower, std::min(n, upper)); }

  template<typename T, typename U> struct bijection_map{
    typedef std::pair<T, U> value_type;
    typedef typename std::vector<value_type>::iterator value_type_iterator;
    typedef typename std::vector<value_type>::const_iterator value_type_const_iterator;

    std::vector<value_type> pairs;

    bijection_map(){}
    bijection_map(std::initializer_list<value_type> const& ll){
      std::vector<T> keys;
      std::vector<U> values;
      for (auto const& vv:ll){
        if (std::find(keys.cbegin(), keys.cend(), vv.first)!=keys.cend()) handleError("bijection_map::bijection_map", "Keys need to be unique, but they are repeated.");
        if (std::find(values.cbegin(), values.cend(), vv.second)!=values.cend()) handleError("bijection_map::bijection_map", "Values need to be unique, but they are repeated.");
        pairs.push_back(vv);
        keys.push_back(vv.first);
        values.push_back(vv.second);
      }
    }
    bijection_map(bijection_map<T, U> const& other) : pairs(other.pairs){}
    bijection_map(bijection_map<T, U>&& other) : pairs(std::move(other.pairs)){}

    bijection_map& operator=(bijection_map const& other){
      pairs = other.pairs;
      return *this;
    }
    bijection_map& operator=(bijection_map&& other){
      pairs = std::move(other.pairs);
      return *this;
    }

    value_type_iterator findByFirst(T const& key){
      for (value_type_iterator it=pairs.begin(); it!=pairs.end(); it++){
        if (it->first == key) return it;
      }
      return pairs.end();
    }
    value_type_const_iterator findByFirst(T const& key) const{
      for (value_type_const_iterator it=pairs.cbegin(); it!=pairs.cend(); it++){
        if (it->first == key) return it;
      }
      return pairs.cend();
    }

    value_type_iterator findBySecond(U const& key){
      for (value_type_iterator it=pairs.begin(); it!=pairs.end(); it++){
        if (it->second == key) return it;
      }
      return pairs.end();
    }
    value_type_const_iterator findBySecond(U const& key) const{
      for (value_type_const_iterator it=pairs.cbegin(); it!=pairs.cend(); it++){
        if (it->second == key) return it;
      }
      return pairs.cend();
    }
  };
}

#endif
