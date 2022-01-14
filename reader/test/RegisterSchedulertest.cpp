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

#include <etherkitten/reader/RegisterScheduler.hpp>

#include <unordered_map>

#include <etherkitten/datatypes/dataobjects.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

void checkIfFrameEqualsVector(uint8_t* pointerIntoFrame, const std::vector<uint8_t>& vec)
{
	for (uint8_t expectedByte : vec)
	{
		REQUIRE(expectedByte == *(pointerIntoFrame++));
	}
}

bool frameEqualsFrame(EtherCATFrame* lhs, EtherCATFrame* rhs)
{
	if (lhs->lengthAndType != rhs->lengthAndType)
		return false;

	size_t length = lhs->lengthAndType & 0x000007ff;
	for (size_t i = 0; i < length; ++i)
	{
		if (lhs->pduArea[i] != rhs->pduArea[i])
			return false;
	}
	return true;
}

bool frameMetaEqualsFrameMeta(EtherCATFrameMetaData* lhs, EtherCATFrameMetaData* rhs)
{
	if (lhs->lengthOfFrame != rhs->lengthOfFrame)
		return false;
	if (lhs->pdus.size() != rhs->pdus.size())
		return false;
	for (size_t i = 0; i < lhs->pdus.size(); ++i)
	{
		if (lhs->pdus[i].slaveConfiguredAddress != rhs->pdus[i].slaveConfiguredAddress)
			return false;
		if (lhs->pdus[i].workingCounterOffset != rhs->pdus[i].workingCounterOffset)
			return false;
		for (const auto& entry : lhs->pdus[i].registerOffsets)
		{
			if (rhs->pdus[i].registerOffsets.count(entry.first) == 0)
				return false;
			if (rhs->pdus[i].registerOffsets.at(entry.first) != entry.second)
				return false;
		}
		for (const auto& entry : rhs->pdus[i].registerOffsets)
		{
			if (lhs->pdus[i].registerOffsets.count(entry.first) == 0)
				return false;
			if (lhs->pdus[i].registerOffsets.at(entry.first) != entry.second)
				return false;
		}
	}
	return true;
}

