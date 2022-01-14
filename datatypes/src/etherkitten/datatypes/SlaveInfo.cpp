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

#include "SlaveInfo.hpp"

#include "errorstatistic.hpp"
#include "esiparser.hpp"

namespace etherkitten::datatypes
{
	SlaveInfo::SlaveInfo(unsigned int id, std::string&& name, std::vector<PDO>&& pdos,
	    std::vector<CoEEntry>&& coes, ESIData&& esiData, std::vector<std::byte>&& esiBinary,
	    std::array<unsigned int, 4>&& neighbors)
	    : id(id)
	    , name(name)
	    , pdos(pdos)
	    , coes(coes)
	    , registers(makeRegisterVector(id))
	    , statistics(makeErrorStatisticVector(id))
	    , esiBinary(esiBinary)
	    , esiData(esiData)
	    , neighbors(neighbors)
	{
	}

	SlaveInfo::SlaveInfo(unsigned int id, std::string&& name, std::vector<PDO>&& pdos,
	    std::vector<CoEEntry>&& coes, std::array<unsigned int, 4>&& neighbors)
	    : id(id)
	    , name(name)
	    , pdos(pdos)
	    , coes(coes)
	    , registers(makeRegisterVector(id))
	    , statistics(makeErrorStatisticVector(id))
	    , neighbors(neighbors)
	{
	}

	unsigned int SlaveInfo::getID() const { return this->id; }

	const std::string& SlaveInfo::getName() const { return this->name; }

	const std::vector<PDO>& SlaveInfo::getPDOs() const { return this->pdos; }

	const std::vector<CoEEntry>& SlaveInfo::getCoEs() const { return this->coes; }

	const std::vector<Register>& SlaveInfo::getRegisters() const { return this->registers; }

	const std::vector<ErrorStatistic>& SlaveInfo::getErrorStatistics() const
	{
		return this->statistics;
	}

	const std::optional<ESIData>& SlaveInfo::getESI() const { return this->esiData; }

	const std::vector<std::byte>& SlaveInfo::getESIBinary() const { return this->esiBinary; }

	const std::array<unsigned int, 4>& SlaveInfo::getNeighbors() const { return this->neighbors; }

	std::vector<Register> SlaveInfo::makeRegisterVector(unsigned int id)
	{
		std::vector<Register> result;
		result.reserve(registerMap.size());
		for (const auto& reg : registerMap)
		{
			result.emplace_back(id, reg.first);
		}
		return result;
	}

	std::vector<ErrorStatistic> SlaveInfo::makeErrorStatisticVector(unsigned int id)
	{
		std::vector<ErrorStatistic> result;
		for (ErrorStatisticInfo error : errorStatisticInfos)
		{
			result.emplace_back(
			    ErrorStatistic{ id, error.getType(ErrorStatisticCategory::TOTAL_SLAVE) });
			result.emplace_back(
			    ErrorStatistic{ id, error.getType(ErrorStatisticCategory::FREQ_SLAVE) });
		}
		return result;
	}
} // namespace etherkitten::datatypes
