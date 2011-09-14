/*
 * @file examples/decision/FailedActivity.cpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2010 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2010 INRA http://www.inra.fr
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

#include <vle/devs/DynamicsDbg.hpp>
#include <vle/extension/decision/Agent.hpp>
#include <vle/extension/decision/Activity.hpp>

namespace vd = vle::devs;
namespace vv = vle::value;
namespace vmd = vle::extension::decision;

namespace vle { namespace examples { namespace decision {

/**
 * @brief Simple facts model that wakes up the Agents
 * each unit time.
 */
class Facts : public vd::Dynamics
{
public:
    Facts(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
        : vd::Dynamics(mdl, evts)
    {
    }

    virtual ~Facts()
    {
    }

    virtual vd::Time init(const vd::Time& /*time*/)
    {
        return 0;
    }

    virtual vd::Time timeAdvance() const
    {
        return 1.0;
    }

    virtual void output(const vd::Time& /*time*/,
                        vd::ExternalEventList& output) const
    {
        vd::ExternalEvent* ev = new vd::ExternalEvent("fact");
        ev->putAttribute("value", new vv::Boolean(true));
        output.addEvent(ev);
    }
};
/**
 * @brief DEVS model that sends an ack signal "done".
 * Parameters : time of send and name of activity
 */
class Ack : public vd::Dynamics
{
public:
    Ack(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
        : vd::Dynamics(mdl, evts)
    {
        timeOfSend = evts.getDouble("timeOfSend");
        activity = evts.getString("activity");
    }
    virtual ~Ack()
    {
    }
    virtual vd::Time init(const vd::Time& /*time*/)
    {
        return timeOfSend;
    }
    virtual vd::Time timeAdvance() const
    {
        return vd::Time::infinity;
    }
    virtual void output(const vd::Time& /*time*/,
                        vd::ExternalEventList& output) const
    {
        vd::ExternalEvent* ev = new vd::ExternalEvent("ack");
        ev->putAttribute("name", new vv::String(activity));
        ev->putAttribute("value", new vv::String("done"));
        output.addEvent(ev);
    }
    double timeOfSend;
    std::string activity;
};

/**
 * @brief Base class for agents with rules for failure.
 */
class FailedActivity : public vmd::Agent
{
public:
	FailedActivity(const vd::DynamicsInit& mdl,
        const vd::InitEventList& evts)
        : vmd::Agent(mdl, evts)
    {
    }

    virtual ~FailedActivity()
    {
    }
    /*****
     * facts
     *****/
    void fact(const vle::value::Value& /*val*/)
    {
    }
    /*****
     * predicats
     *****/
    bool failA() const
    {
        return currentTime() == dateFailedA;
    }
    bool validateA() const
    {
        return currentTime() == dateValidateA;
    }
    bool validateB() const
    {
        return currentTime() == dateValidateB;
    }

