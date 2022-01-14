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

#include "DataObject.hpp"

namespace etherkitten::datatypes
{
	/*!
	 * \brief The PDODirection enum encodes whether a PDO is read or written by the EtherCAT master.
	 *
	 * INPUT means that the PDO is read by the slave and written by the master.
	 * OUTPUT means that the PDO is read by the master and written by the slave.
	 */
	enum class PDODirection
	{
		INPUT,
		OUTPUT,
	};

	/*!
	 * \brief The PDO class uniquely identifies one process data object (PDO) of an EtherCAT slave.
	 */
	class PDO : public DataObject
	{
	public:
		/*!
		 * \brief Construct a new PDO with the given parameters.
		 * \param slaveID the ID of the slave this PDO belongs to
		 * \param name the name of this PDO
		 * \param type the type of the data source identified by this PDO
		 * \param index the index of this PDO among the slave's PDOs.
		 * \param direction whether this is an input or output PDO.
		 */
		PDO(unsigned int slaveID, std::string&& name, EtherCATDataTypeEnum type, unsigned int index,
		    PDODirection direction);

		void acceptVisitor(DataObjectVisitor& visitor) const override;

		/*!
		 * \brief Get the index of this PDO among the slave's PDOs.
		 * \return the index of this PDO
		 */
		unsigned int getIndex() const;

		/*!
		 * \brief Get the read direction of this PDO.
		 * \return the read direction of this PDO.
		 */
		PDODirection getDirection() const;

	private:
		unsigned int index;
		PDODirection direction;
	};

	struct PDOHash
	{
		std::size_t operator()(const datatypes::PDO& pdo) const
		{
			return std::hash<unsigned int>()(pdo.getSlaveID())
			    ^ std::hash<unsigned int>()(pdo.getIndex());
		}
	};

	struct PDOEqual
	{
		bool operator()(const datatypes::PDO& lhs, const datatypes::PDO& rhs) const
		{
			return lhs.getSlaveID() == rhs.getSlaveID() && lhs.getIndex() == rhs.getIndex();
		}
	};

	/*!
	 * \brief The PDOInfo struct holds additional information about a PDO.
	 *
	 * This information is not stored in the PDO objects because it is only needed
	 * when reading and writing PDOs, but not for using them.
	 */
	struct PDOInfo
	{
		size_t bitOffset;
		size_t bitLength;
	};
} // namespace etherkitten::datatypes
