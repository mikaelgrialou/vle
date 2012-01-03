/*
 * @file examples/qss/ladybird2.hpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2012 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2012 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
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


#ifndef EXAMPLES_QSS_LADYBIRD_HPP
#define EXAMPLES_QSS_LADYBIRD_HPP 1

#include <vle/extension/difference-equation/Simple.hpp>

namespace vle { namespace examples { namespace qss {

class Ladybird2 : public vle::extension::DifferenceEquation::Simple
{
public:
    Ladybird2(const vle::devs::DynamicsInit& model,
	      const vle::devs::InitEventList& events);

    virtual ~Ladybird2();

    virtual double compute(const vle::devs::Time& /* time */);

private:
    Var y;
    Sync x;

    double b;
    double d;
    double e;
};

}}} // namespace vle examples qss
#endif
