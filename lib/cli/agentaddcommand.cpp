/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012-2014 Icinga Development Team (http://www.icinga.org)    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "cli/agentaddcommand.hpp"
#include "cli/agentutility.hpp"
#include "base/logger.hpp"
#include "base/application.hpp"
#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <fstream>
#include <vector>

using namespace icinga;
namespace po = boost::program_options;

REGISTER_CLICOMMAND("agent/add", AgentAddCommand);

String AgentAddCommand::GetDescription(void) const
{
	return "Add Icinga 2 agent.";
}

String AgentAddCommand::GetShortDescription(void) const
{
	return "add agent";
}

/**
 * The entry point for the "agent add" CLI command.
 *
 * @returns An exit status.
 */
int AgentAddCommand::Run(const boost::program_options::variables_map& vm, const std::vector<std::string>& ap) const
{
	if (ap.empty()) {
		Log(LogCritical, "cli", "No agent name provided.");
		return 1;
	}

	if (!AgentUtility::AddAgent(ap[0])) {
		Log(LogCritical, "cli", "Cannot add agent '" + ap[0] + "'.");
		return 1;
	}

	return 0;
}