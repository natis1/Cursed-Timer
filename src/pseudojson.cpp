/*
 * Biome, A simple ncurses productivity program, made while procrastinating.
 * Copyright (C) 2019  Eli Stone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tools.h"

using namespace pseudojson;

std::string pseudojson::getValue(const pseudojson::ValueData* v)
{
    std::string prefix = "";
    std::string affix = "";
    std::string val = "";
    if (v->lon != UNSET_INTVAL) {
        prefix = "l";
        val = std::to_string(v->lon);
    } else if (v->d != UNSET_DOUBLEVAL) {
        prefix = "d";
        val = std::to_string(v->d);
    } else if (v->string.length() > 0) {
        prefix = "\"";
        affix = "\"";
        val = v->string;
    }

    return prefix + val + affix;
}

std::vector<std::string> pseudojson::split(const std::string &text, char sep) {
    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
    return tokens;
}

std::pair<std::string, pseudojson::Value> pseudojson::stringToValue(std::string valueLine)
{
    std::vector<std::string> x = pseudojson::split(valueLine, ':');
    x.at(0) = x.at(0).substr(0, x.at(0).size()-1);
    x.at(1).erase(0, 1);

    pseudojson::Value val;

    switch (x.at(1).at(0)) {
        case '"':
            x.at(1).erase(0, 1);
            x.at(1) = x.at(1).substr(0, x.at(1).size()-1);
            val.data.string = x.at(1);
            break;
        case 'l':
            x.at(1).erase(0, 1);
            val.data.lon = std::stol(x.at(1));
            break;
        case 'd':
            x.at(1).erase(0, 1);
            val.data.d = std::stod(x.at(1));
            break;
    }
    if (x.at(1).at(0) == '"') {
        x.at(1).erase(0, 1);
        x.at(1) = x.at(1).substr(1, x.at(1).size()-1);
        val.data.string = x.at(1);
    }
    return std::make_pair(x.at(0), val);
}

pseudojson::Value pseudojson::fileToPseudoJson(std::string filename)
{
    std::ifstream in(filename);
    std::string str;

    pseudojson::Value dat;

    while (std::getline(in, str)) {
        if (str.size() > 3) {
            auto pair = stringToValue(str);
            dat.data.subObject.insert(pair);
        }
    }
    return dat;
}

bool pseudojson::writeToFile(pseudojson::Value val, std::string filename)
{
    try {
        std::ofstream os(filename);
        for( auto const& [key, val] : val.data.subObject ) {
            os << key << " : " << pseudojson::getValue(&val.data) << std::endl;
        }
        os.close();
    } catch (std::exception e) {
        return 0;
    }
    return 1;
}





template<>
long& pseudojson::asAny<long>(Value& value) {
    return value.data.lon;
}

template<>
const long& pseudojson::asAny<long>(const Value& value) {
    return value.data.lon;
}

template<>
const std::string& pseudojson::asAny<std::string>(const Value& value) {
    return value.data.string;
}

template<>
std::string& pseudojson::asAny<std::string>(Value& value) {
    return value.data.string;
}

template<>
double& pseudojson::asAny<double>(Value& value) {
    return value.data.d;
}

template<>
const double& pseudojson::asAny<double>(const Value& value) {
    return value.data.d;
}
