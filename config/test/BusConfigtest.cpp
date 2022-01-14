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
#include <etherkitten/config/BusConfig.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::config;
SCENARIO("BusConfig remembers register information properly")
{
	GIVEN("A new BusConfig instance")
	{
		BusConfig busConfig{};
		WHEN("I set a register as visible")
		{
			busConfig.setRegisterVisible(7, true);
			THEN("It is reported as visible") { REQUIRE(busConfig.isRegisterVisible(7)); }
		}
		WHEN("I set a register as invisible")
		{
			busConfig.setRegisterVisible(7, false);
			THEN("It is reported as invisible") { REQUIRE_FALSE(busConfig.isRegisterVisible(7)); }
		}
		WHEN("I request the visibility for a register")
		{
			bool isVisible = busConfig.isRegisterVisible(7);
			THEN("It is reported as invisible") { REQUIRE_FALSE(isVisible); }
		}
	}
	GIVEN("A BusConfig instance with a visible register")
	{
		BusConfig busConfig{};
		busConfig.setRegisterVisible(7, true);
		WHEN("I set the register as invisible")
		{
			busConfig.setRegisterVisible(7, false);
			THEN("It is reported as invisible") { REQUIRE_FALSE(busConfig.isRegisterVisible(7)); }
		}
		WHEN("I set a register as visible (again)")
		{
			busConfig.setRegisterVisible(7, true);
			THEN("It is reported as visible") { REQUIRE(busConfig.isRegisterVisible(7)); }
		}
	}
	GIVEN("A BusConfig instance with an invisible register")
	{
		BusConfig busConfig{};
		busConfig.setRegisterVisible(7, false);
		WHEN("I set a register as visible")
		{
			busConfig.setRegisterVisible(7, true);
			THEN("It is reported as visible") { REQUIRE(busConfig.isRegisterVisible(7)); }
		}
		WHEN("I set the register as invisible (again)")
		{
			busConfig.setRegisterVisible(7, false);
			THEN("It is reported as invisible") { REQUIRE_FALSE(busConfig.isRegisterVisible(7)); }
		}
	}
}
