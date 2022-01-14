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
 * \brief Defines data objects that represent data sources on an EtherCAT bus.
 */

#include <stdexcept>

#include "DataObject.hpp"
#include "register.hpp"

namespace etherkitten::datatypes
{
	/*!
	 * \brief The ErrorStatisticType enum encodes the types of metrics and statistics offered by
	 * ErrorStatistic objects.
	 *
	 * Statistics prefixed with TOTAL are sums over error counters.
	 * Statistics prefixed with FREQ are error frequencies per second.
	 * Statistics containing SLAVE are measured only based on one slave, those without based
	 * on all slaves.
	 */
	enum class ErrorStatisticType
	{
		TOTAL_SLAVE_FRAME_ERROR,
		TOTAL_SLAVE_PHYSICAL_ERROR,
		TOTAL_SLAVE_PREVIOUS_ERROR,
		TOTAL_SLAVE_LINK_LOST_ERROR,
		TOTAL_SLAVE_MALFORMAT_FRAME_ERROR,
		TOTAL_SLAVE_LOCAL_PROBLEM_ERROR,
		TOTAL_FRAME_ERROR,
		TOTAL_PHYSICAL_ERROR,
		TOTAL_PREVIOUS_ERROR,
		TOTAL_LINK_LOST_ERROR,
		TOTAL_MALFORMAT_FRAME_ERROR,
		TOTAL_LOCAL_PROBLEM_ERROR,
		FREQ_SLAVE_FRAME_ERROR,
		FREQ_SLAVE_PHYSICAL_ERROR,
		FREQ_SLAVE_PREVIOUS_ERROR,
		FREQ_SLAVE_LINK_LOST_ERROR,
		FREQ_SLAVE_MALFORMAT_FRAME_ERROR,
		FREQ_SLAVE_LOCAL_PROBLEM_ERROR,
		FREQ_FRAME_ERROR,
		FREQ_PHYSICAL_ERROR,
		FREQ_PREVIOUS_ERROR,
		FREQ_LINK_LOST_ERROR,
		FREQ_MALFORMAT_FRAME_ERROR,
		FREQ_LOCAL_PROBLEM_ERROR,
	};

	/*!
	 * \brief Holds information on the available error statistics.
	 */
	// clang-format off
const std::unordered_map<ErrorStatisticType, std::string> errorStatisticMap = {
    { ErrorStatisticType::TOTAL_SLAVE_FRAME_ERROR, "Total Slave frame error" },
    { ErrorStatisticType::TOTAL_SLAVE_PHYSICAL_ERROR, "Total slave physical error" },
    { ErrorStatisticType::TOTAL_SLAVE_PREVIOUS_ERROR, "Total slave previous error" },
    { ErrorStatisticType::TOTAL_SLAVE_LINK_LOST_ERROR, "Total slave link lost error" },
    { ErrorStatisticType::TOTAL_SLAVE_MALFORMAT_FRAME_ERROR, "Total slave malformat frame error" },
    { ErrorStatisticType::TOTAL_SLAVE_LOCAL_PROBLEM_ERROR, "Total slave local problem error" },
    { ErrorStatisticType::TOTAL_FRAME_ERROR, "Total frame error" },
    { ErrorStatisticType::TOTAL_PHYSICAL_ERROR, "Total physical error" },
    { ErrorStatisticType::TOTAL_PREVIOUS_ERROR, "Total previous error" },
    { ErrorStatisticType::TOTAL_LINK_LOST_ERROR, "Total link lost error" },
    { ErrorStatisticType::TOTAL_MALFORMAT_FRAME_ERROR, "Total malformat frame error" },
    { ErrorStatisticType::TOTAL_LOCAL_PROBLEM_ERROR, "Total local problem error" },
    { ErrorStatisticType::FREQ_SLAVE_FRAME_ERROR, "Frequency of slave frame error" },
    { ErrorStatisticType::FREQ_SLAVE_PHYSICAL_ERROR, "Frequency of slave physical error" },
    { ErrorStatisticType::FREQ_SLAVE_PREVIOUS_ERROR, "Frequency of slave previous error" },
    { ErrorStatisticType::FREQ_SLAVE_LINK_LOST_ERROR, "Frequency of slave link lost error" },
    { ErrorStatisticType::FREQ_SLAVE_MALFORMAT_FRAME_ERROR, "Frequency of slave malformat frame error" },
    { ErrorStatisticType::FREQ_SLAVE_LOCAL_PROBLEM_ERROR, "Frequency of slave local problem error" },
    { ErrorStatisticType::FREQ_FRAME_ERROR, "Frequency of frame error" },
    { ErrorStatisticType::FREQ_PHYSICAL_ERROR, "Frequency of physical error" },
    { ErrorStatisticType::FREQ_PREVIOUS_ERROR, "Frequency of previous error" },
    { ErrorStatisticType::FREQ_LINK_LOST_ERROR, "Frequency of link lost error" },
    { ErrorStatisticType::FREQ_MALFORMAT_FRAME_ERROR, "Frequency of malformat frame error" },
    { ErrorStatisticType::FREQ_LOCAL_PROBLEM_ERROR, "Frequency of local problem error" }
};
	// clang-format on

