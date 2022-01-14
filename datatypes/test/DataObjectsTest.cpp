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

#include <etherkitten/datatypes/DataObjectVisitor.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::datatypes;

bool correctPDO = false;
bool correctCoE = false;
bool correctStatistic = false;
bool correctRegister = false;

class MockVisitor : public DataObjectVisitor
{
public:
	MockVisitor(PDO& pdo, CoEObject& coe, ErrorStatistic& statistic, Register& reg)
	    : pdo(pdo)
	    , coe(coe)
	    , statistic(statistic)
	    , reg(reg)
	{
	}

	void handlePDO(const PDO& object) { correctPDO = object.getType() == pdo.getType(); }

	void handleCoE(const CoEObject& object) { correctCoE = object.getType() == coe.getType(); }

	void handleErrorStatistic(const ErrorStatistic& handedStatistic)
	{
		correctStatistic = handedStatistic.getType() == statistic.getType();
	}

	void handleRegister(const Register& handedReg)
	{
		correctRegister = handedReg.getType() == reg.getType();
	}

private:
	PDO& pdo;
	CoEObject& coe;
	ErrorStatistic& statistic;
	Register& reg;
};

SCENARIO("DataObjects have to store and provide object specific information and accept a visitor",
    "[DataObject], [CoEObject], [ErrorStatistic], [PDO], [Register], "
    "[DataObjectVisitor]")
{
	GIVEN("The DataObjects and a DataObjectVisitor")
	{
		CoEObject coe(1, "CoE Object", EtherCATDataTypeEnum::VISIBLE_STRING, 2, 3, 0);
		ErrorStatistic errorStatistic(2, ErrorStatisticType::FREQ_SLAVE_MALFORMAT_FRAME_ERROR);
		PDO pdo(3, "PDO", EtherCATDataTypeEnum::TIME_OF_DAY, 5, PDODirection::OUTPUT);
		Register reg(4, RegisterEnum::DLS_USER_R8);

		MockVisitor visitor(pdo, coe, errorStatistic, reg);
		WHEN("I request the data for a CoEObject")
		{
			unsigned int index = coe.getIndex();
			std::string name = coe.getName();
			unsigned int slaveID = coe.getSlaveID();
			unsigned int subIndex = coe.getSubIndex();
			EtherCATDataTypeEnum type = coe.getType();
			bool isReadOp = coe.isReadableInOp();
			bool isReadSafeOp = coe.isReadableInSafeOp();
			bool isWriteOp = coe.isWritableInOp();
			bool isWriteSafeOp = coe.isWritableInSafeOp();

			THEN("It is consistent with the data from construction")
			{
				REQUIRE(index == 2);
				REQUIRE(name == "CoE Object");
				REQUIRE(slaveID == 1);
				REQUIRE(subIndex == 3);
				REQUIRE(type == EtherCATDataTypeEnum::VISIBLE_STRING);
				REQUIRE(isReadOp == 0);
				REQUIRE(isReadSafeOp == 0);
				REQUIRE(isWriteOp == 0);
				REQUIRE(isWriteSafeOp == 0);
			}
		}
		WHEN("I request the data for a ErrorStatistic")
		{
			std::string name = errorStatistic.getName();
			unsigned int slaveID = errorStatistic.getSlaveID();
			ErrorStatisticType statisticType = errorStatistic.getStatisticType();
			EtherCATDataTypeEnum type = errorStatistic.getType();

			THEN("It is consistent with the data from construction")
			{
				REQUIRE(name == "Frequency of slave malformat frame error");
				REQUIRE(slaveID == 2);
				REQUIRE(type == EtherCATDataTypeEnum::REAL64);
				REQUIRE(statisticType == ErrorStatisticType::FREQ_SLAVE_MALFORMAT_FRAME_ERROR);
			}
		}
		WHEN("I request the data for a PDO")
		{
			PDODirection direction = pdo.getDirection();
			unsigned int index = pdo.getIndex();
			std::string name = pdo.getName();
			unsigned int slaveID = pdo.getSlaveID();
			EtherCATDataTypeEnum type = pdo.getType();

			THEN("It is consistent with the data from construction")
			{
				REQUIRE(direction == PDODirection::OUTPUT);
				REQUIRE(index == 5);
				REQUIRE(name == "PDO");
				REQUIRE(slaveID == 3);
				REQUIRE(type == EtherCATDataTypeEnum::TIME_OF_DAY);
			}
		}
		WHEN("I request the data for a Register")
		{
			std::string name = reg.getName();
			RegisterEnum regType = reg.getRegister();
			unsigned int slaveID = reg.getSlaveID();
			EtherCATDataTypeEnum type = reg.getType();

			THEN("It is consistent with the data from construction")
			{
				REQUIRE(name == "DLS user R8");
				REQUIRE(regType == RegisterEnum::DLS_USER_R8);
				REQUIRE(slaveID == 4);
				REQUIRE(type == EtherCATDataTypeEnum::UNSIGNED32);
			}
		}
		WHEN("I hand over a visitor")
		{
			coe.acceptVisitor(visitor);
			errorStatistic.acceptVisitor(visitor);
			pdo.acceptVisitor(visitor);
			reg.acceptVisitor(visitor);
			THEN("The visitor gets the correct type")
			{
				REQUIRE(correctCoE);
				REQUIRE(correctPDO);
				REQUIRE(correctRegister);
				REQUIRE(correctStatistic);
			}
		}
	}
}
