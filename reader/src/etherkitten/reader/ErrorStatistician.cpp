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

#include "ErrorStatistician.hpp"
#include "etherkitten/datatypes/dataviews.hpp"
#include "etherkitten/datatypes/errorstatistic.hpp"
#include "etherkitten/datatypes/time.hpp"
#include <chrono>

namespace etherkitten::reader
{
	ErrorStatistician::ErrorStatistician(SlaveInformant& slaveInformant, Reader& reader)
	    : reader(reader)
	    , overallSearchListsLength(0)
	    , maxMemory(std::numeric_limits<unsigned int>::max())
	{
		createErrorStatistics(slaveInformant.getSlaveCount());
		getDataViews(slaveInformant.getSlaveCount());
		createRingBuffers();
		// Be up-to-date, even at the time of construction
		updateStatistics();
		timer = std::thread(&ErrorStatistician::callUpdate, this);
	}

	ErrorStatistician::~ErrorStatistician()
	{
		destructing = true;
		timer.join();
	}

	datatypes::ErrorStatistic& ErrorStatistician::getErrorStatistic(
	    datatypes::ErrorStatisticType& type, unsigned int slaveId)
	{
		if (slaveId == std::numeric_limits<unsigned int>::max())
		{
			return errorStats.at(type).at(0);
		}
		return errorStats.at(type).at(slaveId - 1);
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> ErrorStatistician::getNewest(
	    const datatypes::ErrorStatistic& errorStatistic)
	{
		return std::make_unique<NewestValueView<double, Reader::nodeSize>>(
		    NewestValueView<double, Reader::nodeSize>(
		        errorStatisticLists[errorStatistic], 0, 0, false));
	}

	std::shared_ptr<datatypes::AbstractDataView> ErrorStatistician::getView(
	    const datatypes::ErrorStatistic& errorStatistic, datatypes::TimeSeries time)
	{
		return errorStatisticLists.at(errorStatistic).getView(time, false);
	}

	void ErrorStatistician::setMaximumMemory(size_t size)
	{
		maxMemory.store(size, std::memory_order_release);
	}

	/*!
	 * \brief Create all the ErrorStatistics that will be offered by this ErrorStatistician
	 * as well as their corresponding SearchLists and place them in their respective maps.
	 *
	 * The created ErrorStatistics and SearchLists will be assigned slaveIds in the range
	 * [1, slaveCount].
	 * \param slaveCount how many slaves to create ErrorStatistics and SearchLists for
	 */
	void ErrorStatistician::createErrorStatistics(unsigned int slaveCount)
	{
		using Cat = datatypes::ErrorStatisticCategory;
		for (const datatypes::ErrorStatisticInfo& error : datatypes::errorStatisticInfos)
		{
			createSlaveErrorStatistics(error.getType(Cat::TOTAL_SLAVE), slaveCount);
			createGlobalErrorStatistic(error.getType(Cat::TOTAL_GLOBAL));
			createSlaveErrorStatistics(error.getType(Cat::FREQ_SLAVE), slaveCount);
			createGlobalErrorStatistic(error.getType(Cat::FREQ_GLOBAL));
		}
	}

	/*!
	 * \brief Create all the ErrorStatistics of the given type that will be offered
	 * by this ErrorStatistician as well as their corresponding SearchLists
	 * and place them in their respective maps.
	 *
	 * The created ErrorStatistics and SearchLists will be assigned slaveIds in the range
	 * [1, count].
	 * \param type the ErrorStatisticType to create ErrorStatistics and SearchLists for
	 * \param count how many slaves to create ErrorStatistics and SearchLists for
	 */
	void ErrorStatistician::createSlaveErrorStatistics(
	    datatypes::ErrorStatisticType type, unsigned int count)
	{
		std::vector<datatypes::ErrorStatistic> slaveErrorStatistics;
		for (unsigned int i = 1; i <= count; ++i)
		{
			slaveErrorStatistics.emplace_back(i, type);
			// Create SearchList for this ErrorStatistic
			errorStatisticLists[slaveErrorStatistics.back()];
		}
		errorStats[type] = slaveErrorStatistics;
	}

	/*!
	 * \brief Create the global ErrorStatistic of the given type that will be offered
	 * by this ErrorStatistician as well as its corresponding SearchList
	 * and place them in their respective maps.
	 * \param type the ErrorStatisticType to create an ErrorStatistic and SearchList for
	 */
	void ErrorStatistician::createGlobalErrorStatistic(datatypes::ErrorStatisticType type)
	{
		errorStats[type] = std::vector(
		    1, datatypes::ErrorStatistic(std::numeric_limits<unsigned int>::max(), type));
		// Create SearchList for this ErrorStatistic
		errorStatisticLists[errorStats[type].back()];
	}

	void ErrorStatistician::getDataViews(unsigned int slaveCount)
	{
		using Cat = datatypes::ErrorStatisticCategory;
		for (const datatypes::ErrorStatisticInfo& error : datatypes::errorStatisticInfos)
		{
			for (unsigned int slaveId = 1; slaveId <= slaveCount; ++slaveId)
			{
				std::vector<std::shared_ptr<datatypes::AbstractDataView>> regViewsForType;
				for (const datatypes::RegisterEnum regType : error.getRegisters())
				{
					datatypes::Register reg{ slaveId, regType };
					regViewsForType.emplace_back(reader.getView(reg, { {}, { resolution } }));
				}
				dataViews[errorStats.at(error.getType(Cat::TOTAL_SLAVE)).at(slaveId - 1)]
				    = std::move(regViewsForType);
			}
		}
	}

	/*!
	 * \brief Cyclically call `updateStatistics()` and sleep.
	 */
	void ErrorStatistician::callUpdate()
	{
		// This is for the timer thread
		while (!destructing)
		{
			updateStatistics();
			std::this_thread::sleep_for(resolution);
		}
	}

	void ErrorStatistician::updateStatistics()
	{
		using Cat = datatypes::ErrorStatisticCategory;
		updateTotalErrorStatistics();
		updateFreqErrorStatistics(Cat::FREQ_SLAVE);
		updateFreqErrorStatistics(Cat::FREQ_GLOBAL);
		size_t llNodeSize = sizeof(LLNode<double>);
		size_t cachedMaxMemory = maxMemory.load(std::memory_order_acquire);
		if (cachedMaxMemory > 0 && 0.8 * cachedMaxMemory < overallSearchListsLength * llNodeSize)
		{
			unsigned int toRemove = 0.6 * cachedMaxMemory / errorStatisticLists.size() / llNodeSize;
			for (auto& [errorStatistic, searchList] : errorStatisticLists)
			{
				overallSearchListsLength -= searchList.removeOldest(toRemove);
			}
		}
	}

	void ErrorStatistician::updateTotalErrorStatistics()
	{
		using Cat = datatypes::ErrorStatisticCategory;
		for (datatypes::ErrorStatisticInfo error : datatypes::errorStatisticInfos)
		{
			bool collectValues = true;
			do
			{
				for (datatypes::ErrorStatistic& errorStat :
				    errorStats[error.getType(Cat::TOTAL_SLAVE)])
				{
					for (std::shared_ptr<datatypes::AbstractDataView>& dataView :
					    dataViews[errorStat])
					{
						// Assure all registers have a value
						if (dataView->isEmpty())
						{
							// Advance view if possible
							if (dataView->hasNext())
							{
								++*dataView;
							}
							else
							{
								collectValues = false;
								break;
							}
						}

						// Check if a view cannot be advanced
						if (!dataView->hasNext())
						{
							collectValues = false;
							break;
						}
					}
				}

				// Only collect and publish the register values if all views have one value
				// left afterwards
				if (collectValues)
				{
					for (datatypes::ErrorStatistic& errorStat :
					    errorStats[error.getType(Cat::TOTAL_SLAVE)])
					{
						unsigned int registerErrorCounterSum = 0;
						datatypes::TimeStamp registerReadTime = datatypes::TimeStamp::min();

						for (std::shared_ptr<datatypes::AbstractDataView>& dataView :
						    dataViews[errorStat])
						{
							registerReadTime = max(registerReadTime, dataView->getTime());
							// error totals should be integers, not doubles
							registerErrorCounterSum
							    += static_cast<unsigned int>(dataView->asDouble());
							// Always advance the view *after* reading it
							++*dataView;
						}
						recentErrorTotals[errorStat].add(datatypes::DataPoint<unsigned int>{
						    registerErrorCounterSum, registerReadTime });
						publishErrorStatistics(
						    errorStat, registerErrorCounterSum, registerReadTime);
					}

					// Update the global statistic every iteration
					updateTotalGlobalErrorStatistic(error);
				}
			} while (collectValues);
		}
	}

	void ErrorStatistician::updateTotalGlobalErrorStatistic(datatypes::ErrorStatisticInfo& error)
	{
		using Cat = datatypes::ErrorStatisticCategory;
		bool errorRegsIsRead = false;
		// Global statistics have just one related ErrorStatistic since they are not
		// slave-specific. Therefore we can access the sole entry in the vector
		datatypes::ErrorStatistic globalStatistic = errorStats[error.getType(Cat::TOTAL_GLOBAL)][0];
		// Now sum up all slave error statistics of this type
		unsigned int globalErrorCounterSum = 0;
		datatypes::TimeStamp registerReadTime = datatypes::TimeStamp::min();
		for (datatypes::ErrorStatistic& slaveStat : errorStats[error.getType(Cat::TOTAL_SLAVE)])
		{
			// This should never be false
			if (!recentErrorTotals[slaveStat].isEmpty())
			{
				errorRegsIsRead = true;
				const datatypes::DataPoint<unsigned int> newestRegisterDataPoint
				    = recentErrorTotals[slaveStat].getNewestElement();
				registerReadTime = max(registerReadTime, newestRegisterDataPoint.getTime());
				globalErrorCounterSum += newestRegisterDataPoint.getValue();
			}
		}
		// Archive the calculated total. It will be needed for the calculation of the global
		// error frequency (of this error type).
		if (errorRegsIsRead)
		{
			recentErrorTotals[globalStatistic].add(
			    datatypes::DataPoint<unsigned int>{ globalErrorCounterSum, registerReadTime });

			publishErrorStatistics(globalStatistic, globalErrorCounterSum, registerReadTime);
		}
	}

	void ErrorStatistician::updateFreqErrorStatistics(
	    datatypes::ErrorStatisticCategory freqCategory)
	{
		for (const datatypes::ErrorStatisticInfo& error : datatypes::errorStatisticInfos)
		{
			// We need the corresponding "total" statistic type.
			// All ErrorStatistics of this type will be summed up as a new ErrorStatistic.
			datatypes::ErrorStatisticType totalStatType
			    = error.getType(datatypes::ErrorStatisticInfo::getTotalCategory(freqCategory));
			for (datatypes::ErrorStatistic& totalStatistic : errorStats[totalStatType])
			{
				ErrorRingBuffer<datatypes::DataPoint<unsigned int>>& ringBuffer
				    = recentErrorTotals[totalStatistic];
				if (ringBuffer.getSize() == 0)
				{
					continue;
				}
				double recentErrorSum = 0;
				double minErrors = ringBuffer.getOldestElement().getValue();
				for (unsigned int i = 0; i < ringBuffer.getSize(); ++i)
				{
					recentErrorSum += ringBuffer.at(i).getValue() - minErrors;
				}

				datatypes::TimeStamp oldestTime = ringBuffer.getOldestElement().getTime();
				datatypes::TimeStamp newestTime = ringBuffer.getNewestElement().getTime();

				// If the newestTime is not greater than the oldest time, no frequency statistic
				// should be calculated for this ErrorStatistic
				if (oldestTime >= newestTime)
				{
					continue;
				}

				// This is a special case for the slave ID of global error statistics
				datatypes::ErrorStatistic& freqStat = errorStats[error.getType(freqCategory)][0];
				if (freqCategory == datatypes::ErrorStatisticCategory::FREQ_SLAVE)
				{
					freqStat
					    = errorStats[error.getType(freqCategory)][totalStatistic.getSlaveID() - 1];
				}

				double frequency = 0; // The default if only one value was recorded
				// A proper frequency requires at least 2 values
				if (ringBuffer.getSize() > 1)
				{
					const double nanosPerSecond = 1000000000.0;
					std::chrono::nanoseconds duration = newestTime - oldestTime;
					double adjDurationSecs = duration.count() / nanosPerSecond;
					// Ignore the reference value (minErrors) for frequency calculation
					frequency = recentErrorSum / (ringBuffer.getSize() - 1) / adjDurationSecs;
				}
				publishErrorStatistics(freqStat, frequency, newestTime);
			}
		}
	}

	/*!
	 * \brief Create the ring buffers used to calculate the frequency ErrorStatistics.
	 */
	void ErrorStatistician::createRingBuffers()
	{
		using Cat = datatypes::ErrorStatisticCategory;
		for (const auto& errorStatInfo : datatypes::errorStatisticInfos)
		{
			for (auto& errorStatistic : errorStats[errorStatInfo.getType(Cat::TOTAL_SLAVE)])
			{
				recentErrorTotals[errorStatistic]
				    = ErrorRingBuffer<datatypes::DataPoint<unsigned int>>(historySize);
			}
			for (auto& errorStatistic : errorStats[errorStatInfo.getType(Cat::TOTAL_GLOBAL)])
			{
				recentErrorTotals[errorStatistic]
				    = ErrorRingBuffer<datatypes::DataPoint<unsigned int>>(historySize);
			}
		}
	}

	/*!
	 * \brief Append a new value for an ErrorStatistic to the respective SearchList.
	 * \param errorStatistic the errorStatistic the new value belongs to
	 * \param value the new value
	 * \param time the TimeStamp to store the value with
	 */
	void ErrorStatistician::publishErrorStatistics(
	    datatypes::ErrorStatistic& errorStatistic, double value, datatypes::TimeStamp time)
	{
		updatingNewestErrorStatisticPoints[errorStatistic]
		    = datatypes::DataPoint<double>{ value, time };
		errorStatisticLists[errorStatistic].append(value, time);
		++overallSearchListsLength;
	}
} // namespace etherkitten::reader
