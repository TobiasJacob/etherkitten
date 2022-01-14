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

#include <iostream>

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/reader/DataView.hpp>
#include <etherkitten/reader/DatatypesSerializer.hpp>
#include <etherkitten/reader/IOMap.hpp>
#include <etherkitten/reader/log/DataViewWrapper.hpp>
#include <etherkitten/reader/log/coe.hpp>
#include <etherkitten/reader/log/coeentry.hpp>
#include <etherkitten/reader/log/esi.hpp>
#include <etherkitten/reader/log/neighbors.hpp>
#include <etherkitten/reader/log/pdo.hpp>
#include <etherkitten/reader/log/pdodetails.hpp>

#include "SlaveInformantMock.hpp"

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
using namespace etherkitten::datatypes;

SCENARIO("Serialized works correctly", "[Logger]")
{
	GIVEN("A serialized object and some data")
	{
		Serialized ser{ 2 };
		uint8_t a = 43;
		uint16_t b = 143;
		std::string c = "u";
		std::string d = "uw";

		WHEN("I write of correct size")
		{
			THEN("No exception is thrown")
			{
				REQUIRE_NOTHROW(ser.write(a, 0));
				REQUIRE_NOTHROW(ser.write(a, 1));
				REQUIRE_NOTHROW(ser.write(b, 0));
				REQUIRE_NOTHROW(ser.write(c, 0));
			}
		}

		WHEN("I write more than ser is long")
		{
			THEN("An exception is thrown")
			{
				REQUIRE_THROWS(ser.write(a, 2));
				REQUIRE_THROWS(ser.write<uint16_t>(4358, 1));
				REQUIRE_THROWS(ser.write(b, 1));
				REQUIRE_THROWS(ser.write(c, 1));
				REQUIRE_THROWS(ser.write(d, 0));
				REQUIRE(a == 43);
			}
		}
	}

	GIVEN("A serialized object")
	{
		Serialized ser(10);

		WHEN("I write a string")
		{
			std::string a = "Hallo";
			ser.write(std::move(a), 0);
			THEN("I can read the written string again")
			{
				REQUIRE(ser.read<std::string>(0) == "Hallo");
			}
		}

		WHEN("I write a int")
		{
			long int a = -3985928;
			ser.write(a, 0);
			THEN("I can read the written int again") { REQUIRE(ser.read<long int>(0) == a); }
		}

		WHEN("I read outside of the buffer boundaries")
		{
			ser.write<uint64_t>(0xfffffffffffffffful, 0);
			ser.write<uint64_t>(0xfffffffffffffffful, 2);

			THEN("I get an exception")
			{
				REQUIRE_THROWS(ser.read<uint64_t>(3));
				REQUIRE_THROWS(ser.read<std::string>(0));
			}
		}

		WHEN("I use getAt")
		{
			THEN("The boundaray checks are correct")
			{
				REQUIRE_NOTHROW(ser.getAt(0, 10));
				REQUIRE_THROWS(ser.getAt(1, 10));
				REQUIRE_THROWS(ser.getAt(0, 11));
				REQUIRE_THROWS(ser.getAt(5, 6));
			}
		}
	}
}

/*
 * We test what the logger test does not test in the Block classes.
 */

