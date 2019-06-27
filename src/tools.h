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

#pragma once
#define UNSET_INTVAL -9969420
#define UNSET_DOUBLEVAL -9969.420

#include <array>
#include <tuple>
#include <map>
#include <iostream>
#include <vector>
#include <fstream>

// sequence for
template <typename T, T... S, typename F>
constexpr void for_sequence(std::integer_sequence<T, S...>, F&& f) {
	using unpack_t = long[];
	(void)unpack_t{(static_cast<void>(f(std::integral_constant<T, S>{})), 0)..., 0};
}

// Sample implementation of a json-like data structure. It is only there for the example to compile and actually produce a testable output
namespace pseudojson {
    struct Value;

    struct ValueData {
        std::map<std::string, Value> subObject;
        std::string string;
        long lon = UNSET_INTVAL;
        double d = UNSET_DOUBLEVAL;
    };

    struct Value {
        ValueData data;

        Value& operator[](std::string name) {
            return data.subObject[std::move(name)];
        }

        const Value& operator[](std::string name) const {
            auto it = data.subObject.find(std::move(name));

            if (it != data.subObject.end()) {
                return it->second;
            }

            throw;
        }

        Value& operator=(std::string value) {
            data.string = value;
            return *this;
        }

        Value& operator=(long value) {
            data.lon = value;
            return *this;
        }

        Value& operator=(double value) {
            data.d = value;
            return *this;
        }

        Value& operator=(int value) {
            data.lon = value;
            return *this;
        }
    };

    std::string getValue(const ValueData *v);
    std::pair<std::string, Value> stringToValue(std::string valueLine);
    pseudojson::Value fileToPseudoJson(std::string filename);

    std::vector<std::string> split(const std::string &s, char delim);
    bool writeToFile(Value val, std::string filename);

    template<typename T> T& asAny(Value&);
    template<typename T> const T& asAny(const Value&);

    template<>
    long& asAny<long>(Value& value);

    template<>
    const long& asAny<long>(const Value& value);

    template<>
    const std::string& asAny<std::string>(const Value& value);

    template<>
    std::string& asAny<std::string>(Value& value);

    template<>
    double& asAny<double>(Value& value);

    template<>
    const double& asAny<double>(const Value& value);
}

template<typename Class, typename T>
struct PropertyImpl {
    constexpr PropertyImpl(T Class::*aMember, const char* aName) : member{aMember}, name{aName} {}

    using Type = T;

    T Class::*member;
    const char* name;
};

// One could overload this function to accept both a getter and a setter instead of a member.
template<typename Class, typename T>
constexpr auto property(T Class::*member, const char* name) {
    return PropertyImpl<Class, T>{member, name};
}


// unserialize function
template<typename T>
T fromJson(const pseudojson::Value& data) {
    T object;

    // We first get the number of properties
    constexpr auto nbProperties = std::tuple_size<decltype(T::properties)>::value;

    // We iterate on the index sequence of size `nbProperties`
    for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i){
        // get the property
        constexpr auto property = std::get<i>(T::properties);

        // get the type of the property
        using Type = typename decltype(property)::Type;

        // set the value to the member
        object.*(property.member) = pseudojson::asAny<Type>(data[property.name]);
    });

    return object;
}

template<typename T>
pseudojson::Value toJson(const T& object) {
    pseudojson::Value data;

    // We first get the number of properties
    constexpr auto nbProperties = std::tuple_size<decltype(T::properties)>::value;

    // We iterate on the index sequence of size `nbProperties`
    for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i){
        // get the property
        constexpr auto property = std::get<i>(T::properties);

        // set the value to the member
        data[property.name] = object.*(property.member);
    });

    return data;
}
