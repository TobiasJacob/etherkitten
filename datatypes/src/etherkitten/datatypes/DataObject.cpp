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


#include "dataobjects.hpp"

#include <utility>

#include "DataObjectVisitor.hpp"

namespace etherkitten::datatypes
{
	DataObject::DataObject(unsigned int slaveID, std::string&& name, EtherCATDataTypeEnum type)
	    : slaveID(slaveID)
	    , name(name)
	    , type(type)
	{
	}

	DataObject::~DataObject() {}

	unsigned int DataObject::getSlaveID() const { return this->slaveID; }

	std::string DataObject::getName() const { return this->name; }

	EtherCATDataTypeEnum DataObject::getType() const { return this->type; }

} // namespace etherkitten::datatypes