    virtual vv::Value* observation(const vd::ObservationEvent& evt) const
    {
        if (evt.onPort("A")) {
            std::stringstream out;
            out << (activities().get("A")->second.state());
            return new vv::String(out.str());
        }
        if (evt.onPort("B")) {
            std::stringstream out;
            out << (activities().get("B")->second.state());
            return new vv::String(out.str());
        }
        return 0;
    }
protected:
    vd::Time dateValidateA;
    vd::Time dateFailedA;
    vd::Time dateValidateB;
    vd::Time dateAckB;
};

/**
 * @brief Agent1, only one activity that is failed while it is in
 * STARTED state. Expected :
 * t=1 -> validate A
 * t=5 -> fail A
 * state dynamic A : 0--WAIT--1--STARTED--5--FAILED--10
 */
class Agent1 : public FailedActivity
{
public:
    Agent1(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
        : FailedActivity(mdl, evts)
    {
        //initialisation of times
        dateValidateA = 1;
        dateFailedA = 5;

        //inititialisation du KnowledgeBase
        addFacts(this) +=
                F("fact", &FailedActivity::fact);

        addPredicates(this) +=
                P("failA", &FailedActivity::failA),
                P("validateA", &FailedActivity::validateA);

        //building rules
        vmd::Rule& r = addRule("ruleValidateA");
        r.add(predicates()["validateA"]);
        vmd::Rule& r2 = addRule("ruleFailA");
        r2.add(predicates()["failA"]);
        //building activities
        vmd::Activity& a = addActivity("A", 0.0, 10.0);
        a.addRule("ruleValidateA", r);
        a.addRuleFailure("ruleFailA", r2);
    }
};

/**
 * @brief Agent2, only one activity that is failed while it is in
 * WAIT state. Expected :
 * t=1 -> fail A
 * t=5 -> validate A
 * state dynamic A : 0--WAIT--1--FAILED--10
 */
class Agent2 : public FailedActivity
{
public:
    Agent2(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
        : FailedActivity(mdl, evts)
    {
        //initialisation of times
        dateValidateA = 5;
        dateFailedA = 1;

        //inititialisation du KnowledgeBase
        addFacts(this) +=
                F("fact", &FailedActivity::fact);

        addPredicates(this) +=
                P("failA", &FailedActivity::failA),
                P("validateA", &FailedActivity::validateA);

        //building rules
        vmd::Rule& r = addRule("ruleValidateA");
        r.add(predicates()["validateA"]);
        vmd::Rule& r2 = addRule("ruleFailA");
        r2.add(predicates()["failA"]);
        //building activities
        vmd::Activity& a = addActivity("A", 0.0, 10.0);
        a.addRule("ruleValidateA", r);
        a.addRuleFailure("ruleFailA", r2);
    }
};


/**
 * @brief Agent3, only one activity that is failed and validate
 * at the same time. Expected :
 * t=1 -> validate A
 * t=1 -> fail A
 * state dynamic A : 0--WAIT--1--FAILED--10
 */
class Agent3 : public FailedActivity
{
public:
    Agent3(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
        : FailedActivity(mdl, evts)
    {
        //initialisation of dates
        dateValidateA = 1;
        dateFailedA = 1;

        //inititialisation du KnowledgeBase
        addFacts(this) +=
                F("fact", &FailedActivity::fact);

        addPredicates(this) +=
                P("failA", &FailedActivity::failA),
                P("validateA", &FailedActivity::validateA);

        //building rules
        vmd::Rule& r = addRule("ruleValidateA");
        r.add(predicates()["validateA"]);
        vmd::Rule& r2 = addRule("ruleFailA");
        r2.add(predicates()["failA"]);
        //building activities
        vmd::Activity& a = addActivity("A", 0.0, 10.0);
        a.addRule("ruleValidateA", r);
        a.addRuleFailure("ruleFailA", r2);
    }
};

/**
 * @brief Agent4, 2 activity with a FF(A,B) precedence constraint.
 * Expected :
 * t=1 -> validate A and B
 * t=5 -> ack sur B
 * t=6 -> fail A
 * state dynamic A : 0--WAIT--1--STARTED------------6--FAILED--10
 * state dynamic B : 0--WAIT--1--STARTED--5--FAILED------------10
 */
class Agent4 : public FailedActivity
{
public:
    Agent4(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
        : FailedActivity(mdl, evts)
    {
        //initialisation of times
        dateValidateA = 1;
        dateValidateB = 1;
        dateFailedA = 6;

        //initialisation of KnowledgeBase
        addFacts(this) +=
                F("fact", &FailedActivity::fact);

        addPredicates(this) +=
                P("failA", &FailedActivity::failA),
                P("validateA", &FailedActivity::validateA),
                P("validateB", &FailedActivity::validateB);

        //building rules
        vmd::Rule& rvA = addRule("ruleValidateA");
        rvA.add(predicates()["validateA"]);
        vmd::Rule& riA = addRule("ruleFailA");
        riA.add(predicates()["failA"]);
        vmd::Rule& rvB = addRule("ruleValidateB");
        rvB.add(predicates()["validateB"]);

        //building activities
        vmd::Activity& a = addActivity("A", 0.0, 10.0);
        a.addRule("ruleValidateA", rvA);
        a.addRuleFailure("ruleFailA", riA);
        vmd::Activity& b = addActivity("B", 0.0, 10.0);
        b.addRule("ruleValidateB", rvB);

        addFinishToFinishConstraint("A", "B", 0.0);
    }
};


}}} // namespace vle examples decision

DECLARE_NAMED_DYNAMICS_DBG(Facts, vle::examples::decision::Facts)
DECLARE_NAMED_DYNAMICS_DBG(Ack, vle::examples::decision::Ack)
DECLARE_NAMED_DYNAMICS_DBG(Agent1, vle::examples::decision::Agent1)
DECLARE_NAMED_DYNAMICS_DBG(Agent2, vle::examples::decision::Agent2)
DECLARE_NAMED_DYNAMICS_DBG(Agent3, vle::examples::decision::Agent3)
DECLARE_NAMED_DYNAMICS_DBG(Agent4, vle::examples::decision::Agent4)

