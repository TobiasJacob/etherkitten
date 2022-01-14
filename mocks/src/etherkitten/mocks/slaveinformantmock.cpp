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

#include "slaveinformantmock.hpp"

namespace etherkitten::reader
{

	MockSlaveInformant::MockSlaveInformant()
	{
		std::vector<std::array<unsigned int, 4>> neigbours = generateGraph();
		for (int slaveIndex = 1; slaveIndex <= 5; ++slaveIndex)
		{
			std::string name = random_string();
			std::vector<datatypes::PDO> pdos;
			std::vector<datatypes::CoEEntry> coes;
			std::vector<std::byte> esiBinary;
			esiBinary.push_back(std::byte(randomNumber(0, 100)));

			for (int i = 1; i <= 5; ++i)
			{
				pdos.push_back(generateMockedPDO(slaveIndex));
				coes.push_back(generateMockedCoEEntry(slaveIndex));
			}

			slaveInfos.emplace_back(slaveIndex, std::move(name), std::move(pdos), std::move(coes),
			    datatypes::esiparsermock::parseESI(std::vector<std::byte>()), std::move(esiBinary),
			    std::move(neigbours[slaveIndex]));
		}
	}

	unsigned int MockSlaveInformant::getSlaveCount() const { return 5; }

	const datatypes::SlaveInfo& MockSlaveInformant::getSlaveInfo(unsigned int slaveIndex) const
	{
		return slaveInfos.at(slaveIndex - 1);
	}

	uint64_t MockSlaveInformant::getIOMapSize() const { return 1337; }

	const datatypes::PDO MockSlaveInformant::generateMockedPDO(unsigned int slaveId) const
	{
		datatypes::PDODirection direction = randomNumber(0, 1) == 0
		    ? datatypes::PDODirection::INPUT
		    : datatypes::PDODirection::OUTPUT;
		return datatypes::PDO(slaveId, random_string(), randomEtherCATDataTypeEnum(),
		    randomNumber(0, 1000), direction);
	}

	const datatypes::CoEEntry MockSlaveInformant::generateMockedCoEEntry(unsigned int slaveId) const
	{
		datatypes::CoEObjectCode coeObjectCode;
		switch (randomNumber(0, 2))
		{
		case 0:
			coeObjectCode = datatypes::CoEObjectCode::ARRAY;
			break;
		case 1:
			coeObjectCode = datatypes::CoEObjectCode::RECORD;
			break;
		case 2:
			coeObjectCode = datatypes::CoEObjectCode::VAR;
			break;
		}

		std::vector<datatypes::CoEObject> coeObjects;
		for (int i = 0; i < 5; ++i)
		{
			coeObjects.push_back(generateMockedCoEObject(slaveId));
		}
		return datatypes::CoEEntry(
		    slaveId, randomNumber(0, 1000), coeObjectCode, random_string(), std::move(coeObjects));
	}

	std::vector<std::array<unsigned int, 4>> MockSlaveInformant::generateGraph() const
	{
		std::vector<std::array<unsigned int, 4>> neigbhours;
		unsigned int no = std::numeric_limits<unsigned int>::max();

		/*            0
		 *        1      2
		 *     3    4
		 *   5
		 */
		// 0 master not used
		neigbhours.push_back({ 1, 2, no, no });
		// 1
		neigbhours.push_back({ 0, 3, 4, no });
		// 2
		neigbhours.push_back({ 0, no, no, no });
		// 3
		neigbhours.push_back({ 1, 5, no, no });
		// 4
		neigbhours.push_back({ 1, no, no, no });
		// 5
		neigbhours.push_back({ 3, no, no, no });

		return neigbhours;
	}

	std::string MockSlaveInformant::random_string() const
	{
		std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
		std::random_device rd;
		std::mt19937 generator(rd());
		std::shuffle(str.begin(), str.end(), generator);
		return str.substr(0, 32); // assumes 32 < number of characters in str
	}

	datatypes::EtherCATDataTypeEnum MockSlaveInformant::randomEtherCATDataTypeEnum() const
	{
		switch (randomNumber(0, 2))
		{
		case 0:
			return datatypes::EtherCATDataTypeEnum::OCTET_STRING;
			break;
		case 1:
			return datatypes::EtherCATDataTypeEnum::BIT7;
			break;
		}
		return datatypes::EtherCATDataTypeEnum::INTEGER64;
	}

	datatypes::CoEObject MockSlaveInformant::generateMockedCoEObject(unsigned int slaveId) const
	{
		unsigned int access = pow(2, randomNumber(1, 4));
		if (access == 8)
		{
			access = 32;
		}
		return datatypes::CoEObject(slaveId, random_string(), randomEtherCATDataTypeEnum(),
		    randomNumber(0, 1000), randomNumber(0, 1000), access);
	}

	int MockSlaveInformant::randomNumber(int from, int to) const
	{
		static std::random_device rd;
		static std::mt19937 mtEngine(rd());
		static std::uniform_int_distribution dist(from, to);
		return dist(mtEngine);
	}

	const std::vector<datatypes::ErrorMessage> MockSlaveInformant::getInitializationErrors()
	{
		const std::vector<datatypes::ErrorMessage> errorsTemp = initErrors;
		initErrors = std::vector<datatypes::ErrorMessage>();
		return errorsTemp;
	}

} // namespace etherkitten::reader
