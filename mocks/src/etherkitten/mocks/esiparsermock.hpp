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

#include <algorithm>
#include <etherkitten/datatypes/esidata.hpp>
#include <random>
#include <string>

namespace etherkitten::datatypes::esiparsermock
{
	ESIData parseESI(const std::vector<std::byte>& esiBinary);

	EtherCATDataType::UNSIGNED8 getStringIdx();

	EtherCATDataType::UNSIGNED8 getU8(const std::vector<std::byte>& esiBinary, size_t address);

	EtherCATDataType::UNSIGNED16 getU16(const std::vector<std::byte>& esiBinary, size_t address);

	EtherCATDataType::INTEGER16 get16(const std::vector<std::byte>& esiBinary, size_t address);

	EtherCATDataType::UNSIGNED16 getU16W(
	    const std::vector<std::byte>& esiBinary, size_t wordAddress);

	EtherCATDataType::UNSIGNED32 getU32(const std::vector<std::byte>& esiBinary, size_t address);

	EtherCATDataType::UNSIGNED32 getU32W(
	    const std::vector<std::byte>& esiBinary, size_t wordAddress);

	ESIHeader parseHeader(const std::vector<std::byte>& esiBinary);

	std::string random_string();

	std::vector<std::string> parseStrings(
	    const std::vector<std::byte>& esiBinary, size_t wordOffset);

	ESIGeneral parseGeneral(const std::vector<std::byte>& esiBinary, size_t wordOffset);

	ESIFMMU
	parseFMMU(const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len);

	ESISyncM parseSyncM(const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len);

	std::vector<ESIPDOObject> parsePDOs(
	    const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len);

} // namespace etherkitten::datatypes::esiparsermock
