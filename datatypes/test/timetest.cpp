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

#include <thread>

#include <etherkitten/datatypes/time.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::datatypes;

SCENARIO("TimeStamps are sequential", "[TimeStamp]")
{
	GIVEN("a TimeStamp")
	{
		TimeStamp t1(now());

		WHEN("I create another TimeStamp")
		{
			TimeStamp t2(now());

			THEN("the second TimeStamp has a later time than the first") { REQUIRE(t1 <= t2); }
		}

		WHEN("I create another TimeStamp after one second")
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1s);
			TimeStamp t2(now());

			THEN("the second TimeStamp is more than one second after the first")
			{
				REQUIRE(t1 <= t2 - 1s);
			}
		}

		WHEN("I copy it to another TimeStamp")
		{
			TimeStamp t2(t1); // NOLINT(performance-unnecessary-copy-initialization)

			THEN("the times from both TimeStamps are equal") { REQUIRE(t1 == t2); }
		}
	}
}
