#pragma once 

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class Bencode {
 public:
  struct Value {

    enum class Type {
      Integer,
      String,
      List,
      Dictionary
    } type;

    /* pseudo union */
    int64_t int_value;
    std::string string_value;
    std::vector<Value> list_value;
    std::map<std::string, Value> dict_value;

    Value() = default;
  };

  Bencode::Value parseValue(const std::string& input, size_t& pos);
   std::string serialize(const Bencode::Value& value);

 private:
  Bencode::Value parseInteger(const std::string& input, size_t& pos);
  Bencode::Value parseString(const std::string& input, size_t& pos);
  Bencode::Value parseList(const std::string& input, size_t& pos);
  Bencode::Value parseDictionary(const std::string& input, size_t& pos);

};