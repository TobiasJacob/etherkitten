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

#include "coe.hpp"

#include "DataObjectVisitor.hpp"

namespace etherkitten::datatypes
{
	CoEEntry::CoEEntry(unsigned int slaveID, unsigned int index, CoEObjectCode objectCode,
	    std::string&& name, std::vector<CoEObject>&& subobjects)
	    : slaveID(slaveID)
	    , index(index)
	    , objectCode(objectCode)
	    , name(name)
	    , subobjects(subobjects)
	{
	}

	unsigned int CoEEntry::getSlaveID() const { return this->slaveID; }

	unsigned int CoEEntry::getIndex() const { return this->index; }

	CoEObjectCode CoEEntry::getObjectCode() const { return this->objectCode; }

	const std::string& CoEEntry::getName() const { return this->name; }

	const std::vector<CoEObject>& CoEEntry::getObjects() const { return this->subobjects; }

	CoEObject::CoEObject(unsigned int slaveID, std::string&& name, EtherCATDataTypeEnum type,
	    unsigned int index, unsigned int subIndex, unsigned int access)
	    : DataObject(slaveID, std::move(name), type)
	    , index(index)
	    , subIndex(subIndex)
	    , access(access)
	{
	}

	void CoEObject::acceptVisitor(DataObjectVisitor& visitor) const { visitor.handleCoE(*this); }

	unsigned int CoEObject::getIndex() const { return this->index; }

	unsigned int CoEObject::getSubIndex() const { return this->subIndex; }

	bool CoEObject::isReadableInSafeOp() const
	{
		return (this->access & CoEAccess::READ_IN_SAFE_OP) != 0;
	}

	bool CoEObject::isReadableInOp() const { return (this->access & CoEAccess::READ_IN_OP) != 0; }

	bool CoEObject::isWritableInSafeOp() const
	{
		return (this->access & CoEAccess::WRITE_IN_SAFE_OP) != 0;
	}

	bool CoEObject::isWritableInOp() const { return (this->access & CoEAccess::WRITE_IN_OP) != 0; }
} // namespace etherkitten::datatypes
