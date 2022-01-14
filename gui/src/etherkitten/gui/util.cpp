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

#include "util.hpp"

namespace etherkitten::gui
{

	bool compareRegisters(datatypes::RegisterEnum a, datatypes::RegisterEnum b)
	{
		return (static_cast<int>(a) & 0xFFFF) == (static_cast<int>(b) & 0xFFFF)
		    ? a < b
		    : (static_cast<int>(a) & 0xFFFF) < (static_cast<int>(b) & 0xFFFF);
	}

	bool dataObjectIsNumeric(const datatypes::DataObject& obj)
	{
		using datatypes::EtherCATDataTypeEnum;
		datatypes::EtherCATDataTypeEnum t = obj.getType();
		return t == EtherCATDataTypeEnum::INTEGER8 || t == EtherCATDataTypeEnum::INTEGER16
		    || t == EtherCATDataTypeEnum::INTEGER24 || t == EtherCATDataTypeEnum::INTEGER32
		    || t == EtherCATDataTypeEnum::INTEGER64 || t == EtherCATDataTypeEnum::UNSIGNED8
		    || t == EtherCATDataTypeEnum::UNSIGNED16 || t == EtherCATDataTypeEnum::UNSIGNED24
		    || t == EtherCATDataTypeEnum::UNSIGNED32 || t == EtherCATDataTypeEnum::UNSIGNED64
		    || t == EtherCATDataTypeEnum::REAL32 || t == EtherCATDataTypeEnum::REAL64
		    || t == EtherCATDataTypeEnum::BIT1 || t == EtherCATDataTypeEnum::BIT2
		    || t == EtherCATDataTypeEnum::BIT3 || t == EtherCATDataTypeEnum::BIT4
		    || t == EtherCATDataTypeEnum::BIT5 || t == EtherCATDataTypeEnum::BYTE
		    || t == EtherCATDataTypeEnum::BIT6 || t == EtherCATDataTypeEnum::BIT7
		    || t == EtherCATDataTypeEnum::BIT8;
	}

} // namespace etherkitten::gui
