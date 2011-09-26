/*
 * @file examples/decision/SequenceActivity.cpp
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


#include <vle/extension/decision/Agent.hpp>
#include <vle/extension/decision/Activity.hpp>
#include <sstream>

namespace vd = vle::devs;
namespace vv = vle::value;
namespace vmd = vle::extension::decision;

namespace vle { namespace examples { namespace decision {

/***
 * This is an example of a plan that adds activities in sequence with a
 * FS(mintl=1) and a FF(maxtl=3) constraints between act_i and act_{i+1}
 * on the 'done' acknowledge of the act_i. The expected behavior is that
 * activities start (and finish) on odd times (2, 4, 6, ...)
 *
 ***/
const std::string& planSequenceActivity(
    "rules {\n"
    "  rule {\n"
    "    id = \"odd\";\n"
	"    predicates = \"odd\";\n"
	"  }"
    "}\n"
    "activities {\n"
    "  sequence-activity {\n"
    "    id-prefix = \"id\";\n"
    "    number = 4;\n"
    "    temporal {\n"
    "      start = 1;\n"
    "      finish = 30;\n"
    "    }\n"
    "    temporal-sequence {\n"
    "      precedence {\n"
	"        type = \"FS\";\n"
	"        mintimelag = 1;\n"
	"      }\n"
	"      precedence {\n"
	"        type = \"FF\";\n"
	"        maxtimelag = 3;\n"
	"      }\n"
    "    }\n"
	"    output =\"out\";\n"
    "  }\n"
	"}\n");


class SequenceActivity: public vmd::Agent
{
public:
  SequenceActivity(const vd::DynamicsInit& mdl, const vd::InitEventList& evts)
   : vmd::Agent(mdl, evts)
  {
	addPredicates(this) += P("odd", &SequenceActivity::odd);
    addOutputFunctions(this) += O("out", &SequenceActivity::out);
    KnowledgeBase::plan().fill(planSequenceActivity);
  }
  virtual ~SequenceActivity()
  {
  }
  bool odd() const
  {
	  return (((int) currentTime()) % 2) == 0;
  }

  void out(const std::string& name, const vmd::Activity& activity,
          vd::ExternalEventList& output)
  {
      if (activity.isInStartedState()) {
         vd::ExternalEvent* ev = new vd::ExternalEvent("ack");
         ev->putAttribute("name", new value::String(name));
         ev->putAttribute("value", new vv::String("done"));
         output.addEvent(ev);
      }
  }

};

}}} // namespace vle examples decision

DECLARE_NAMED_DYNAMICS(SequenceActivity,
		vle::examples::decision::SequenceActivity)
