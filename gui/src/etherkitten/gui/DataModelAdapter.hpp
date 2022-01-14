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

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <memory>
#include <string>

namespace etherkitten::gui
{
	/*!
	 * \brief Provides an interface for the GUI for manipulating DataObjects.
	 */
	class DataModelAdapter
	{
	public:
		/*!
		 * \brief Default destructor - does nothing.
		 */
		virtual ~DataModelAdapter() {}

		/*!
		 * \brief Return an AbstractNewestValueView for the given DataObject.
		 * \param data the DataObject to return an AbstractNewestValueView for
		 * \return an AbstractNewestValueView for the given DataObject
		 */
		virtual std::unique_ptr<datatypes::AbstractNewestValueView> getNewestValueView(
		    const datatypes::DataObject& data)
		    = 0;

		/*!
		 * \brief Return an AbstractDataView for the given DataObject.
		 * \param data the DataObject to return an AbstractDataView for
		 * \param timeSeries The start time and step for the AbstractDataView
		 * \return an AbstractDataView for the given DataObject
		 */
		virtual std::shared_ptr<datatypes::AbstractDataView> getDataView(
		    const datatypes::DataObject& data, datatypes::TimeSeries timeSeries)
		    = 0;

		/*!
		 * \brief Convert the given value into the correct format and write it to the bus
		 * for the given DataObject. An exception is thrown on error.
		 * \param data the DataObject to write a value for
		 * \param value the value to write
		 */
		virtual void writeData(const datatypes::DataObject& data, std::string value) = 0;

		/*!
		 * \brief Read the value from the bus for the given CoEObject.
		 * It can then be accessed via the views.
		 * \param obj the DataObject to read the value for
		 */
		virtual void readCoEObject(const datatypes::CoEObject& obj) = 0;

		/*!
		 * \brief Reset all error registers for all slaves.
		 */
		virtual void resetAllErrorRegisters() = 0;

		/*!
		 * \brief Reset all error registers for the slave with the given index.
		 * \param slave the index of the slave
		 */
		virtual void resetErrorRegisters(unsigned int slave) = 0;
	};
} // namespace etherkitten::gui
