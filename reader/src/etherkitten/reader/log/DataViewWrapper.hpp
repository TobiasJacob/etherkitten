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
 * \brief Defines RegisterDataViewWrapper and IOMapDataViewWrapper that simplify
 * usage of DataViews.
 */

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/dataviews.hpp>

#include "../DataView.hpp"
#include "../IOMap.hpp"
#include "../Reader.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Wraps DataView<T> and hides template by handling serialization directly.
	 */
	class RegisterDataViewWrapper
	{
	public:
		/*!
		 * \brief Create RegisterDataViewWrapper with given register and dataView
		 * \param reg The register object corresponding to the dataView
		 * \param dataView The dataView for the register
		 */
		RegisterDataViewWrapper(uint16_t regAddr, uint16_t slaveId,
		    std::shared_ptr<datatypes::AbstractDataView> dataView);

		/*!
		 * \brief Get whether there is new data for the register
		 *
		 * \return whether there is new data
		 */
		bool hasNext();

		/*!
		 * \brief Go to the next data
		 */
		void next();

		/*!
		 * \brief Serialize the current data point.
		 * Must only be called after next() has been called at least once.
		 * \return the buffer containing the data for the log file
		 */
		Serialized get();

		/*!
		 * \brief Get whether get() and getTime() can be called.
		 * This is true, if there has been data from the underlaying DataView.
		 * \return if the DataView is empty
		 */
		bool isEmpty();

		/*!
		 * \brief Get the timestamp of the current data
		 * \return the timestamp of the data this Wrapper currently points to
		 */
		datatypes::TimeStamp getTime();

	private:
		const uint16_t regAddr;
		const uint16_t slaveId;
		bool firstIsValid = false;
		std::shared_ptr<datatypes::AbstractDataView> dataView;
	};

	/*!
	 * \brief Wraps DataView<T> to enable easy access to first element
	 */
	class IOMapDataViewWrapper
	{
	public:
		/*!
		 * \brief Create IOMapViewWrapper with given register and dataView
		 * \param reg The register object corresponding to the dataView
		 * \param dataView The dataView for the register
		 */
		IOMapDataViewWrapper(
		    std::shared_ptr<DataView<std::unique_ptr<IOMap>, Reader::nodeSize, IOMap*>> dataView);

		/*!
		 * \brief Get whether there is new data for the register
		 * \return whether there is new data
		 */
		bool hasNext();

		/*!
		 * \brief Go to the next data
		 */
		void next();

		/*!
		 * \brief Get time of current data point
		 * \return time of current data point
		 */
		datatypes::TimeStamp getTime();

		/*!
		 * \brief Get the current iomap pointer.
		 * Must only be called after next() has been called at least once.
		 * \return current ioMap pointer
		 */
		IOMap* get();

	private:
		bool firstIsValid = false;
		std::shared_ptr<DataView<std::unique_ptr<IOMap>, Reader::nodeSize, IOMap*>> dataView;
	};

} // namespace etherkitten::reader
