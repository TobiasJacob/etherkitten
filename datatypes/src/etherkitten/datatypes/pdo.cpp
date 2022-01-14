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

#include "pdo.hpp"

#include "DataObjectVisitor.hpp"

namespace etherkitten::datatypes
{
	PDO::PDO(unsigned int slaveID, std::string&& name, EtherCATDataTypeEnum type,
	    unsigned int index, PDODirection direction)
	    : DataObject(slaveID, std::move(name), type)
	    , index(index)
	    , direction(direction)
	{
	}

	void PDO::acceptVisitor(DataObjectVisitor& visitor) const { visitor.handlePDO(*this); }

	unsigned int PDO::getIndex() const { return this->index; }

	PDODirection PDO::getDirection() const { return this->direction; }
} // namespace etherkitten::datatypes
