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

#include <string>
#include <unordered_map>

#include "ethercatdatatypes.hpp"

namespace etherkitten::datatypes
{
	class DataObjectVisitor;

	/*!
	 * \brief The DataObject class uniquely identifies a data source on an EtherCAT bus.
	 *
	 * A DataObject can have different values at different points in time
	 * and may be associated with one slave on an EtherCAT bus.
	 * A DataObject always has exactly one EtherCATDataType.
	 * A DataPoint acquired from the data source identified by the DataObject will always have that
	 * type.
	 */
	class DataObject // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		/*!
		 * \brief Construct a new DataObject with the given parameters.
		 *
		 * If the DataObject is not associated with a slave, pass
		 * `std::numeric_limits<unsigned int>::max()` as the slaveID.
		 * \param slaveID the ID of the slave this DataObject belongs to
		 * \param name the name of this DataObject
		 * \param type the type of data that can be obtained from the data source identified by this
		 * DataObject.
		 */
		DataObject(unsigned int slaveID, std::string&& name, EtherCATDataTypeEnum type);

		virtual ~DataObject();

		/*!
		 * \brief Accept a DataObjectVisitor.
		 *
		 * This method will call the method on the visitor
		 * that corresponds to the subclass of this DataObject.
		 * \param visitor the visitor to accept
		 */
		virtual void acceptVisitor(DataObjectVisitor& visitor) const = 0;

		/*!
		 * \brief Get the ID of the slave this DataObject belongs to.
		 * \return the ID of this DataObject's slave
		 */
		unsigned int getSlaveID() const;

		/*!
		 * \brief Get the name of this DataObject.
		 * \return the name of this DataObject
		 */
		std::string getName() const;

		/*!
		 * \brief Get the type of this DataObject.
		 * \return the type of this DataObject
		 */
		EtherCATDataTypeEnum getType() const;

	private:
		unsigned int slaveID;
		std::string name;
		EtherCATDataTypeEnum type;
	};
} // namespace etherkitten::datatypes
