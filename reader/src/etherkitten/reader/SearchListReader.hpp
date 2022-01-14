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
/*!
 * \file
 * \brief Defines the SearchListReader, a Reader that holds its data in SearchLists.
 */

#include <map>
#include <memory>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "BusSlaveInformant.hpp"
#include "IOMap.hpp"
#include "Reader.hpp"
#include "RingBuffer.hpp"
#include "SearchList.hpp"
#include "endianness.hpp"
#include "viewtemplates.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief A SearchListReader reads values that belong to DataObjects from a data source
	 * into SearchLists.
	 *
	 * These objects are defined by a SlaveInformant.
	 * Users of the library may access the values with AbstractNewestValueViews and
	 * AbstractDataViews. If an implementing class is destroyed, all AbstractNewestValueViews
	 * and AbstractDataViews that were created by it are invalidated.
	 */
	class SearchListReader : public Reader // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		SearchListReader(std::vector<uint16_t>&& slaveConfiguredAddresses, size_t ioMapUsedSize,
		    datatypes::TimeStamp&& startTime);

		virtual ~SearchListReader();

		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::PDO& pdo) override;

		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::Register& reg) override;

		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::PDO& pdo, datatypes::TimeSeries time) override;

		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::Register& reg, datatypes::TimeSeries time) override;

		std::shared_ptr<DataView<std::unique_ptr<IOMap>, nodeSize, IOMap*>> getIOMapView(
		    datatypes::TimeStamp startTime) override;

		std::shared_ptr<datatypes::AbstractDataView> getRegisterRawDataView(
		    uint16_t slaveId, uint16_t regId, datatypes::TimeSeries time) override;

		datatypes::PDOInfo getAbsolutePDOInfo(const datatypes::PDO& pdo) override = 0;

		double getPDOFrequency() override;

		double getRegisterFrequency() override;

		void changeRegisterSettings(
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead) override = 0;

		void toggleBusSafeOp() override = 0;

		datatypes::BusMode getBusMode() override = 0;

		void setMaximumMemory(size_t size) override;

		datatypes::TimeStamp getStartTime() const override;

	protected:
		/*!
		 * \brief Insert an IOMap into the respective SearchList.
		 * \param ioMap the IOMap to insert
		 * \param time the TimeStamp to associate with the IOMap
		 */
		void insertIOMap(std::unique_ptr<IOMap> ioMap, datatypes::TimeStamp&& time);

		/*!
		 * \brief Insert a register value into its respective SearchList.
		 *
		 * The registerType must be the byte-aligned register if there are multiple
		 * registers in the same byte.
		 * \param registerType the type of the register to insert
		 * \param dataPtr a pointer to the data to insert
		 * \param slaveConfiguredAddress the address of the slave the register belongs to
		 * \param time the TimeStamp to associate with the register value
		 * \exception std::out_of_range iff the combination of slaveConfiguredAddress and
		 * registerType is not valid
		 */
		void insertRegister(datatypes::RegisterEnum registerType, uint8_t* dataPtr,
		    uint16_t slaveConfiguredAddress, datatypes::TimeStamp& time);

		/*!
		 * \brief Insert a new TimeStamp into the frequency calculation RingBuffer.
		 * \param time the TimeStamp to insert
		 */
		void insertNewRegisterTimeStamp(datatypes::TimeStamp& time);

		/*!
		 * \brief Free memory from the SearchLists if too much has been used.
		 */
		void freeMemoryIfNecessary();

		std::vector<uint16_t> slaveConfiguredAddresses; // NOLINT

	private:
		SearchList<std::unique_ptr<IOMap>, nodeSize> ioMapList;
		std::unordered_map<uint16_t,
		    std::unordered_map<datatypes::RegisterEnum, bReader::RegTypesVariant<nodeSize>>>
		    registerLists;

		const datatypes::TimeStamp startTime;

		std::atomic_size_t maximumMemory;
		static constexpr float quotaUsageBeforeDeletion = 0.9;
		size_t currentMemoryUsage;

		size_t ioMapUsedSize;

		static constexpr size_t frequencyAveragerCount = 100;
		// Note that the atomics in the RingBuffer are non-blocking on x86-64 with g++
		RingBuffer<datatypes::TimeStamp, frequencyAveragerCount> pdoTimeStamps;
		RingBuffer<datatypes::TimeStamp, frequencyAveragerCount> registerTimeStamps;

		size_t freeMemory(size_t toFree);

		static double getFrequency(
		    RingBuffer<datatypes::TimeStamp, frequencyAveragerCount>& buffer);
	};
} // namespace etherkitten::reader
