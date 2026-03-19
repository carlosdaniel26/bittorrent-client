#include "Bencode.h"
#include <stdexcept>

Bencode::Value Bencode::parseInteger(const std::string& input, size_t& pos)
{
    Bencode::Value value;
    value.type = Bencode::Value::Type::Integer;

    pos++; // skip 'i'

    size_t start = pos;

    while (input[pos] != 'e') {
        pos++;
    }

    /* convert */
    std::string int_str = input.substr(start, pos - start);
    value.int_value = std::stoll(int_str);

    pos++; /* skip 'e' (end of integer) */

    return value;
}

Bencode::Value Bencode::parseString(const std::string& input, size_t& pos)
{
    Bencode::Value value;
    value.type = Bencode::Value::Type::String;

    size_t start = pos;

    while (input[pos] != ':') {
        pos++;
    }

    /* convert */
    std::string len_str = input.substr(start, pos - start);
    size_t len = std::stoul(len_str);

    pos++; /* skip ':' */

    value.string_value = input.substr(pos, len);
    pos += len; /* skip string */

    return value;
}

Bencode::Value Bencode::parseList(const std::string& input, size_t& pos)
{
    Bencode::Value value;
    value.type = Bencode::Value::Type::List;

    pos++; /* skip 'l' */

    while (input[pos] != 'e') {
        value.list_value.push_back(parseValue(input, pos));
    }

    pos++; /* skip 'e' (end of list) */

    return value;
}

Bencode::Value Bencode::parseDictionary(const std::string& input, size_t& pos)
{
    Bencode::Value value;
    value.type = Bencode::Value::Type::Dictionary;

    pos++; /* skip 'd' */

    while (input[pos] != 'e') {
        Bencode::Value key = parseString(input, pos);
        Bencode::Value val = parseValue(input, pos);
        value.dict_value[key.string_value] = val;
    }

    pos++; /* skip 'e' (end of dictionary) */

    return value;
}

Bencode::Value Bencode::parseValue(const std::string& input, size_t& pos)
{
    if (input[pos] == 'i') {
        return parseInteger(input, pos);
    }
    if (input[pos] == 'l') {
        return parseList(input, pos);
    }
    if (input[pos] >= '0' && input[pos] <= '9') {
        return parseString(input, pos);
    }
    if (input[pos] == 'd') {
        return parseDictionary(input, pos);
    }
    
    throw std::runtime_error("Unexpected value type");
}

std::string Bencode::serialize(const Bencode::Value& value)
{
    switch (value.type) {
        case Bencode::Value::Type::Integer:
            return "i" + std::to_string(value.int_value) + "e";
        case Bencode::Value::Type::String:
            return std::to_string(value.string_value.size()) + ":" + value.string_value;
        case Bencode::Value::Type::List: {
            std::string result = "l";
            for (const auto& item : value.list_value) {
                result += serialize(item);
            }
            result += "e";
            return result;
        }
        case Bencode::Value::Type::Dictionary: {
            std::string result = "d";
            for (const auto& pair : value.dict_value) {
                result += serialize(Bencode::Value{Bencode::Value::Type::String, 0, pair.first}) + serialize(pair.second);
            }
            result += "e";
            return result;
        }
    }
    throw std::runtime_error("Invalid value type");
}