SCENARIO("Block classes are working properly", "[Logger]")
{
	GIVEN("A CoEBlock")
	{
		CoEBlock block{ 4354, 54, 3485, "test" };
		WHEN("I serialize that block")
		{
			Serialized ser = block.getSerializer().serialize(block);
			THEN("The result is as expected")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				auto obj = CoEBlock::serializer.parseSerialized(ser, pc);
				REQUIRE(obj.getIndex() == 4354);
				REQUIRE(obj.getSubIndex() == 54);
				REQUIRE(static_cast<uint16_t>(obj.getType()) == 3485);
				REQUIRE(obj.getName() == "test");
			}
		}

		WHEN("I try to read a CoEBlock with wrong identifier")
		{
			Serialized ser = block.getSerializer().serialize(block);
			ser.data[0] = 45;
			THEN("An exception is thrown")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				REQUIRE_THROWS(CoEBlock::serializer.parseSerialized(ser, pc));
			}
		}
	}

	GIVEN("A CoEEntryBlock")
	{
		CoEBlock coeBlock{ 4354, 54, 3485, "test" };
		CoEEntryBlock block{ 1234, 194, "teeest", std::vector<CoEBlock>{ coeBlock } };
		WHEN("I serialize that block")
		{
			Serialized ser = block.getSerializer().serialize(block);
			THEN("The result is as expected")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				auto obj = CoEEntryBlock::serializer.parseSerialized(ser, pc);
				REQUIRE(obj.getIndex() == 1234);
				REQUIRE(static_cast<uint8_t>(obj.getObjectCode()) == 194);
				REQUIRE(obj.getName() == "teeest");
				REQUIRE(obj.getObjects().at(0).getIndex() == 4354);
				REQUIRE(obj.getObjects().at(0).getSubIndex() == 54);
				REQUIRE(static_cast<uint16_t>(obj.getObjects().at(0).getType()) == 3485);
				REQUIRE(obj.getObjects().at(0).getName() == "test");
			}
		}

		WHEN("I try to read a CoEBlock with wrong identifier")
		{
			Serialized ser = block.getSerializer().serialize(block);
			ser.data[0] = 45;
			THEN("An exception is thrown")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				REQUIRE_THROWS(CoEEntryBlock::serializer.parseSerialized(ser, pc));
			}
		}
	}

	GIVEN("An ESIBlock")
	{
		std::vector<std::byte> vec{ std::byte(78), std::byte(195), std::byte(136) };
		ESIBlock block{ vec };
		WHEN("I serialize that block")
		{
			Serialized ser = block.getSerializer().serialize(block);
			THEN("The result is as expected")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				auto obj = ESIBlock::serializer.parseSerialized(ser, pc);
				REQUIRE(obj.size() == 3);
				REQUIRE(obj.at(0) == std::byte(78));
				REQUIRE(obj.at(1) == std::byte(195));
				REQUIRE(obj.at(2) == std::byte(136));
			}
		}

		WHEN("I try to read a CoEBlock with wrong identifier")
		{
			Serialized ser = block.getSerializer().serialize(block);
			ser.data[0] = 45;
			THEN("An exception is thrown")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				REQUIRE_THROWS(ESIBlock::serializer.parseSerialized(ser, pc));
			}
		}
	}

	GIVEN("A NeighborsBlock")
	{
		std::array<unsigned int, 4> n{ 1, 2, 3, 4 };
		NeighborsBlock block{ n };
		WHEN("I serialize that block")
		{
			Serialized ser = block.getSerializer().serialize(block);
			THEN("The result is as expected")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				auto obj = NeighborsBlock::serializer.parseSerialized(ser, pc);
				REQUIRE(obj.size() == 4);
				REQUIRE(obj.at(0) == 1);
				REQUIRE(obj.at(1) == 2);
				REQUIRE(obj.at(2) == 3);
				REQUIRE(obj.at(3) == 4);
			}
		}

		WHEN("I try to read a CoEBlock with wrong identifier")
		{
			Serialized ser = block.getSerializer().serialize(block);
			ser.data[0] = 45;
			THEN("An exception is thrown")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				REQUIRE_THROWS(NeighborsBlock::serializer.parseSerialized(ser, pc));
			}
		}
	}

	GIVEN("A PDOBlock")
	{
		PDOBlock block{ 4354, 2054, 3485, "test" };
		WHEN("I serialize that block")
		{
			Serialized ser = block.getSerializer().serialize(block);
			THEN("The result is as expected")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				auto obj = PDOBlock::serializer.parseSerialized(ser, pc);
				REQUIRE(obj.getIndex() == 4354);
				REQUIRE(static_cast<uint16_t>(obj.getType()) == 3485);
				REQUIRE(obj.getName() == "test");
			}
		}

		WHEN("I try to read a PDOBlock with wrong identifier")
		{
			Serialized ser = block.getSerializer().serialize(block);
			ser.data[0] = 45;
			THEN("An exception is thrown")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				REQUIRE_THROWS(PDOBlock::serializer.parseSerialized(ser, pc));
			}
		}
	}

	GIVEN("A PDODetailsBlock")
	{
		PDODetailsBlock block{ 7283, 5763, 93, 3765 };
		WHEN("I serialize that block")
		{
			Serialized ser = block.getSerializer().serialize(block);
			THEN("The result is as expected")
			{
				SlaveInformantMock si{ 0, 8 };
				LogBusInfo bi;
				ParsingContext pc(si, bi);
				auto obj = PDODetailsBlock::serializer.parseSerialized(ser, pc);
				REQUIRE(obj.index == 7283);
				REQUIRE(obj.offset == 5763);
				REQUIRE(obj.bitLength == 93);
				REQUIRE(obj.datatype == 3765);
			}
		}
	}
}

