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
 * \brief Defines the ErrorStatistician, a class that uses the data gathered by a Reader
 * to make ErrorStatistics available via DataViews and NewestValueViews.
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/datatypes/errorstatistic.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "ErrorRingBuffer.hpp"
#include "NewestValueView.hpp"
#include "Reader.hpp"
#include "SearchList.hpp"
#include "SlaveInformant.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The ErrorStatistician class uses data gathered by a Reader to make
	 * AbstractNewestValueViews and AbstractDataViews over ErrorStatistics available.
	 *
	 * Like the Reader, it supports setting a maximum memory usage in bytes and will
	 * automatically free memory on a best-effort basis before hitting that limit.
	 */
	class ErrorStatistician
	{
	public:
		/*!
		 * \brief Construct a new ErrorStatistician that creates ErrorStatistic values
		 * for the slaves from the SlaveInformant using data from the given Reader.
		 *
		 * The Reader must be constructed using the same SlaveInformant as the ErrorStatistician.
		 * \param slaveInformant the SlaveInformant to get information on the slaves from
		 * \param reader the Reader to gather the data to base the ErrorStatistics on from
		 */
		ErrorStatistician(SlaveInformant& slaveInformant, Reader& reader);

		// We don't require these constructors and they would
		// be a hassle to implement, so they'll stay deleted for now.
		ErrorStatistician(const ErrorStatistician&) = delete;
		ErrorStatistician(ErrorStatistician&&) = delete;
		ErrorStatistician& operator=(const ErrorStatistician&) = delete;
		ErrorStatistician& operator=(ErrorStatistician&&) = delete;

		~ErrorStatistician();

		/*!
		 * \brief Get the ErrorStatistic of the given type for the given slave.
		 *
		 * If type is of a non-slave-associated ErrorStatisticType, slaveId must be
		 * `std::numeric_limits<unsigned int>::max()`.
		 * \param type the type of the ErrorStatistic to get
		 * \param slaveId the ID of the slave the ErrorStatistic belongs to
		 * \return the ErrorStatistic of the given type belonging to the given slave or
		 * to no slave
		 * \exception std::out_of_range iff type is not a valid ErrorStatisticType
		 * or if type is slave-associated and slaveID does not identify a slave or
		 * if type is non-slave-associated and slaveID is not `std::numeric_limits<unsigned
		 * int>::max()`
		 */
		datatypes::ErrorStatistic& getErrorStatistic(
		    datatypes::ErrorStatisticType& type, unsigned int slaveId);

		/*!
		 * \brief Return an AbstractNewestValueView over the values of the given ErrorStatistic.
		 * \param errorStatistic the ErrorStatistic to get the view over
		 * \return an AbstractNewestValueView over the values of errorStatistic
		 * \exception std::out_of_range iff errorStatistic is not a valid ErrorStatistic
		 * provided by this ErrorStatistician
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::ErrorStatistic& errorStatistic);

		/*!
		 * \brief Return an AbstractDataValueView over the values of the given ErrorStatistic.
		 * \param errorStatistic the ErrorStatistic to get the view over
		 * \return an AbstractDataValueView over the values of errorStatistic
		 * \exception std::out_of_range iff errorStatistic is not a valid ErrorStatistic
		 * provided by this ErrorStatistician
		 */
		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::ErrorStatistic& errorStatistic, datatypes::TimeSeries time);

		/*!
		 * \brief Set the maximum memory this ErrorStatistican is allowed to use to store
		 * the values of its ErrorStatistics.
		 * \param size the maximum memory in bytes
		 */
		void setMaximumMemory(size_t size);

	private:
		const unsigned int historySize = 100;
		const datatypes::TimeStep resolution = 30ms;

		std::unordered_map<datatypes::ErrorStatistic, SearchList<double, Reader::nodeSize>,
		    datatypes::ErrorStatisticHash, datatypes::ErrorStatisticEqual>
		    errorStatisticLists;

		std::unordered_map<datatypes::ErrorStatisticType, std::vector<datatypes::ErrorStatistic>,
		    datatypes::ErrorStatisticTypeHash, datatypes::ErrorStatisticTypeEqual>
		    errorStats;

		Reader& reader;

		size_t overallSearchListsLength;

		std::atomic_size_t maxMemory;

		std::unordered_map<datatypes::ErrorStatistic,
		    std::vector<std::shared_ptr<datatypes::AbstractDataView>>,
		    datatypes::ErrorStatisticHash, datatypes::ErrorStatisticEqual>
		    dataViews;

		std::unordered_map<datatypes::ErrorStatistic,
		    ErrorRingBuffer<datatypes::DataPoint<unsigned int>>, datatypes::ErrorStatisticHash,
		    datatypes::ErrorStatisticEqual>
		    recentErrorTotals;

		std::unordered_map<datatypes::ErrorStatistic, datatypes::DataPoint<double>,
		    datatypes::ErrorStatisticHash, datatypes::ErrorStatisticEqual>
		    updatingNewestErrorStatisticPoints;

		std::atomic_bool destructing = false;

		std::thread timer;

		void createErrorStatistics(unsigned int slaveCount);

		void getDataViews(unsigned int slaveCount);

		void createSlaveErrorStatistics(datatypes::ErrorStatisticType type, unsigned int count);

		void createGlobalErrorStatistic(datatypes::ErrorStatisticType type);

		void callUpdate();

		void updateStatistics();

		void updateTotalErrorStatistics();

		void updateTotalGlobalErrorStatistic(datatypes::ErrorStatisticInfo& error);

		void updateFreqErrorStatistics(datatypes::ErrorStatisticCategory category);

		void publishErrorStatistics(
		    datatypes::ErrorStatistic& errorStatistic, double value, datatypes::TimeStamp time);

		void createRingBuffers();
	};
} // namespace etherkitten::reader