SCENARIO(
    "The RegisterScheduler produces correct EtherCAT frames with one PDU", "[RegisterScheduler]")
{
	GIVEN("one register and one slave that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddress = { 0x3468 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap
		    = { { ekdatatypes::RegisterEnum::BUILD, true } };
		RegisterScheduler sched(slaveAddress, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame = { 0x0e, 0x10, /* frame length + type */        // NOLINT
			0x04, /* FPRD */ 0xff, /* index */ 0x68, 0x34, /* slave address */              // NOLINT
			0x02, 0x00, /* register address */ 0x02, 0x00, /* data section length */        // NOLINT
			0x00, 0x00, /* external event */   0x00, 0x00, /* data */ 0x00, 0x00 /* wkc */};// NOLINT
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 16);
				REQUIRE((*it).second->pdus.size() == 1);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 14);
				REQUIRE((*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::BUILD)
				    == 12);
			}
		}
		WHEN("I want to have multiple frames")
		{
			const unsigned int numberOfFrames = 14;
			auto it = sched.getNextFrames(numberOfFrames);

			THEN("I always get the same frame")
			{
				REQUIRE(sched.getFrameCount() == 1);
				for (unsigned int i = 0; i < numberOfFrames; i++)
				{
					INFO("At frame #" << (i + 1));
					checkIfFrameEqualsVector(
					    reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
					REQUIRE((*it).second->lengthOfFrame == 16);
					REQUIRE((*it).second->pdus.size() == 1);
					REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
					REQUIRE((*it).second->pdus[0].workingCounterOffset == 14);
					REQUIRE(
					    (*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::BUILD)
					    == 12);
					++it;
				}
			}
		}
	}
	GIVEN("multiple adjacent registers that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddress = { 0x3468 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::CONFIGURED_STATION_ADDRESS, true },
			{ ekdatatypes::RegisterEnum::CONFIGURED_STATION_ALIAS, true },
		};
		RegisterScheduler sched(slaveAddress, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x10, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x10, 0x00 /* Register address */,       // NOLINT
			0x04, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00, 0x00, 0x00, 0x00 /* Data */, 0x00, 0x00 /* wkc */                  // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 18);
				REQUIRE((*it).second->pdus.size() == 1);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 16);
				REQUIRE((*it).second->pdus[0].registerOffsets.at(
				            ekdatatypes::RegisterEnum::CONFIGURED_STATION_ADDRESS)
				    == 12);
				REQUIRE((*it).second->pdus[0].registerOffsets.at(
				            ekdatatypes::RegisterEnum::CONFIGURED_STATION_ALIAS)
				    == 14);
			}
		}
		WHEN("I want to have multiple frames")
		{
			const unsigned int numberOfFrames = 14;
			auto it = sched.getNextFrames(numberOfFrames);

			THEN("I always get the same frame")
			{
				REQUIRE(sched.getFrameCount() == 1);
				for (unsigned int i = 0; i < numberOfFrames; i++)
				{
					INFO("At frame #" << (i + 1));
					checkIfFrameEqualsVector(
					    reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
					REQUIRE((*it).second->lengthOfFrame == 18);
					REQUIRE((*it).second->pdus.size() == 1);
					REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
					REQUIRE((*it).second->pdus[0].workingCounterOffset == 16);
					REQUIRE((*it).second->pdus[0].registerOffsets.at(
					            ekdatatypes::RegisterEnum::CONFIGURED_STATION_ADDRESS)
					    == 12);
					REQUIRE((*it).second->pdus[0].registerOffsets.at(
					            ekdatatypes::RegisterEnum::CONFIGURED_STATION_ALIAS)
					    == 14);
					++it;
				}
			}
		}
	}
	GIVEN("multiple disjunct registers that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddress = { 0x3468 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::RAM_SIZE, true },
			{ ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1, true },
		};
		RegisterScheduler sched(slaveAddress, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x1a, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x68, 0x34 /* Slave address */, 0x02, 0x03 /* Register address */,       // NOLINT
			0x01, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */                                    // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 28);
				REQUIRE((*it).second->pdus.size() == 2);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
				REQUIRE(
				    (*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 12);
				REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
				REQUIRE((*it).second->pdus[1].registerOffsets.at(
				            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
				    == 25);
			}
		}
		WHEN("I want to have multiple frames")
		{
			const unsigned int numberOfFrames = 14;
			auto it = sched.getNextFrames(numberOfFrames);

			THEN("I always get the same frame")
			{
				REQUIRE(sched.getFrameCount() == 1);
				for (unsigned int i = 0; i < numberOfFrames; i++)
				{
					INFO("At frame #" << (i + 1));
					checkIfFrameEqualsVector(
					    reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
					REQUIRE((*it).second->lengthOfFrame == 28);
					REQUIRE((*it).second->pdus.size() == 2);
					REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
					REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
					REQUIRE((*it).second->pdus[0].registerOffsets.at(
					            ekdatatypes::RegisterEnum::RAM_SIZE)
					    == 12);
					REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddress[0]);
					REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
					REQUIRE((*it).second->pdus[1].registerOffsets.at(
					            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
					    == 25);
					++it;
				}
			}
		}
	}
	GIVEN("multiple slaves with one register that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddresses = { 0x3468, 0x2350 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::RAM_SIZE, true },
		};
		RegisterScheduler sched(slaveAddresses, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x1a, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x50, 0x23 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */                                    // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 28);
				REQUIRE((*it).second->pdus.size() == 2);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddresses[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
				REQUIRE(
				    (*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 12);
				REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddresses[1]);
				REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
				REQUIRE(
				    (*it).second->pdus[1].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 25);
			}
		}
		WHEN("I want to have multiple frames")
		{
			const unsigned int numberOfFrames = 14;
			auto it = sched.getNextFrames(numberOfFrames);

			THEN("I always get the same frame")
			{
				REQUIRE(sched.getFrameCount() == 1);
				for (unsigned int i = 0; i < numberOfFrames; i++)
				{
					INFO("At frame #" << (i + 1));
					checkIfFrameEqualsVector(
					    reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
					REQUIRE((*it).second->lengthOfFrame == 28);
					REQUIRE((*it).second->pdus.size() == 2);
					REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddresses[0]);
					REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
					REQUIRE((*it).second->pdus[0].registerOffsets.at(
					            ekdatatypes::RegisterEnum::RAM_SIZE)
					    == 12);
					REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddresses[1]);
					REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
					REQUIRE((*it).second->pdus[1].registerOffsets.at(
					            ekdatatypes::RegisterEnum::RAM_SIZE)
					    == 25);
					++it;
				}
			}
		}
	}
	GIVEN("multiple slaves with multiple disjunct registers that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddresses = { 0x3468, 0x2350 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::RAM_SIZE, true },
			{ ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1, true },
		};
		RegisterScheduler sched(slaveAddresses, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x34, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x68, 0x34 /* Slave address */, 0x02, 0x03 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */,                                   // NOLINT
			0x04 /* FPRD */, 0xff /* Index */,                                       // NOLINT
			0x50, 0x23 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x50, 0x23 /* Slave address */, 0x02, 0x03 /* Register address */,       // NOLINT
			0x01, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */                                    // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 54);
				REQUIRE((*it).second->pdus.size() == 4);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddresses[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
				REQUIRE(
				    (*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 12);
				REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddresses[0]);
				REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
				REQUIRE((*it).second->pdus[1].registerOffsets.at(
				            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
				    == 25);
				REQUIRE((*it).second->pdus[2].slaveConfiguredAddress == slaveAddresses[1]);
				REQUIRE((*it).second->pdus[2].workingCounterOffset == 39);
				REQUIRE(
				    (*it).second->pdus[2].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 38);
				REQUIRE((*it).second->pdus[3].slaveConfiguredAddress == slaveAddresses[1]);
				REQUIRE((*it).second->pdus[3].workingCounterOffset == 52);
				REQUIRE((*it).second->pdus[3].registerOffsets.at(
				            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
				    == 51);
			}
		}
		WHEN("I want to have multiple frames")
		{
			const unsigned int numberOfFrames = 14;
			auto it = sched.getNextFrames(numberOfFrames);

			THEN("I always get the same frame")
			{
				REQUIRE(sched.getFrameCount() == 1);
				for (unsigned int i = 0; i < numberOfFrames; i++)
				{
					INFO("At frame #" << (i + 1));
					checkIfFrameEqualsVector(
					    reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
					REQUIRE((*it).second->lengthOfFrame == 54);
					REQUIRE((*it).second->pdus.size() == 4);
					REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddresses[0]);
					REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
					REQUIRE((*it).second->pdus[0].registerOffsets.at(
					            ekdatatypes::RegisterEnum::RAM_SIZE)
					    == 12);
					REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddresses[0]);
					REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
					REQUIRE((*it).second->pdus[1].registerOffsets.at(
					            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
					    == 25);
					REQUIRE((*it).second->pdus[2].slaveConfiguredAddress == slaveAddresses[1]);
					REQUIRE((*it).second->pdus[2].workingCounterOffset == 39);
					REQUIRE((*it).second->pdus[2].registerOffsets.at(
					            ekdatatypes::RegisterEnum::RAM_SIZE)
					    == 38);
					REQUIRE((*it).second->pdus[3].slaveConfiguredAddress == slaveAddresses[1]);
					REQUIRE((*it).second->pdus[3].workingCounterOffset == 52);
					REQUIRE((*it).second->pdus[3].registerOffsets.at(
					            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
					    == 51);
					++it;
				}
			}
		}
	}
	GIVEN("the maximum total PDU length and enough slaves with disjunct registers to fill more "
	      "than one frame")
	{
		std::vector<uint16_t> slaveAddresses;
		slaveAddresses.reserve(
		    (1.5 * maxTotalPDULength / (sizeof(EtherCATPDU) * 2 /*register count*/)));
		for (size_t i = 0; i < slaveAddresses.capacity(); ++i)
		{
			slaveAddresses.push_back(0x2000 + i * 0x100);
		}
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::RAM_SIZE, true },
			{ ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1, true },
		};
		RegisterScheduler sched(slaveAddresses, rMap);
		WHEN("I want to have multiple frames")
		{
			const unsigned int numberOfFrames = 62;
			auto it = sched.getNextFrames(numberOfFrames);
			THEN("I get two distinct alternating frames")
			{
				REQUIRE(sched.getFrameCount() == 2);
				std::pair<EtherCATFrame*, EtherCATFrameMetaData*> frame1 = *it;
				++it;
				std::pair<EtherCATFrame*, EtherCATFrameMetaData*> frame2 = *it;
				REQUIRE_FALSE(frameEqualsFrame(frame1.first, frame2.first));
				REQUIRE_FALSE(frameMetaEqualsFrameMeta(frame1.second, frame2.second));
				++it;
				for (unsigned int i = 2; i < numberOfFrames; i++)
				{
					INFO("At frame #" << (i + 1));
					REQUIRE(frameEqualsFrame((*it).first, frame1.first));
					REQUIRE(frameMetaEqualsFrameMeta((*it).second, frame1.second));
					REQUIRE_FALSE(frameEqualsFrame((*it).first, frame2.first));
					REQUIRE_FALSE(frameMetaEqualsFrameMeta((*it).second, frame2.second));
					frame1 = frame2;
					frame2 = *it;
					++it;
				}
			}
		}
	}
	GIVEN("multiple adjacent registers that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddress = { 0x3468 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::CONFIGURED_STATION_ADDRESS, true },
			{ ekdatatypes::RegisterEnum::CONFIGURED_STATION_ALIAS, true },
		};
		RegisterScheduler sched(slaveAddress, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x10, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x10, 0x00 /* Register address */,       // NOLINT
			0x04, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00, 0x00, 0x00, 0x00 /* Data */, 0x00, 0x00 /* wkc */                  // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 18);
				REQUIRE((*it).second->pdus.size() == 1);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 16);
				REQUIRE((*it).second->pdus[0].registerOffsets.at(
				            ekdatatypes::RegisterEnum::CONFIGURED_STATION_ADDRESS)
				    == 12);
				REQUIRE((*it).second->pdus[0].registerOffsets.at(
				            ekdatatypes::RegisterEnum::CONFIGURED_STATION_ALIAS)
				    == 14);
			}
		}
	}
	GIVEN("multiple disjunct registers that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddress = { 0x3468 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::RAM_SIZE, true },
			{ ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1, true },
		};
		RegisterScheduler sched(slaveAddress, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x1a, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x68, 0x34 /* Slave address */, 0x02, 0x03 /* Register address */,       // NOLINT
			0x01, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */                                    // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 28);
				REQUIRE((*it).second->pdus.size() == 2);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
				REQUIRE(
				    (*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 12);
				REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
				REQUIRE((*it).second->pdus[1].registerOffsets.at(
				            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
				    == 25);
			}
		}
	}
	GIVEN("multiple slaves with one register that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddress = { 0x3468, 0x2350 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::RAM_SIZE, true },
		};
		RegisterScheduler sched(slaveAddress, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x1a, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x50, 0x23 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */                                    // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 28);
				REQUIRE((*it).second->pdus.size() == 2);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
				REQUIRE(
				    (*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 12);
				REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddress[1]);
				REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
				REQUIRE(
				    (*it).second->pdus[1].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 25);
			}
		}
	}
	GIVEN("multiple slaves with multiple disjunct registers that I construct the scheduler with")
	{
		std::vector<uint16_t> slaveAddress = { 0x3468, 0x2350 };
		std::unordered_map<ekdatatypes::RegisterEnum, bool> rMap{
			{ ekdatatypes::RegisterEnum::RAM_SIZE, true },
			{ ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1, true },
		};
		RegisterScheduler sched(slaveAddress, rMap);

		// clang-format off
		std::vector<uint8_t> expectedFrame{
			0x34, 0x10 /* frame length + type */, 0x04 /* FPRD */, 0xff /* Index */, // NOLINT
			0x68, 0x34 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x68, 0x34 /* Slave address */, 0x02, 0x03 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */,                                   // NOLINT
			0x04 /* FPRD */, 0xff /* Index */,                                       // NOLINT
			0x50, 0x23 /* Slave address */, 0x06, 0x00 /* Register address */,       // NOLINT
			0x01, 0x80 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */, 0x04 /* FPRD */, 0xff /* Index */,// NOLINT
			0x50, 0x23 /* Slave address */, 0x02, 0x03 /* Register address */,       // NOLINT
			0x01, 0x00 /* Data section length */, 0x00, 0x00 /* External event */,   // NOLINT
			0x00 /* Data */, 0x00, 0x00 /* wkc */                                    // NOLINT
		};
		// clang-format on

		WHEN("I want to have one frame")
		{
			auto it = sched.getNextFrames(1);

			THEN("I get a valid EtherCAT frame")
			{
				checkIfFrameEqualsVector(reinterpret_cast<uint8_t*>((*it).first), expectedFrame);
				REQUIRE((*it).second->lengthOfFrame == 54);
				REQUIRE((*it).second->pdus.size() == 4);
				REQUIRE((*it).second->pdus[0].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[0].workingCounterOffset == 13);
				REQUIRE(
				    (*it).second->pdus[0].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 12);
				REQUIRE((*it).second->pdus[1].slaveConfiguredAddress == slaveAddress[0]);
				REQUIRE((*it).second->pdus[1].workingCounterOffset == 26);
				REQUIRE((*it).second->pdus[1].registerOffsets.at(
				            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
				    == 25);
				REQUIRE((*it).second->pdus[2].slaveConfiguredAddress == slaveAddress[1]);
				REQUIRE((*it).second->pdus[2].workingCounterOffset == 39);
				REQUIRE(
				    (*it).second->pdus[2].registerOffsets.at(ekdatatypes::RegisterEnum::RAM_SIZE)
				    == 38);
				REQUIRE((*it).second->pdus[3].slaveConfiguredAddress == slaveAddress[1]);
				REQUIRE((*it).second->pdus[3].workingCounterOffset == 52);
				REQUIRE((*it).second->pdus[3].registerOffsets.at(
				            ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1)
				    == 51);
			}
		}
	}
}
