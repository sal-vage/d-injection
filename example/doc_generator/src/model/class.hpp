//          Copyright Adam Lach 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef CLASS_HPP
#define CLASS_HPP

#include <string>
#include "model/method.hpp"

namespace model {

struct Class {
    std::string description;
	std::string filename;
	std::string name;
    std::string space;
    std::string signature;
	std::vector<Method> methods;
};

} // namespace Model

#endif // CLASS_HPP