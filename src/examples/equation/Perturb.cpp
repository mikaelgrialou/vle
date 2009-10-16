/**
 * @file examples/equation/Perturb.cpp
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


#include <vle/extension/DifferenceEquation.hpp>
#include <vle/extension/Statechart.hpp>

using namespace boost::assign;

namespace vle { namespace examples { namespace equation {

namespace ve = vle::extension;
namespace vd = vle::devs;

enum State { A, B };

class Perturb : public ve::Statechart
{
public:
    Perturb(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        ve::Statechart(init, events)
	{
	    ve::states(this) << A << B;

	    ve::transition(this, A, B) << ve::after(5.)
				       << ve::outputFunc(&Perturb::out1);
	    ve::transition(this, B, A) << ve::after(5.)
				       << ve::outputFunc(&Perturb::out2);

	    initialState(A);
	}

    virtual ~Perturb() { }

    void out1(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("a") = 10);
	}

    void out2(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("a") = 0);
	}

};

class Perturb2 : public ve::Statechart
{
public:
    Perturb2(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        ve::Statechart(init, events)
	{
	    ve::states(this) << A << B;

	    ve::transition(this, A, B) << ve::after(5.)
				       << ve::outputFunc(&Perturb2::out1);
	    ve::transition(this, B, A) << ve::after(5.)
				       << ve::outputFunc(&Perturb2::out2);

	    initialState(A);
	}

    virtual ~Perturb2() { }

    void out1(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output.addEvent(buildEvent("out1"));
	}

    void out2(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output.addEvent(buildEvent("out2"));
	}

};

class Perturb3 : public ve::Statechart
{
public:
    Perturb3(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        ve::Statechart(init, events)
	{
	    ve::states(this) << A;

	    ve::transition(this, A, A) << ve::event("in1")
				       << ve::outputFunc(&Perturb3::out1);
	    ve::transition(this, A, A) << ve::event("in2")
				       << ve::outputFunc(&Perturb3::out2);

	    initialState(A);
	}

    virtual ~Perturb3() { }

    void out1(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("a") = 10);
	}

    void out2(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("a") = 0);
	}

};

class Perturb4 : public ve::Statechart
{
public:
    Perturb4(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        ve::Statechart(init, events)
	{
	    ve::states(this) << A;

	    ve::transition(this, A, A) << ve::event("in1")
				       << ve::outputFunc(&Perturb4::out1);
	    ve::transition(this, A, A) << ve::event("in2")
				       << ve::outputFunc(&Perturb4::out2);

	    initialState(A);
	}

    virtual ~Perturb4() { }

    void out1(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("c") = 10);
	}

    void out2(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("c") = 0);
	}

};

class Perturb5 : public ve::Statechart
{
public:
    Perturb5(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        ve::Statechart(init, events)
	{
	    ve::states(this) << A;

	    ve::transition(this, A, A) << ve::event("in1")
				       << ve::outputFunc(&Perturb5::out1);
	    ve::transition(this, A, A) << ve::event("in2")
				       << ve::outputFunc(&Perturb5::out2);

	    initialState(A);
	}

    virtual ~Perturb5() { }

    void out1(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output.addEvent(buildEvent("out1"));
	}

    void out2(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output.addEvent(buildEvent("out2"));
	}

};

class Perturb6 : public ve::Statechart
{
public:
    Perturb6(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        ve::Statechart(init, events)
	{
	    ve::states(this) << A << B;

	    ve::transition(this, A, B) << ve::after(4.5)
				       << ve::outputFunc(&Perturb6::out1);
	    ve::transition(this, B, A) << ve::after(3.2)
				       << ve::outputFunc(&Perturb6::out2);

	    initialState(A);
	}

    virtual ~Perturb6() { }

    void out1(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("a") = 10);
	}

    void out2(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    output << (ve::DifferenceEquation::Var("a") = 0);
	}

};

class Perturb7 : public ve::Statechart
{
public:
    Perturb7(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        ve::Statechart(init, events)
	{
	    ve::states(this) << A;

	    ve::transition(this, A, A) << ve::after(5.)
				       << ve::outputFunc(&Perturb7::out);

	    initialState(A);
	}

    virtual ~Perturb7() { }

    void out(const vd::Time& /* time */, vd::ExternalEventList& output) const
	{
	    vd::ExternalEvent* ee = new vd::ExternalEvent("out");

	    ee << vd::attribute("name", std::string("a"));
            ee << vd::attribute("value", 2.);
            ee << vd::attribute("type", ve::DifferenceEquation::ADD);
	    output.addEvent(ee);
	}

};

}}} // namespace vle examples equation

DECLARE_NAMED_DYNAMICS(Perturb, vle::examples::equation::Perturb)
DECLARE_NAMED_DYNAMICS(Perturb2, vle::examples::equation::Perturb2)
DECLARE_NAMED_DYNAMICS(Perturb3, vle::examples::equation::Perturb3)
DECLARE_NAMED_DYNAMICS(Perturb4, vle::examples::equation::Perturb4)
DECLARE_NAMED_DYNAMICS(Perturb5, vle::examples::equation::Perturb5)
DECLARE_NAMED_DYNAMICS(Perturb6, vle::examples::equation::Perturb6)
DECLARE_NAMED_DYNAMICS(Perturb7, vle::examples::equation::Perturb7)