	struct ErrorStatisticTypeHash
	{
		std::size_t operator()(const datatypes::ErrorStatisticType& errorStatistic) const
		{
			return std::hash<unsigned int>()(static_cast<size_t>(errorStatistic));
		}
	};

	struct ErrorStatisticTypeEqual
	{
		bool operator()(const datatypes::ErrorStatisticType& lhs,
		    const datatypes::ErrorStatisticType& rhs) const
		{
			return lhs == rhs;
		}
	};

	/*!
	 * \brief The ErrorStatistic class uniquely identifies a statistic
	 * of the errors on an EtherCAT bus.
	 */
	class ErrorStatistic : public DataObject
	{
	public:
		/*!
		 * \brief Construct a new ErrorStatistic with the given parameters.
		 *
		 * If the ErrorStatistic is not associated with a slave, pass
		 * `std::numeric_limits<unsigned int>::max()` as the slaveID.
		 * \param slaveID the ID of the slave this statistic is associated with
		 * \param type the type of error statistic identified by this ErrorStatistic
		 */
		ErrorStatistic(unsigned int slaveID, ErrorStatisticType type);

		void acceptVisitor(DataObjectVisitor& visitor) const override;

		/*!
		 * \brief Get the type of this statistic.
		 * \return the type of this statistic
		 */
		ErrorStatisticType getStatisticType() const;

	private:
		ErrorStatisticType type;
	};

	struct ErrorStatisticHash
	{
		std::size_t operator()(const datatypes::ErrorStatistic& errorStatistic) const
		{
			return std::hash<std::string>()(errorStatistic.getName())
			    ^ std::hash<unsigned int>()(errorStatistic.getSlaveID())
			    ^ std::hash<datatypes::ErrorStatisticType>()(errorStatistic.getStatisticType())
			    ^ std::hash<datatypes::EtherCATDataTypeEnum>()(errorStatistic.getType());
		}
	};

	struct ErrorStatisticEqual
	{
		bool operator()(
		    const datatypes::ErrorStatistic& lhs, const datatypes::ErrorStatistic& rhs) const
		{
			return lhs.getName() == rhs.getName() && lhs.getSlaveID() == rhs.getSlaveID()
			    && lhs.getStatisticType() == rhs.getStatisticType()
			    && lhs.getType() == rhs.getType();
		}
	};

	enum class ErrorStatisticCategory
	{
		TOTAL_SLAVE,
		TOTAL_GLOBAL,
		FREQ_SLAVE,
		FREQ_GLOBAL
	};

	/*!
	 * \brief ErrorStatisticInfo correlates different statistics with one another
	 * and with their underlying registers.
	 *
	 * Thus, different statistics such as `TOTAL_SLAVE_FRAME_ERROR` and `TOTAL_FRAME_ERROR`
	 * can be handled by the same data source more easily.
	 */
	class ErrorStatisticInfo
	{
	public:
		/*!
		 * \brief Construct a new ErrorStatisticInfo that correlates the given statistic
		 * types with one another and with the given registers as their data source.
		 *
		 * \param name the human-readable name of this statistic category
		 * \param totalSlave the `TOTAL_SLAVE` variant of the statistic
		 * \param totalGlobal the `TOTAL` variant of the statistic
		 * \param freqSlave the `FREQ_SLAVE` variant of the statistic
		 * \param freqGlobal the `FREQ` variant of the statistic
		 * \param registers the registers that provide the data for the statistics
		 */
		ErrorStatisticInfo(std::string name, ErrorStatisticType totalSlave,
		    ErrorStatisticType totalGlobal, datatypes::ErrorStatisticType freqSlave,
		    ErrorStatisticType freqGlobal, std::vector<RegisterEnum>&& registers)
		    : name(name)
		    , totalSlave(totalSlave)
		    , totalGlobal(totalGlobal)
		    , freqSlave(freqSlave)
		    , freqGlobal(freqGlobal)
		    , registers(registers)
		{
		}

		/*!
		 * \brief Get the human-readable name of this statistic category.
		 * \return the human-readable name of this statistic category
		 */
		std::string getName() const { return name; }

		/*!
		 * \brief Get the enum ErrorStatisticType of this error statistic group for
		 * the given category.
		 * \param category the category of the ErrorStatisticType
		 * \return the ErrorStatisticType from this group that matches the category
		 */
		ErrorStatisticType getType(ErrorStatisticCategory category) const
		{
			switch (category)
			{
			case ErrorStatisticCategory::TOTAL_SLAVE:
				return totalSlave;
			case ErrorStatisticCategory::TOTAL_GLOBAL:
				return totalGlobal;
			case ErrorStatisticCategory::FREQ_SLAVE:
				return freqSlave;
			case ErrorStatisticCategory::FREQ_GLOBAL:
				return freqGlobal;
			}
			throw std::runtime_error("unsupported ErrorStatisticCategory");
		}

