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
	 * \brief The CoEObject class uniquely identifies one CANOpen over EtherCAT (CoE) object
	 * of an EtherCAT slave.
	 */
	class CoEObject : public DataObject
	{
	public:
		/*!
		 * \brief Construct a new CoEObject with the given parameters.
		 * \param slaveID the ID of the slave this CoEObject belongs to
		 * \param name the name of this CoEObject
		 * \param type the type of data source identified by this CoEObject
		 * \param index the index of this CoEObject in the object dictionary of the slave
		 * \param subIndex the subindex of the CoEObject in the object dictionary of the slave
		 * \param access encodes the accessibility of this CoEObject in different states.
		 */
		CoEObject(unsigned int slaveID, std::string&& name, EtherCATDataTypeEnum type,
		    unsigned int index, unsigned int subIndex, unsigned int access);

		void acceptVisitor(DataObjectVisitor& visitor) const override;

		/*!
		 * \brief Get the index of this CoEObject in the object dictionary of the slave.
		 * \return the index of this CoEObject
		 */
		unsigned int getIndex() const;

		/*!
		 * \brief Get the subindex of this CoEObject in the object dictionary of the slave.
		 * \return the subindex of this CoEObject
		 */
		unsigned int getSubIndex() const;

		/*!
		 * \brief Get whether this CoEObject is readable
		 * when the slave is in the Safe-OP state.
		 * \return whether the slave is readable in Safe-OP
		 */
		bool isReadableInSafeOp() const;

		/*!
		 * \brief Get whether this CoEObject is readable
		 * when the slave is in the OP state.
		 * \return whether the slave is readable in OP
		 */
		bool isReadableInOp() const;

		/*!
		 * \brief Get whether this CoEObject is writable
		 * when the slave is in the Safe-OP state.
		 * \return whether the slave is writable in Safe-OP
		 */
		bool isWritableInSafeOp() const;

		/*!
		 * \brief Get whether this CoEObject is writable
		 * when the slave is in the OP state.
		 * \return whether the slave is writable in OP
		 */
		bool isWritableInOp() const;

	private:
		enum CoEAccess
		{
			READ_IN_SAFE_OP = 2,
			READ_IN_OP = 4,
			WRITE_IN_SAFE_OP = 16,
			WRITE_IN_OP = 32,
		};

		unsigned int index;
		unsigned int subIndex;
		unsigned int access;
	};

	/*!
	 * \brief The CoEObjectCode enum encodes the types of entries found in the CoE object
	 * dictionary.
	 */
	enum class CoEObjectCode
	{
		VAR = 7,
		ARRAY = 8,
		RECORD = 9,
	};

	/*!
	 * \brief The CoEEntry class uniquely identifies one entry in the CoE object
	 * dictionary of a slave. It is not a DataObject, but holds CoEObjects.
	 */
	class CoEEntry
	{
	public:
		/*!
		 * \brief Construct a new CoEEntry with the given parameters.
		 * \param slaveID the ID of the slave this CoEObject belongs to
		 * \param name the name of this CoEEntry
		 * \param index the index of this CoEEntry in the object dictionary of the slave
		 * \param objectCode the type of this CoEEntry
		 * \param subobjects the CoEObjects that are found at subindices of this CoEEntry
		 */
		CoEEntry(unsigned int slaveID, unsigned int index, CoEObjectCode objectCode,
		    std::string&& name, std::vector<CoEObject>&& subobjects);

		/*!
		 * \brief Get the ID of the slave this CoEEntry belongs to.
		 * \return the ID of this CoEEntry's slave
		 */
		unsigned int getSlaveID() const;

		/*!
		 * \brief Get the index of this CoEObject in the object dictionary of the slave.
		 * \return the index of this CoEObject
		 */
		unsigned int getIndex() const;

		/*!
		 * \brief Get the object code of this CoEEntry
		 * \return the object code of this CoEEntry
		 */
		CoEObjectCode getObjectCode() const;

		/*!
		 * \brief Get the name of this CoEEntry.
		 * \return the name of this CoEEntry
		 */
		const std::string& getName() const;

		/*!
		 * \brief Get the CoEObjects that are found at subindices of this CoEEntry.
		 * \return the CoEObjects at subindices of this CoEEntry
		 */
		const std::vector<CoEObject>& getObjects() const;

	private:
		unsigned int slaveID;
		unsigned int index;
		CoEObjectCode objectCode;
		std::string name;
		std::vector<CoEObject> subobjects;
	};

	struct CoEObjectHash
	{
		std::size_t operator()(const datatypes::CoEObject& coe) const
		{
			return std::hash<unsigned int>()(coe.getSlaveID())
			    ^ std::hash<unsigned int>()(coe.getIndex())
			    ^ std::hash<unsigned int>()(coe.getSubIndex())
			    ^ std::hash<std::string>()(coe.getName())
			    ^ std::hash<size_t>()(static_cast<size_t>(coe.getType()));
		}
	};

	struct CoEObjectEqual
	{
		bool operator()(const datatypes::CoEObject& lhs, const datatypes::CoEObject& rhs) const
		{
			return lhs.getSlaveID() == rhs.getSlaveID() && lhs.getIndex() == rhs.getIndex()
			    && lhs.getSubIndex() == rhs.getSubIndex() && lhs.getName() == rhs.getName()
			    && lhs.getType() == rhs.getType();
		}
	};

	/*!
	 * \brief The CoEInfo struct holds additional information about a CoE.
	 *
	 * This information is not stored in the CoE objects because it is only needed
	 * when reading and writing CoE objects, but not for using them.
	 */
	struct CoEInfo
	{
		size_t bitLength;
	};
} // namespace etherkitten::datatypes
