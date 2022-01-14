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

#pragma once

#include <map>
#include <memory>
#include <random>
#include <thread>
#include <vector>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/time.hpp>
#include <etherkitten/reader/BusQueues.hpp>
#include <etherkitten/reader/Reader.hpp>

namespace etherkitten::reader
{
	/**
	 * \brief A hash generator for datatypes::Register
	 */
	struct RegisterHash
	{
		/**
		 * \brief Generate a hash for the given Register
		 * \param[in] reg the Register to generate a hash for
		 * \return the generated hash
		 */
		size_t operator()(const datatypes::Register& reg) const
		{
			return std::hash<size_t>{}(reg.getSlaveID())
			    ^ std::hash<size_t>{}(static_cast<size_t>(reg.getRegister()));
		}
	};

	/**
	 * \brief A comparator for datatypes::Register
	 */
	struct RegisterEqual
	{
		/**
		 * \brief Compare two registers
		 * \param[in] lhs the first of the registers to compare
		 * \param[in] rhs the second of the registers to compare
		 * \return true if the registers were equal
		 */
		bool operator()(const datatypes::Register& lhs, const datatypes::Register& rhs) const
		{
			return (lhs.getSlaveID() == rhs.getSlaveID())
			    ^ (lhs.getRegister() == rhs.getRegister());
		}
	};

	/**
	 * \brief A mocked Reader to be able to run EtherKitten in "live" mode without an actual bus.
	 */
	class MockReader : public Reader
	{
	public:
		/**
		 * \brief Create a mocked Reader.
		 * \param[in]  queues the BusQueues to communicate with
		 * \param[in]  toRead the registers to read initially
		 */
		MockReader(BusQueues& queues, std::unordered_map<datatypes::RegisterEnum, bool>& toRead);

		~MockReader();

		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(const datatypes::PDO& pdo);

		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::Register& reg);

		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::PDO& pdo, datatypes::TimeSeries time);

		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::Register& reg, datatypes::TimeSeries time);

		std::shared_ptr<DataView<std::unique_ptr<IOMap>, nodeSize, IOMap*>> getIOMapView(
		    datatypes::TimeStamp startTime);

		std::shared_ptr<datatypes::AbstractDataView> getRegisterRawDataView(
		    const uint16_t slaveId, const uint16_t regId, datatypes::TimeSeries time) override;

		datatypes::PDOInfo getAbsolutePDOInfo(const datatypes::PDO& pdo);

		double getPDOFrequency();

		double getRegisterFrequency();

		void changeRegisterSettings(
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead);

		void toggleBusSafeOp();

		datatypes::BusMode getBusMode();

		void setMaximumMemory(size_t size);

		datatypes::TimeStamp getStartTime() const override;

		void messageHalt() override;

	private:
		BusQueues& queues;
		datatypes::BusMode busMode;
		std::unordered_map<datatypes::RegisterEnum, bool> regSettings;

		const datatypes::TimeStamp startTime;

		std::random_device rd;
		std::mt19937 mtEngine{ rd() };
		std::uniform_real_distribution<double> freqDist{ 950.0, 1050.0 };
		std::uniform_real_distribution<double> pdoDist{ 0.0, 255.0 };
		std::uniform_real_distribution<double> regDist{ 0.0, 1.0 };

		std::unordered_map<datatypes::PDO, SearchList<double, nodeSize>, datatypes::PDOHash,
		    datatypes::PDOEqual>
		    pdoSearchLists;
		std::unordered_map<datatypes::Register, SearchList<uint8_t, nodeSize>, RegisterHash,
		    RegisterEqual>
		    regSearchLists;

		bool destructing = false;

		std::thread timer;
		void generateData();
	};

} // namespace etherkitten::reader
