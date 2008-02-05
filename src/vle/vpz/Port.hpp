/**
 * @file src/vle/vpz/Port.hpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
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




#ifndef VLE_VPZ_PORT_HPP
#define VLE_VPZ_PORT_HPP

#include <vle/vpz/Base.hpp>

namespace vle { namespace vpz {

    class In : public Base
    {
    public:
        In() { }

        virtual ~In() { }

        virtual void write(std::ostream& /* out */) const
        { }

        virtual Base::type getType() const
        { return IN; }
    };

    class Out : public Base
    {
    public:
        Out() { }

        virtual ~Out() { }

        virtual void write(std::ostream& /* out */) const
        { }

        virtual Base::type getType() const
        { return OUT; }
    };

    class Init : public Base
    {
    public:
        Init() { }

        virtual ~Init() { }

        virtual void write(std::ostream& /* out */) const
        { }

        virtual Base::type getType() const
        { return INIT; }
    };

    class State : public Base
    {
    public:
        State() { }

        virtual ~State() { }

        virtual void write(std::ostream& /* out */) const
        { }

        virtual Base::type getType() const
        { return STATE; }
    };

    class Port : public Base
    {
    public:
        Port() { }

        virtual ~Port() { }

        virtual void write(std::ostream& /* out */) const
        { }

        virtual Base::type getType() const
        { return PORT; }
    };

}} // namespace vle vlz

#endif
