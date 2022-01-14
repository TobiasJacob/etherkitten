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
 * \brief Defines the interface for Readers, classes that provide data via Views and process
 * requests.
 */

#include <map>
#include <memory>
#include <vector>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "DataView.hpp"
#include "IOMap.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief A Reader reads values that belong to DataObjects from the bus.
	 *
	 * These objects are defined by a SlaveInformant. The read values are saved internally.
	 * Users of the library may access the values with AbstractNewestValueViews and
	 * AbstractDataViews. If an implementing class is destroyed, all AbstractNewestValueViews
	 * and AbstractDataViews that were created by it are invalidated.
	 */
	class Reader // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		/*!
		 * \brief The node size used for the reader's views.
		 *
		 * This variable is exposed because the ioMapViews require it in their signature.
		 */
		static constexpr size_t nodeSize = 1000;

		Reader() = default;

		virtual ~Reader() {}

		/*!
		 * \brief Return a view for the newest value of the given PDO.
		 * \param pdo the PDO to return a view for
		 * \return a view for the newest value of the given PDO
		 */
		virtual std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::PDO& pdo)
		    = 0;

		/*!
		 * \brief Return a view for the newest value of the given Register.
		 * \param reg the Register to return a view for
		 * \return a view for the newest value of the given Register
		 */
		virtual std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::Register& reg)
		    = 0;

		/*!
		 * \brief Return a view for all recorded values of the given PDO.
		 * \param pdo the PDO to return a view for
		 * \param time the time series for the DataView
		 * \return a view for all values of the given PDO
		 */
		virtual std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::PDO& pdo, datatypes::TimeSeries time)
		    = 0;

		/*!
		 * \brief Return a view for all recorded values of the given Register.
		 * \param reg the Register to return a view for
		 * \param time the time series for the DataView
		 * \return a view for all values of the given Register
		 */
		virtual std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::Register& reg, datatypes::TimeSeries time)
		    = 0;

		/*!
		 * \brief Return a view for the unprocessed IOMap data
		 * \param startTime the timestamp of the first data point
		 * \return a view for all IOMaps after startTime
		 */
		virtual std::shared_ptr<DataView<std::unique_ptr<IOMap>, nodeSize, IOMap*>> getIOMapView(
		    datatypes::TimeStamp startTime)
		    = 0;

		/*!
		 * \brief Return a view for unprocessed Register data. There can be more than one Register
		 * value encoded in every datapoint
		 * \param slaveId id of the slave, starting at 1
		 * \param regId register address
		 * \param time the time series for the DataView
		 * \return a view with all unporcessed register data of registers with given address
		 * \exception std::out_of_range iff the combination of slaveId and regId is not valid
		 */
		virtual std::shared_ptr<datatypes::AbstractDataView> getRegisterRawDataView(
		    uint16_t slaveId, uint16_t regId, datatypes::TimeSeries time)
		    = 0;

		/*!
		 * \brief Return the offset of the values for the given PDO in the IOMap.
		 * \param pdo the PDO to return the offset in the IOMap for
		 * \return the offset of the values for the given PDO in the IOMap
		 * \exception std::out_of_range iff the PDO object is invalid
		 */
		virtual datatypes::PDOInfo getAbsolutePDOInfo(const datatypes::PDO& pdo) = 0;

		/*!
		 * \brief Return the frequency that the process image is read with.
		 * \return the frequency (in Hz)
		 */
		virtual double getPDOFrequency() = 0;

		/*!
		 * \brief Return the frequency that the registers are read with.
		 * \return the frequency (in Hz)
		 */
		virtual double getRegisterFrequency() = 0;

		/*!
		 * \brief Change which registers are read.
		 * \param toRead the registers that shall be read. The key is the register address
		 * and the value is whether to read the register with that address.
		 */
		virtual void changeRegisterSettings(
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
		    = 0;

		/*!
		 * \brief either toggle all slaves on the attached bus into OP mode or into SafeOP mode,
		 * depending on which they are in right now (see getBusStatus()). All slaves will have
		 * the same mode.
		 */
		virtual void toggleBusSafeOp() = 0;

		/*!
		 * \brief Return the current state of the bus.
		 * \return the current state of the bus.
		 */
		virtual datatypes::BusMode getBusMode() = 0;

		/*!
		 * \brief Set the maximum amount of memory this class may use for its data.
		 * The oldest values are deleted first if this threshold is reached.
		 * \param size the usable memory in byte.
		 */
		virtual void setMaximumMemory(size_t size) = 0;

		/*!
		 * \brief Get a TimeStamp that is earlier than all DataPoints offered by this Reader.
		 *
		 * This TimeStamp can be used as the base time to calculate time offsets
		 * of DataPoints.
		 * \return a TimeStamp that is earlier than all DataPoints
		 */
		virtual datatypes::TimeStamp getStartTime() const = 0;

		/*!
		 * \brief request the Reader to stop
		 */
		virtual void messageHalt() = 0;
	};

} // namespace etherkitten::reader
