/**
 * @file vle/extension/decision/Plan.cpp
 * @author The VLE Development Team
 * See the AUTHORS or Authors.txt file
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment
 * http://www.vle-project.org
 *
 * Copyright (C) 2007-2010 INRA http://www.inra.fr
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


#include <vle/extension/decision/Plan.hpp>
#include <vle/extension/decision/KnowledgeBase.hpp>
#include <vle/utils/Parser.hpp>
#include <vle/utils/Debug.hpp>
#include <vle/utils/i18n.hpp>
#include <string>
#include <istream>

namespace vle { namespace extension { namespace decision {

typedef utils::Block UB;
typedef utils::Block::Blocks UBB;
typedef utils::Block::Strings UBS;
typedef utils::Block::Reals UBR;

Plan::Plan(KnowledgeBase& kb, const std::string& buffer)
    : mKb(kb)
{
    try {
        std::istringstream in(buffer);
        utils::Parser parser(in);
        fill(parser.root());
    } catch (const std::exception& e) {
        throw utils::ArgError(fmt(_("Decision plan error in %1%")) % e.what());
    }
}

Plan::Plan(KnowledgeBase& kb, std::istream& stream)
    : mKb(kb)
{
    try {
        utils::Parser parser(stream);
        fill(parser.root());
    } catch (const std::exception& e) {
        throw utils::ArgError(fmt(_("Decision plan error: %1%")) % e.what());
    }
}

void Plan::fill(const std::string& buffer)
{
    try {
        std::istringstream in(buffer);
        utils::Parser parser(in);
        fill(parser.root());
    } catch (const std::exception& e) {
        throw utils::ArgError(fmt(_("Decision plan error in %1%")) % e.what());
    }
}

void Plan::fill(std::istream& stream)
{
    try {
        utils::Parser parser(stream);
        fill(parser.root());
    } catch (const std::exception& e) {
        throw utils::ArgError(fmt(_("Decision plan error: %1%")) % e.what());
    }
}

void Plan::fill(const utils::Block& root)
{
    utils::Block::BlocksResult mainrules, mainactivities, mainprecedences;

    mainrules = root.blocks.equal_range("rules");
    mainactivities = root.blocks.equal_range("activities");
    mainprecedences = root.blocks.equal_range("precedences");

    utils::Block::Blocks::const_iterator it;

    for (it = mainrules.first; it != mainrules.second; ++it) {
        utils::Block::BlocksResult rules;
        rules = it->second.blocks.equal_range("rule");
        fillRules(rules);
    }

    for (it = mainactivities.first; it != mainactivities.second; ++it) {
        utils::Block::BlocksResult activities;
        activities = it->second.blocks.equal_range("activity");
        fillActivities(activities);
        utils::Block::BlocksResult seqActivities;
        seqActivities = it->second.blocks.equal_range("sequence-activity");
        fillActivitiesSequence(seqActivities);
    }

    for (it = mainprecedences.first; it != mainprecedences.second; ++it) {
        utils::Block::BlocksResult precedences;
        precedences = it->second.blocks.equal_range("precedence");
        fillPrecedences(precedences);
    }
}

void Plan::fillRules(const utils::Block::BlocksResult& rules)
{
    for (UBB::const_iterator it = rules.first; it != rules.second; ++it) {
        const utils::Block& block = it->second;

        UB::StringsResult id = block.strings.equal_range("id");
        if (id.first == id.second) {
            throw utils::ArgError(_("Decision: rule needs id"));
        }

        Rule& rule = mRules.add(id.first->second);
        UB::StringsResult preds = block.strings.equal_range("predicates");

        for (UB::Strings::const_iterator jt = preds.first;
             jt != preds.second; ++jt) {
            rule.add((mKb.predicates().get(jt->second))->second);
        }
    }
}

void Plan::fillActivities(const utils::Block::BlocksResult& acts)
{
    for (UBB::const_iterator it = acts.first; it != acts.second; ++it) {
        const utils::Block& block = it->second;

        UB::StringsResult id = block.strings.equal_range("id");
        if (id.first == id.second) {
            throw utils::ArgError(_("Decision: activity needs id"));
        }

        Activity& act = mActivities.add(id.first->second, Activity());

        UB::StringsResult rules = block.strings.equal_range("rules");
        for (UBS::const_iterator jt = rules.first; jt != rules.second; ++jt) {
            act.addRule(jt->second, mRules.get(jt->second));
        }

        UB::StringsResult rulesFail = block.strings.equal_range("rules-fail");
        for (UBS::const_iterator jt = rulesFail.first; jt != rulesFail.second; ++jt) {
            act.addRuleFailure(jt->second, mRules.get(jt->second));
        }

        UB::StringsResult ack = block.strings.equal_range("ack");
        if (ack.first != ack.second) {
            act.addAcknowledgeFunction((mKb.acknowledgeFunctions().get(
                        ack.first->second))->second);
        }

        UB::StringsResult out = block.strings.equal_range("output");
        if (out .first != out.second) {
            act.addOutputFunction((mKb.outputFunctions().get(
                        out.first->second))->second);
        }

        UB::StringsResult upd = block.strings.equal_range("update");
        if (upd.first != upd.second) {
            act.addUpdateFunction((mKb.updateFunctions().get(
                        upd.first->second))->second);
        }

        UB::BlocksResult temporal = block.blocks.equal_range("temporal");
        if (temporal.first != temporal.second) {
            fillTemporal(temporal, act);
        }
    }
}

void Plan::fillActivitiesSequence(const utils::Block::BlocksResult& acts)
{
    for (UBB::const_iterator it = acts.first; it != acts.second; ++it) {
        const utils::Block& block = it->second;

        UB::StringsResult id = block.strings.equal_range("id-prefix");
        if (id.first == id.second) {
            throw utils::ArgError(_("Decision: sequence of activities needs "
                "an id-prefix"));
        }
        std::string idPrefix = id.first->second;
        std::stringstream idFirstActivity;
        idFirstActivity << idPrefix << "_1";

        Activity& act = mActivities.add(idFirstActivity.str(), Activity());
        value::Map& params = act.parameters().addMap("__internal");

        UB::RealsResult number = block.reals.equal_range("number");
        bool hasNumber = (number.first != number.second);
        if (hasNumber) {
            params.addInt("recNumber", (int)number.first->second);
        } else {
            params.addInt("recNumber", -1);//Infinite recursion
        }

        UB::StringsResult rules = block.strings.equal_range("rules");
        for (UBS::const_iterator jt = rules.first; jt != rules.second; ++jt) {
            act.addRule(jt->second, mRules.get(jt->second));
        }

        UB::StringsResult rulesFail = block.strings.equal_range("rules-fail");
        for (UBS::const_iterator jt = rulesFail.first; jt != rulesFail.second; ++jt) {
            act.addRuleFailure(jt->second, mRules.get(jt->second));
        }

        UB::StringsResult ack = block.strings.equal_range("ack");
        if (ack.first != ack.second) {
            act.addAcknowledgeFunction((mKb.acknowledgeFunctions().get(
                        ack.first->second))->second);
        }

        UB::StringsResult out = block.strings.equal_range("output");
        if (out .first != out.second) {
            act.addOutputFunction((mKb.outputFunctions().get(
                        out.first->second))->second);
        }

        UB::StringsResult upd = block.strings.equal_range("update");
        if (upd.first != upd.second) {
            act.addUpdateFunction((mKb.updateFunctions().get(
                        upd.first->second))->second);
        }

        UB::BlocksResult temporal = block.blocks.equal_range("temporal");
        if (temporal.first != temporal.second) {
            fillTemporal(temporal, act);
        }
        UB::BlocksResult temporalSeq =
           block.blocks.equal_range("temporal-sequence");
        if (temporalSeq.first != temporalSeq.second) {
            fillTemporalSequence(temporalSeq, act);
        }
    }
}


void Plan::fillTemporal(const utils::Block::BlocksResult& temps,
                        Activity& activity)
{
    for (UBB::const_iterator it = temps.first; it != temps.second; ++it) {
        const utils::Block& block = it->second;

        UB::RealsResult start = block.reals.equal_range("start");
        UB::RealsResult mins = block.reals.equal_range("minstart");
        UB::RealsResult maxs = block.reals.equal_range("maxstart");
        UB::RealsResult finish = block.reals.equal_range("finish");
        UB::RealsResult minf = block.reals.equal_range("minfinish");
        UB::RealsResult maxf = block.reals.equal_range("maxfinish");

        if (start.first != start.second) {
            if (finish.first != finish.second) {
                activity.initStartTimeFinishTime(
                    start.first->second, finish.first->second);
            } else {
                double vmin, vmax;
                if (minf.first != minf.second) {
                    if (maxf.first != maxf.second) {
                        vmin = minf.first->second;
                        vmax = maxf.first->second;
                    } else {
                        vmin = minf.first->second;
                        vmax = devs::Time::infinity;
                    }
                } else {
                    if (maxf.first != maxf.second) {
                        vmin = 0;
                        vmax = maxf.first->second;
                    } else {
                        vmin = 0;
                        vmax = devs::Time::infinity;
                    }
                }
                activity.initStartTimeFinishRange(start.first->second, vmin,
                                                  vmax);
            }
        } else {
            double vmin, vmax;
            if (mins.first != mins.second) {
                vmin = mins.first->second;
            } else {
                vmin = devs::Time::negativeInfinity;
            }

            if (maxs.first != maxs.second) {
                vmax = maxs.first->second;
            } else {
                vmax = devs::Time::infinity;
            }

            if (finish.first != finish.second) {
                activity.initStartRangeFinishTime(vmin, vmax,
                                                  finish.first->second);
            } else {
                double vminf, vmaxf;
                if (minf.first != minf.second) {
                    if (maxf.first != maxf.second) {
                        vminf = minf.first->second;
                        vmaxf = maxf.first->second;
                    } else {
                        vminf = minf.first->second;
                        vmaxf = devs::Time::infinity;
                    }
                } else {
                    if (maxf.first != maxf.second) {
                        vminf = 0;
                        vmaxf = maxf.first->second;
                    } else {
                        vminf = 0;
                        vmaxf = devs::Time::infinity;
                    }
                }
                activity.initStartRangeFinishRange(
                    vmin, vmax, vminf, vmaxf);
            }
        }
    }
}


void Plan::fillTemporalSequence(const utils::Block::BlocksResult& temps,
                        Activity& activity)
{
	//gets the precedences parameters of the current activity
	value::Map& params = activity.parameters().getMap("__internal");
	if (!params.exist("precedences")) {
		params.addSet("precedences");
	}
	value::Set& precConstraints = params.getSet("precedences");
	//for all blocks 'temporal-sequence'
	for (UBB::const_iterator it = temps.first; it != temps.second; ++it) {
		const utils::Block& tempSeqBlock = it->second;
		UB::BlocksResult precBlocks =
				tempSeqBlock.blocks.equal_range("precedence");
		//for all blocks 'precedence'
		for (UBB::const_iterator it = precBlocks.first;
				it != precBlocks.second; ++it) {
			const utils::Block& precBlock = it->second;
			UB::RealsResult mintl = precBlock.reals.equal_range("mintimelag");
			UB::RealsResult maxtl = precBlock.reals.equal_range("maxtimelag");
			UB::StringsResult type = precBlock.strings.equal_range("type");
			bool hasMintl = (mintl.first != mintl.second);
			bool hasMaxtl = (maxtl.first != maxtl.second);
			bool hasType = (type.first != type.second);
			double valuemintl = 0.0;
			double valuemaxtl = devs::Time::infinity;
			std::string valueType = "FS";

			if (hasMintl) {
				valuemintl = mintl.first->second;
			}
			if (hasMaxtl) {
				valuemaxtl = maxtl.first->second;
			}
			if (hasType) {
				valueType = type.first->second;
			}
			if (valuemintl > valuemaxtl) {
				throw utils::ArgError(fmt(_(
						"Decision: mintimelag (%1%) > maxtimelag (%2%)"))
						%  valuemintl % valuemaxtl);
			}
			if ((valueType != "FS") && (valueType != "FF")
					&& (valueType != "SS")){
				throw utils::ArgError(fmt(_(
						"Decision: precedence constraint '%1%' unknown "))
						% valueType);
			}
			value::Map& precConst = precConstraints.addMap();
			precConst.addDouble("mintimelag",valuemintl);
			precConst.addDouble("maxtimelag",valuemaxtl);
			precConst.addString("type",valueType);
		}
	}
}

void Plan::fillPrecedences(const utils::Block::BlocksResult& preds)
{
    for (UBB::const_iterator it = preds.first; it != preds.second; ++it) {
        const utils::Block& block = it->second;

        std::string valuefirst, valuesecond;
        double valuemintl = 0.0;
        double valuemaxtl = devs::Time::infinity;

        UB::StringsResult first = block.strings.equal_range("first");
        if (first.first != first.second) {
            valuefirst = first.first->second;
        }

        UB::StringsResult second = block.strings.equal_range("second");
        if (second.first != second.second) {
            valuesecond = second.first->second;
        }

        UB::RealsResult mintl = block.reals.equal_range("mintimelag");
        if (mintl.first != mintl.second) {
            valuemintl = mintl.first->second;
        }

        UB::RealsResult maxtl = block.reals.equal_range("maxtimelag");
        if (maxtl.first != maxtl.second) {
            valuemaxtl = maxtl.first->second;
        }

        UB::StringsResult type = block.strings.equal_range("type");
        if (type.first != type.second) {
            if (type.first->second == "SS") {
                mActivities.addStartToStartConstraint(valuefirst, valuesecond,
                                                      valuemintl, valuemaxtl);
            } else if (type.first->second == "FS") {
                mActivities.addFinishToStartConstraint(valuefirst, valuesecond,
                                                       valuemintl, valuemaxtl);
            } else if (type.first->second == "FF") {
                mActivities.addFinishToFinishConstraint(valuefirst,
                                                        valuesecond,
                                                        valuemintl, valuemaxtl);
            } else {
                throw utils::ArgError(fmt(
                        _("Decision: precedence type `%1%' unknown")) %
                    type.first->second);
            }
        } else {
            throw utils::ArgError(_("Decision: precedences type unknown"));
        }
    }
}

void Plan::manageRecursion(const std::string& name,
                           const Activity& act,
                           const devs::Time& /*date*/)
{
  value::Map::const_iterator ifInt = act.parameters().find("__internal");
  if (ifInt != act.parameters().end()){
    const value::Map& internParams = ifInt->second->toMap();
    value::Map::const_iterator ifNum = internParams.find("recNumber");
    if (ifNum != internParams.end()) {
      //add recursive activity in the plan with FF constraint
      const value::Integer& recNum = ifNum->second->toInteger();
      if ((recNum.value() > 1) || (recNum.value() == -1)) {
        //builds the name of the new activity
        std::string newActName;
        {
          std::vector < std::string > splitVect;
          boost::split(splitVect, name, boost::is_any_of("_"));
          if (splitVect.size() < 2) {
            throw utils::InternalError(fmt(_(
               "Decision: activity '%1%' is not generated "
               " from a sequence")) % name);
          }
          unsigned int currIndex =
             boost::lexical_cast<int>(splitVect[splitVect.size()-1]);
          std::stringstream ss;
          for(unsigned int i=0; i< splitVect.size()-1; i++){
            ss << splitVect[i] << "_";
          }
          newActName = (boost::format("%1%%2%")
             % ss.str() % (currIndex+1)).str();
        }
        //builds the activity
        Activity& newAct = activities().add(newActName,Activity(act));
        //add precedence constraints with the source
        if (internParams.exist("precedences")) {
            const value::Set& precConstraints =
                    internParams.getSet("precedences");
            for (unsigned int i = 0; i < precConstraints.size(); i++) {
                const value::Map& precConstr = precConstraints.getMap(i);
                const std::string& type = precConstr.getString("type");
                double mintl = precConstr.getDouble("mintimelag");
                double maxtl = precConstr.getDouble("maxtimelag");
                if (type == "SS") {
                    mActivities.addStartToStartConstraint(
                        name,newActName,mintl,maxtl);
                } else if (type == "FF") {
                    mActivities.addFinishToFinishConstraint(
                        name,newActName,mintl,maxtl);
                } else if (type == "FS") {
                    mActivities.addFinishToStartConstraint(
                        name,newActName,mintl,maxtl);
                }
            }
        }
        //Decrease recursion parameter
        if(recNum.value() > 1){
          value::Map& internParamsNew = newAct.parameters().getMap("__internal");
          internParamsNew.set("recNumber",value::Integer(recNum.value()-1));
        }
      }
    }
  }
}

}}} // namespace vle extension decision
