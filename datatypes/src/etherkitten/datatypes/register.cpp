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

#include "register.hpp"

#include "DataObjectVisitor.hpp"

namespace etherkitten::datatypes
{
	size_t getRegisterByteLength(datatypes::RegisterEnum reg)
	{
		static constexpr size_t byteSize = 8;
		size_t bitLength = datatypes::registerMap.at(reg).bitLength;
		// We only do bytes 'round these parts, kid.
		if (bitLength < byteSize)
		{
			bitLength = byteSize;
		}
		return bitLength / byteSize;
	}

	Register::Register(unsigned int slaveID, RegisterEnum registerType)
	    : DataObject(slaveID, std::string(registerMap.at(registerType).name),
	          registerMap.at(registerType).enumType)
	    , type(registerType)
	{
	}

	void Register::acceptVisitor(DataObjectVisitor& visitor) const
	{
		visitor.handleRegister(*this);
	}

	RegisterEnum Register::getRegister() const { return this->type; }
} // namespace etherkitten::datatypes
