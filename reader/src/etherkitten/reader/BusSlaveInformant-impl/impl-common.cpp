/*
 * Copyright 2021 Niklas Arlt, Matthias Becht, Florian Bossert, Marwin Madsen, and Philip Scherer
 *
 * This file is part of EtherKITten.
 *
 * EtherKITten is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * EtherKITten is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EtherKITten.
 * If not, see <https://www.gnu.org/licenses/>.
 */


#include "impl-common.hpp"

namespace etherkitten::reader::bSInformant
{
	bool containsOnlyLowSeverityErrors(const std::vector<ErrorMessage>& errors)
	{
		return std::find_if(errors.begin(), errors.end(), [](const ErrorMessage& e) {
			return e.getSeverity() != ErrorSeverity::LOW;
		}) == errors.end();
	}

	bool containsFatalSeverityError(const std::vector<ErrorMessage>& errors)
	{
		return std::find_if(errors.begin(), errors.end(), [](const ErrorMessage& e) {
			return e.getSeverity() == ErrorSeverity::FATAL;
		}) != errors.end();
	}

	void addSOEMErrorsToVector(std::vector<ErrorMessage>& errors, ErrorSeverity severity)
	{
		while (EcatError != 0)
		{
			errors.emplace_back("SOEM error: " + std::string(ec_elist2string()), severity);
		}
	}

	void addSOEMErrorsToVector(
	    std::vector<ErrorMessage>& errors, unsigned int slave, ErrorSeverity severity)
	{
		while (EcatError != 0)
		{
			errors.emplace_back("SOEM error: " + std::string(ec_elist2string()), slave, severity);
		}
	}
} // namespace etherkitten::reader::bSInformant
