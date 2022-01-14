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

#include <catch2/catch.hpp>

#include <etherkitten/datatypes/errors.hpp>
#include <sstream>

using namespace etherkitten::datatypes;

SCENARIO("An ErrorMessage can be formatted as a string", "[ErrorMessage]")
{
	GIVEN("ErrorMessages of different severities")
	{
		const ErrorMessage lowError{ "I would do anything for love", { 14, 23 },
			ErrorSeverity::LOW };
		const ErrorMessage mediumError{ "but I won't do that", { 7058, 16372 },
			ErrorSeverity::MEDIUM };
		const ErrorMessage fatalError{ "Shot through the heart", ErrorSeverity::FATAL };
		const ErrorMessage unknownError{ "What is love?", 585, static_cast<ErrorSeverity>(485) };

		WHEN("I put them through a string stream")
		{
			std::ostringstream lowOs;
			std::ostringstream mediumOs;
			std::ostringstream fatalOs;
			std::ostringstream unknownOs;

			lowOs << lowError;
			mediumOs << mediumError;
			fatalOs << fatalError;
			unknownOs << unknownError;

			THEN("I get the correct strings")
			{
				REQUIRE(lowOs.str()
				    == "Error message: I would do anything for love; Slaves: 14, 23; Severity: "
				       "Low");
				REQUIRE(mediumOs.str()
				    == "Error message: but I won't do that; Slaves: 7058, 16372; Severity: Medium");
				REQUIRE(fatalOs.str() == "Error message: Shot through the heart; Severity: Fatal");
				REQUIRE(unknownOs.str()
				    == "Error message: What is love?; Slave: 585; Severity: Unknown");
			}
		}
	}
}
