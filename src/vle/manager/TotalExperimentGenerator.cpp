/*
 * @file vle/manager/TotalExperimentGenerator.cpp
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


#include <vle/manager/TotalExperimentGenerator.hpp>
#include <vle/value/Set.hpp>



namespace vle { namespace manager {

void TotalExperimentGenerator::buildCombination(size_t& nb)
{
    size_t sz = mCondition.size() - 1;

    if (mCondition[sz].pos != mCondition[sz].sz - 1) {
        mCondition[sz].pos++;
    } else {
        int i = sz;

        while (i >= 0) {
            if (mCondition[i].pos == mCondition[i].sz - 1) {
                mCondition[i].pos = 0;
                i--;
            } else {
                mCondition[i].pos++;
                break;
            }
        }
    }
    nb++;
}

size_t TotalExperimentGenerator::getCombinationNumber() const
{
    size_t nb = 1;

    std::vector < cond_t >::const_iterator it;
    for (it = mCondition.begin(); it != mCondition.end(); ++it) {
        nb *= it->sz;
    }

    return nb;
}

}} // namespace vle manager
