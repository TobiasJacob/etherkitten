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
#include <etherkitten/config/BusLayout.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::config;
SCENARIO("A BusLayout saves positions correctly", "[BusLayout]")
{
	GIVEN("An empty BusLayout")
	{
		BusLayout busLayout{};
		WHEN("I set the first Slave's position")
		{
			Position position{ 10, 10 };
			busLayout.setPosition(1, position);
			THEN("I can retrieve the position again")
			{
				auto retrievedPosition = busLayout.getPosition(1);
				REQUIRE(position.x == retrievedPosition.x);
				REQUIRE(position.y == retrievedPosition.y);
			}
		}
		WHEN("I set the position of Slave #10")
		{
			Position position{ 10, 10 };
			busLayout.setPosition(10, position);
			THEN("I can retrieve the position again")
			{
				auto retrievedPosition = busLayout.getPosition(10);
				REQUIRE(position.x == retrievedPosition.x);
				REQUIRE(position.y == retrievedPosition.y);
			}
		}
		WHEN("I set the master's position")
		{
			Position position{ 10, 10 };
			busLayout.setPosition(0, position);
			THEN("I can retrieve the position again")
			{
				auto retrievedPosition = busLayout.getPosition(0);
				REQUIRE(position.x == retrievedPosition.x);
				REQUIRE(position.y == retrievedPosition.y);
			}
		}
	}
}