		/*!
		 * \brief Get the registers that are the data sources for the group of error statistics.
		 * \return the data source registers
		 */
		std::vector<RegisterEnum> getRegisters() const { return registers; }

		/*!
		 * \brief Get the total category associated with the given frequency category.
		 * \param freqCategory the frequency category
		 * \return the total category
		 */
		static ErrorStatisticCategory getTotalCategory(ErrorStatisticCategory freqCategory)
		{
			switch (freqCategory)
			{
			case ErrorStatisticCategory::FREQ_SLAVE:
				return ErrorStatisticCategory::TOTAL_SLAVE;
			case ErrorStatisticCategory::FREQ_GLOBAL:
				return ErrorStatisticCategory::TOTAL_GLOBAL;
			default:
				// Illegal input, acting as identity
				return freqCategory;
			}
		}

	private:
		const std::string name;
		const ErrorStatisticType totalSlave;
		const ErrorStatisticType totalGlobal;
		const ErrorStatisticType freqSlave;
		const ErrorStatisticType freqGlobal;
		const std::vector<RegisterEnum> registers;
	};

	// clang-format off
    /*!
     * \brief Holds the default error statistic groups used in EtherKITten.
     */
    const std::vector<ErrorStatisticInfo> errorStatisticInfos = {
        ErrorStatisticInfo{
	        "Frame",
            ErrorStatisticType::TOTAL_SLAVE_FRAME_ERROR, ErrorStatisticType::TOTAL_FRAME_ERROR,
            ErrorStatisticType::FREQ_SLAVE_FRAME_ERROR,  ErrorStatisticType::FREQ_FRAME_ERROR,
            { RegisterEnum::FRAME_ERROR_COUNTER_PORT_0, RegisterEnum::FRAME_ERROR_COUNTER_PORT_1,
              RegisterEnum::FRAME_ERROR_COUNTER_PORT_2, RegisterEnum::FRAME_ERROR_COUNTER_PORT_3 } },

        ErrorStatisticInfo{
	        "Physical",
            ErrorStatisticType::TOTAL_SLAVE_PHYSICAL_ERROR, ErrorStatisticType::TOTAL_PHYSICAL_ERROR,
            ErrorStatisticType::FREQ_SLAVE_PHYSICAL_ERROR,  ErrorStatisticType::FREQ_PHYSICAL_ERROR,
            { RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_0, RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_1,
              RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_2, RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_3 } },

        ErrorStatisticInfo{
	        "Previous",
            ErrorStatisticType::TOTAL_SLAVE_PREVIOUS_ERROR, ErrorStatisticType::TOTAL_PREVIOUS_ERROR,
            ErrorStatisticType::FREQ_SLAVE_PREVIOUS_ERROR,  ErrorStatisticType::FREQ_PREVIOUS_ERROR,
            { RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_0, RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_1,
              RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_2, RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_3 } },

        ErrorStatisticInfo{
	        "Link lost",
            ErrorStatisticType::TOTAL_SLAVE_LINK_LOST_ERROR, ErrorStatisticType::TOTAL_LINK_LOST_ERROR,
            ErrorStatisticType::FREQ_SLAVE_LINK_LOST_ERROR,  ErrorStatisticType::FREQ_LINK_LOST_ERROR,
            { RegisterEnum::LOST_LINK_COUNTER_PORT_0, RegisterEnum::LOST_LINK_COUNTER_PORT_1,
                RegisterEnum::LOST_LINK_COUNTER_PORT_2, RegisterEnum::LOST_LINK_COUNTER_PORT_3 } },

        ErrorStatisticInfo{
	        "Malformat frame",
            ErrorStatisticType::TOTAL_SLAVE_MALFORMAT_FRAME_ERROR, ErrorStatisticType::TOTAL_MALFORMAT_FRAME_ERROR,
            ErrorStatisticType::FREQ_SLAVE_MALFORMAT_FRAME_ERROR,  ErrorStatisticType::FREQ_MALFORMAT_FRAME_ERROR,
            { RegisterEnum::MALFORMAT_FRAME_COUNTER } },

        ErrorStatisticInfo{
	        "Local problem",
            ErrorStatisticType::TOTAL_SLAVE_LOCAL_PROBLEM_ERROR, ErrorStatisticType::TOTAL_LOCAL_PROBLEM_ERROR,
            ErrorStatisticType::FREQ_SLAVE_LOCAL_PROBLEM_ERROR,  ErrorStatisticType::FREQ_LOCAL_PROBLEM_ERROR,
            { RegisterEnum::LOCAL_PROBLEM_COUNTER } }
    };
	// clang-format on
} // namespace etherkitten::datatypes
