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

#include <etherkitten/datatypes/dataobjects.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::datatypes;

SCENARIO("Some DataObjects have to be comparable to use them in Maps",
    "[CoEObject], [ErrorStatistic], [ErrorStatisticType], [PDO]")
{
	GIVEN("The CoEObject equal and hash")
	{
		CoEObjectEqual equal{};
		CoEObjectHash hash{};
		WHEN("I create two CoEObjects with the same parameters and a different one")
		{
			CoEObject coeObject1{ 1, "CoE-Object 1", EtherCATDataTypeEnum::INTEGER64, 0, 0,
				0b110110 };
			CoEObject coeObject2{ 1, "CoE-Object 1", EtherCATDataTypeEnum::INTEGER64, 0, 0,
				0b110110 };
			CoEObject coeObject3{ 1, "CoE-Object 2", EtherCATDataTypeEnum::BOOLEAN, 0, 0,
				0b110110 };
			THEN("Then the CoEObjects with the same parameters are equal and have the same hash")
			{
				REQUIRE(equal(coeObject1, coeObject2));
				REQUIRE(hash(coeObject1) == hash(coeObject2));
			}
			THEN("The different CoEObject is not equal and dosn't have the same hash")
			{
				REQUIRE(!equal(coeObject1, coeObject3));
				REQUIRE(hash(coeObject1) != hash(coeObject3));
			}
		}
	}

	GIVEN("The ErrorStatisticType equal and hash")
	{
		ErrorStatisticTypeEqual equal{};
		ErrorStatisticTypeHash hash{};
		WHEN("I create two ErrorStatisticTypes with the same parameters and a different one")
		{
			ErrorStatisticType type1 = ErrorStatisticType::FREQ_FRAME_ERROR;
			ErrorStatisticType type2 = ErrorStatisticType::FREQ_FRAME_ERROR;
			ErrorStatisticType type3 = ErrorStatisticType::FREQ_LINK_LOST_ERROR;
			THEN("Then the ErrorStatisticTypes with the same parameters are equal and have the "
			     "same hash")
			{
				REQUIRE(equal(type1, type2));
				REQUIRE(hash(type1) == hash(type2));
			}
			THEN("The different ErrorStatisticTypes is not equal and dosn't have the same hash")
			{
				REQUIRE(!equal(type1, type3));
				REQUIRE(hash(type1) != hash(type3));
			}
		}
	}

	GIVEN("The ErrorStatistic equal and hash")
	{
		ErrorStatisticEqual equal{};
		ErrorStatisticHash hash{};
		WHEN("I create two ErrorStatistic with the same parameters and a different one")
		{
			ErrorStatistic statistic1 = ErrorStatistic(1, ErrorStatisticType::FREQ_FRAME_ERROR);
			ErrorStatistic statistic2 = ErrorStatistic(1, ErrorStatisticType::FREQ_FRAME_ERROR);
			ErrorStatistic statistic3 = ErrorStatistic(1, ErrorStatisticType::FREQ_LINK_LOST_ERROR);
			THEN(
			    "Then the ErrorStatistic with the same parameters are equal and have the same hash")
			{
				REQUIRE(equal(statistic1, statistic2));
				REQUIRE(hash(statistic1) == hash(statistic2));
			}
			THEN("The different ErrorStatistic is not equal and dosn't have the same hash")
			{
				REQUIRE(!equal(statistic1, statistic3));
				REQUIRE(hash(statistic1) != hash(statistic3));
			}
		}
	}

	GIVEN("The PDO equal and hash")
	{
		PDOEqual equal{};
		PDOHash hash{};
		WHEN("I create two ErrorStatistic with the same parameters and a different one")
		{
			PDO pdo1 = PDO(0, "PDO 1", EtherCATDataTypeEnum::BIT3, 0, PDODirection::INPUT);
			PDO pdo2 = PDO(0, "PDO 1", EtherCATDataTypeEnum::BIT3, 0, PDODirection::INPUT);
			PDO pdo3 = PDO(0, "PDO 2", EtherCATDataTypeEnum::BIT4, 1, PDODirection::OUTPUT);
			THEN(
			    "Then the ErrorStatistic with the same parameters are equal and have the same hash")
			{
				REQUIRE(equal(pdo1, pdo2));
				REQUIRE(hash(pdo1) == hash(pdo2));
			}
			THEN("The different ErrorStatistic is not equal and dosn't have the same hash")
			{
				REQUIRE(!equal(pdo1, pdo3));
				REQUIRE(hash(pdo1) != hash(pdo3));
			}
		}
	}
}