SCENARIO("ParsingContext handles errors correctly", "[Logger]")
{
	GIVEN("A SlaveInformant and ParsingContext")
	{
		SlaveInformantMock si{ 1, 8 };
		si.feedSlaveInfo(
		    1, etherkitten::datatypes::SlaveInfo{ 1, "hallo", {}, {}, {}, {}, { 1, 2, 3, 4 } });
		LogBusInfo bi;
		ParsingContext pc{ si, bi };
		pc.slaveId = 1;

		WHEN("ParsingContext is requested a CoEObject it cannot find")
		{
			THEN("It throws an exception") { REQUIRE_THROWS(pc.getCoEType(10, 4)); }
		}
	}
}

SCENARIO("DatatypesSerializes works")
{
	GIVEN("Some value")
	{
		WHEN("I serialize and parse bitset<24>")
		{
			std::any i(EtherCATDataType::UNSIGNED24{ 0xAF39B5ull });
			THEN("I get the same value again")
			{
				Serialized ser(DatatypesSerializer::getLength(EtherCATDataTypeEnum::UNSIGNED24, i));
				DatatypesSerializer::serialize(EtherCATDataTypeEnum::UNSIGNED24, i, ser);
				REQUIRE(std::any_cast<EtherCATDataType::UNSIGNED24>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::UNSIGNED24, ser))
				            .to_ullong()
				    == 0xAF39B5ull);
			}
		}

		WHEN("I serialize and parse bitset<48>")
		{
			std::any i(EtherCATDataType::TIME_OF_DAY{ 0xAF39B56794D3ull });
			THEN("I get the same value again")
			{
				Serialized ser(
				    DatatypesSerializer::getLength(EtherCATDataTypeEnum::TIME_OF_DAY, i));
				DatatypesSerializer::serialize(EtherCATDataTypeEnum::TIME_OF_DAY, i, ser);
				REQUIRE(std::any_cast<EtherCATDataType::TIME_OF_DAY>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::TIME_OF_DAY, ser))
				            .to_ullong()
				    == 0xAF39B56794D3ull);
			}
		}

		WHEN("I serialize and parse bitset<5>")
		{
			std::any i(EtherCATDataType::BIT5{ 0b10011u });
			THEN("I get the same value again")
			{
				Serialized ser(DatatypesSerializer::getLength(EtherCATDataTypeEnum::BIT5, i));
				DatatypesSerializer::serialize(EtherCATDataTypeEnum::BIT5, i, ser);
				REQUIRE(std::any_cast<EtherCATDataType::BIT5>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::BIT5, ser))
				            .to_ullong()
				    == 0b10011u);
			}
		}

		WHEN("I serialize and parse vector<uint8_t>")
		{
			std::any i(EtherCATDataType::OCTET_STRING{ 0x5F, 0x73, 0x50, 0x7D, 0x9F });
			THEN("I get the same value again")
			{
				Serialized ser(
				    DatatypesSerializer::getLength(EtherCATDataTypeEnum::OCTET_STRING, i));
				DatatypesSerializer::serialize(EtherCATDataTypeEnum::OCTET_STRING, i, ser);
				REQUIRE(std::any_cast<EtherCATDataType::OCTET_STRING>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::OCTET_STRING, ser))
				            .size()
				    == 5);
				REQUIRE(std::any_cast<EtherCATDataType::OCTET_STRING>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::OCTET_STRING, ser))
				            .at(0)
				    == 0x5F);
				REQUIRE(std::any_cast<EtherCATDataType::OCTET_STRING>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::OCTET_STRING, ser))
				            .at(1)
				    == 0x73);
				REQUIRE(std::any_cast<EtherCATDataType::OCTET_STRING>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::OCTET_STRING, ser))
				            .at(2)
				    == 0x50);
				REQUIRE(std::any_cast<EtherCATDataType::OCTET_STRING>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::OCTET_STRING, ser))
				            .at(3)
				    == 0x7D);
				REQUIRE(std::any_cast<EtherCATDataType::OCTET_STRING>(
				            DatatypesSerializer::parse(EtherCATDataTypeEnum::OCTET_STRING, ser))
				            .at(4)
				    == 0x9F);
			}
		}
	}
}
