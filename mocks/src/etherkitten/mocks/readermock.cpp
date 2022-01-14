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

#include "readermock.hpp"
#include "etherkitten/datatypes/time.hpp"
#include <etherkitten/reader/BusQueues.hpp>
#include <etherkitten/reader/NewestValueView.hpp>

namespace etherkitten::reader
{
	MockReader::MockReader(
	    BusQueues& queues, std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	    : queues(queues)
	    , busMode(datatypes::BusMode::READ_WRITE_OP)
	    , regSettings(toRead)
	    , startTime(datatypes::now())
	{
		queues.postError(
		    datatypes::ErrorMessage("This is a not a real bus, but a mock. Deal with it.",
		        datatypes::ErrorSeverity::MEDIUM));
		timer = std::thread(&MockReader::generateData, this);
	}

	MockReader::~MockReader()
	{
		destructing = true;
		timer.join();
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> MockReader::getNewest(
	    const datatypes::PDO& pdo)
	{
		return std::make_unique<reader::NewestValueView<double, nodeSize>>(
		    pdoSearchLists[pdo], 0, 0, false);
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> MockReader::getNewest(
	    const datatypes::Register& reg)
	{
		return std::make_unique<reader::NewestValueView<uint8_t, nodeSize>>(
		    regSearchLists[reg], 0, 0, false);
	}

	std::shared_ptr<datatypes::AbstractDataView> MockReader::getView(
	    const datatypes::PDO& pdo, datatypes::TimeSeries time)
	{
		return pdoSearchLists[pdo].getView(time, false);
	}

	std::shared_ptr<datatypes::AbstractDataView> MockReader::getView(
	    const datatypes::Register& reg, datatypes::TimeSeries time)
	{
		return regSearchLists[reg].getView(time, false);
	}

	std::shared_ptr<DataView<std::unique_ptr<IOMap>, MockReader::nodeSize, IOMap*>>
	MockReader::getIOMapView(datatypes::TimeStamp startTime)
	{
		(void)startTime;
		std::shared_ptr<DataView<std::unique_ptr<IOMap>, nodeSize, IOMap*>> result;
		return result;
	}

	std::shared_ptr<datatypes::AbstractDataView> MockReader::getRegisterRawDataView(
	    const uint16_t slaveId, const uint16_t regId, datatypes::TimeSeries time)
	{
		datatypes::Register reg{ slaveId, static_cast<datatypes::RegisterEnum>(regId) };
		return getView(reg, time);
	}

	datatypes::PDOInfo MockReader::getAbsolutePDOInfo(const datatypes::PDO& pdo)
	{
		(void)pdo;
		return datatypes::PDOInfo{ 0, 0 };
	}

	double MockReader::getPDOFrequency() { return freqDist(mtEngine); }

	double MockReader::getRegisterFrequency() { return freqDist(mtEngine); }

	void MockReader::changeRegisterSettings(
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		regSettings = toRead;
	}

	void MockReader::toggleBusSafeOp()
	{
		busMode = busMode == datatypes::BusMode::READ_WRITE_OP
		    ? datatypes::BusMode::READ_WRITE_SAFE_OP
		    : datatypes::BusMode::READ_WRITE_OP;
	}

	datatypes::BusMode MockReader::getBusMode() { return busMode; }

	void MockReader::setMaximumMemory(size_t size)
	{
		(void)size;
		return;
	}

	void MockReader::messageHalt() {}

	void MockReader::generateData()
	{
		// This is for the timer thread
		while (!destructing)
		{
			for (auto& [pdo, list] : pdoSearchLists)
			{
				list.append(pdoDist(mtEngine), datatypes::now());
			}
			for (auto& [reg, list] : regSearchLists)
			{
				if (regSettings[reg.getRegister()])
				{
					ListLocation<uint8_t, nodeSize> newest = list.getNewest();
					// Set all read registers to 0 initially
					if (newest.node == nullptr)
					{
						list.append(0, datatypes::now());
						newest = list.getNewest();
					}
					// Have a 0.1% chance per cycle to increment a register
					if (regDist(mtEngine) < 0.001)
					{
						list.append(newest.node->values[newest.index] + 1, datatypes::now());
					}
				}
			}
			std::this_thread::sleep_for(30ms);
		}
	}

	datatypes::TimeStamp MockReader::getStartTime() const { return startTime; }
} // namespace etherkitten::reader
