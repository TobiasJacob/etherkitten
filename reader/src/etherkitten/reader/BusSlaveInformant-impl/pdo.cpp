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

#include <optional>
#include <sstream>

#include <etherkitten/datatypes/dataobjects.hpp>

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief Holds information read from a single entry in a PDO mapping.
	 */
	struct PDOSubObject
	{
		bool failed{}; /*!< Whether reading the subobject failed */
		size_t bitLength{}; /*!< The length of the subobject in bits */
		uint16_t objectIndex{}; /*!< The index of the subobject in the CoE dictionary */
		std::optional<datatypes::PDO> pdo; /*!< The subobject, if available */
	};

	/*!
	 * \brief Try to construct a PDO from its entry in a PDO mapping and the CoE dictionary
	 *
	 * If the mapping has an index and subindex of 0, no PDO will be constructed but the
	 * bit length of the gap will be returned. This is not an error.
	 * If the mapping has a nonzero index or subindex, the given CoE entries will be searched
	 * for the matching object. If it couldn't be found, `failed` will be set in the return
	 * value and no PDO will be constructed. However, the bit length will still be accurate.
	 * \param slave the slave to assign the PDO to
	 * \param pdoIndex the index to assign to the PDO - this is an internal identification
	 * index, not an index in the CoE dictionary
	 * \param mappedObject the PDO mapping entry to construct the PDO from as read from the CoE
	 * dictionary
	 * \param dir the PDODirection to assign to the PDO
	 * \param coes the CoE entries to search through for the object corresponding to the PDO
	 * \return the PDO along with metadata if successful
	 */
	PDOSubObject constructSubObject(unsigned int slave, unsigned int pdoIndex,
	    uint32_t mappedObject, datatypes::PDODirection dir,
	    const std::vector<datatypes::CoEEntry>& coes)
	{
		static const size_t objectIndexStartBit = 16;
		static const size_t objectSubIndexStartBit = 8;
		static const size_t byteMask = 0xFF;

		uint16_t objectIndex = static_cast<uint16_t>(mappedObject >> objectIndexStartBit);
		uint8_t objectSubIndex
		    = static_cast<uint8_t>((mappedObject >> objectSubIndexStartBit) & byteMask);
		uint8_t objectBitLength = static_cast<uint8_t>(mappedObject & byteMask);

		if (objectIndex == 0 && objectSubIndex == 0)
		{
			// This is a padding object, only relevant for the bit offset.
			return { false, objectBitLength, objectIndex, {} };
		}

		auto it = std::find_if(
		    coes.begin(), coes.end(), [objectIndex](const datatypes::CoEEntry& entry) {
			    return entry.getIndex() == objectIndex;
		    });
		if (it == coes.end())
		{
			return { true, objectBitLength, objectIndex, {} };
		}

		datatypes::CoEEntry entry = *it;
		datatypes::CoEObject subobject = entry.getObjects()[objectSubIndex];
		std::string name(subobject.getName());
		datatypes::EtherCATDataTypeEnum type(subobject.getType());
		return { false, objectBitLength, objectIndex,
			datatypes::PDO{ slave, std::move(name), type, pdoIndex, dir } };
	}

	/*!
	 * \brief Read a PDO Mapping object from the CoE dictionary.
	 *
	 * Stores the read PDOs and their offsets in inProgressResult.
	 * If the returned errors contain a MEDIUM severity error, a PDO mapping
	 * could not be read and PDO reading for this slave must be stopped to
	 * keep the bitOffset consistent.
	 * If the coes do not contain the object a PDO refers to, that PDO will
	 * not be made available in the PDOList, but its offset will be counted.
	 * \param slave the slave to read a PDO mapping of
	 * \param pdoMappingIndex the index of the PDO mapping in the object dictionary
	 * \param dir the direction of the PDO mapping
	 * \param coes CoE entries to read the name and type of PDOs from
	 * \param[in,out] inProgressResult the PDOList and PDOOffsets to write results to
	 * \param[in,out] bitOffset will be updated by the length of all read PDOs
	 * \return any errors that occured during the reading process
	 */
	std::vector<ErrorMessage> readPDOMappingStruct(unsigned int slave, uint16_t pdoMappingIndex,
	    datatypes::PDODirection dir, const std::vector<datatypes::CoEEntry>& coes,
	    PDOResult& inProgressResult, size_t& bitOffset)
	{
		std::vector<ErrorMessage> errors;

		std::optional<uint8_t> mappedObjectCount
		    = performSDOread<uint8_t>(slave, pdoMappingIndex, 0);
		if (!mappedObjectCount.has_value())
		{
			std::stringstream errorStream;
			errorStream << "Failed to map this slave's PDOs - could not read the number of"
			               " mapped objects in PDO mapping "
			            << std::hex << std::showbase << pdoMappingIndex
			            << ". Some PDOs may not be available for this slave.";
			errors.emplace_back(errorStream.str(), slave, ErrorSeverity::MEDIUM);
			addSOEMErrorsToVector(errors, slave, ErrorSeverity::MEDIUM);
			return errors;
		}

		for (uint8_t mappedObjectIndex = 0; mappedObjectIndex < mappedObjectCount;
		     ++mappedObjectIndex)
		{
			std::optional<uint32_t> mappedObject
			    = performSDOread<uint32_t>(slave, pdoMappingIndex, mappedObjectIndex + 1);
			if (!mappedObject.has_value())
			{
				std::stringstream errorStream;
				errorStream << "Failed to map this slave's PDOs - could not read mapped object "
				            << mappedObjectIndex << " in PDO mapping " << std::hex << std::showbase
				            << pdoMappingIndex
				            << ". Some PDOs may not be available for this slave.";
				// We have to return here because we would generate incorrect bit offsets otherwise.
				errors.emplace_back(errorStream.str(), slave, ErrorSeverity::MEDIUM);
				addSOEMErrorsToVector(errors, slave, ErrorSeverity::MEDIUM);
				return errors;
			}
			PDOSubObject pdoSub = constructSubObject(
			    slave, inProgressResult.first.size(), mappedObject.value(), dir, coes);
			bitOffset += pdoSub.bitLength;
			if (pdoSub.failed)
			{
				std::stringstream errorStream;
				errorStream << "Failed to read PDO mapping " << std::hex << std::showbase
				            << pdoMappingIndex << "'s corresponding CoE Object (Index "
				            << mappedObjectIndex << ") - the object dictionary index " << std::hex
				            << std::showbase << pdoSub.objectIndex
				            << " is not in the object dictionary."
				               " This PDO will not be available for this slave.";
				errors.emplace_back(errorStream.str(), slave, ErrorSeverity::LOW);
			}
			else if (pdoSub.pdo.has_value())
			{
				inProgressResult.first.push_back(pdoSub.pdo.value());
				inProgressResult.second.insert(
				    { pdoSub.pdo.value(), { bitOffset - pdoSub.bitLength, pdoSub.bitLength } });
			}
		}
		return errors;
	}

	/*!
	 * \brief Read a PDO Assign object for a specific Sync Manager from the object dictionary.
	 *
	 * Stores the read PDOs and their offsets in inProgressResult.
	 * If the returned errors contain a MEDIUM severity error, a PDO mapping
	 * could not be read and PDO reading for this slave must be stopped to
	 * keep the bitOffset consistent.
	 * If the coes do not contain the object a PDO refers to, that PDO will
	 * not be made available in the PDOList, but its offset will be counted.
	 * \param slave the slave to read the PDO Assign object of
	 * \param syncManagerIndex the index of the Sync Manager to read the PDO Assign struct of
	 * (must be in the range 2-31)
	 * \param dir the direction of the given Sync Manager
	 * \param coes CoE entries to read the name and type of PDOs from
	 * \param[in,out] inProgressResult the PDOList and PDOOffsets to write results to
	 * \param[in,out] bitOffset will be updated by the length of all read PDOs
	 * \return any errors that occured during the reading process
	 */
	std::vector<ErrorMessage> readPDOAssignStruct(unsigned int slave, unsigned int syncManagerIndex,
	    datatypes::PDODirection dir, const std::vector<datatypes::CoEEntry>& coes,
	    PDOResult& inProgressResult, size_t& bitOffset)
	{
		std::vector<ErrorMessage> errors;

		std::optional<uint8_t> pdoMappingCount
		    = performSDOread<uint8_t>(slave, ECT_SDO_PDOASSIGN + syncManagerIndex, 0);
		if (!pdoMappingCount.has_value())
		{
			std::stringstream errorStream;
			errorStream << "Failed to read this slave's PDO Assignment struct for Sync Manager "
			            << syncManagerIndex
			            << " - could not read the number of PDO mappings."
			               " Some PDOs may not be available for this slave.";
			errors.emplace_back(errorStream.str(), slave, ErrorSeverity::MEDIUM);
			addSOEMErrorsToVector(errors, slave, ErrorSeverity::MEDIUM);
			return errors;
		}

		for (uint8_t pdoAssignIndex = 0; pdoAssignIndex < pdoMappingCount; ++pdoAssignIndex)
		{
			std::optional<uint16_t> pdoMapping = performSDOread<uint16_t>(
			    slave, ECT_SDO_PDOASSIGN + syncManagerIndex, pdoAssignIndex + 1);
			if (!pdoMapping.has_value())
			{
				std::stringstream errorStream;
				errorStream << "Failed to read this slave's PDO mapping index for Sync Manager "
				            << syncManagerIndex << " and mapping index " << pdoAssignIndex
				            << ". Some PDOs may not be available for this slave.";
				errors.emplace_back(errorStream.str(), slave, ErrorSeverity::MEDIUM);
				addSOEMErrorsToVector(errors, slave, ErrorSeverity::MEDIUM);
				return errors;
			}
			auto errors = readPDOMappingStruct(
			    slave, pdoMapping.value(), dir, coes, inProgressResult, bitOffset);
			errors.insert(errors.end(), errors.begin(), errors.end());
			if (!containsOnlyLowSeverityErrors(errors))
			{
				return errors;
			}
		}

		return errors;
	}

	MayError<PDOResult> readPDOsViaCoE(
	    unsigned int slave, const std::vector<datatypes::CoEEntry>& coes)
	{
		// Reading the PDO mappings looks like this when the slave supports CoE:
		// - Read Sync Manager communication type struct from 0x1C00 (ECT_SDO_SMCOMMTYPE, 6-77)
		// - Iterate over SM comm type list
		//  * (first couple SMs have fixed comm types, check footnote a) on 6-77)
		//  * If a SM entry is RxPDO or TxPDO:
		//   + Read corresponding PDO Assign (0x1C10 upwards, ECT_SDO_PDOASSIGN, 6-73/78)
		//   + Go through list of PDO mappings in PDO Assign:
		//    > Read PDO mapping (0x1600-0x17FF, 0x1A00-0x1BFF, 6-73/76)
		//    > For every object in mapping:
		//     - Read entry -> get index, subindex, bit length
		//     - Read object from object dictionary to get name and data type

		std::vector<ErrorMessage> errors;

		std::optional<uint8_t> syncManagerCount
		    = performSDOread<uint8_t>(slave, ECT_SDO_SMCOMMTYPE, 0);
		if (!syncManagerCount.has_value())
		{
			std::stringstream errorStream;
			errorStream << "Failed to read this slave's Sync Manager count (Index " << std::hex
			            << std::showbase << ECT_SDO_SMCOMMTYPE
			            << "). PDOs will not be available for this slave.";
			errors.emplace_back(errorStream.str(), slave, ErrorSeverity::MEDIUM);
			addSOEMErrorsToVector(errors, slave, ErrorSeverity::MEDIUM);
			return { {}, errors };
		}

		PDOResult result;
		size_t inputBitOffset = 0;
		size_t outputBitOffset = 0;
		// We skip over the first two sync managers - they're always responsible for the
		// mailbox. See the EtherCAT spec. part 6, page 77.
		for (uint8_t syncMIndex = 2; syncMIndex < syncManagerCount; ++syncMIndex)
		{
			std::optional<uint8_t> syncMType
			    = performSDOread<uint8_t>(slave, ECT_SDO_SMCOMMTYPE, syncMIndex + 1);
			if (!syncMType.has_value())
			{
				std::stringstream errorStream;
				errorStream << "Failed to read this slave's Sync Manager type for SM " << syncMIndex
				            << ". Some PDOs may not be available for this slave.";
				errors.emplace_back(errorStream.str(), slave, ErrorSeverity::MEDIUM);
				addSOEMErrorsToVector(errors, slave, ErrorSeverity::MEDIUM);
				return { result, errors };
			}

			// SOEM does some attempted error correction for malconfigured slaves here.
			// I've omitted it for simplicity, but it may become necessary later.

			std::vector<ErrorMessage> newErrors;

			SyncMTypes type = static_cast<SyncMTypes>(syncMType.value());
			if (type == SyncMTypes::PDO_MASTER_TO_SLAVE)
			{
				// These are referred to as RxPDOs in EtherCAT-speak
				newErrors = readPDOAssignStruct(slave, syncMIndex, datatypes::PDODirection::INPUT,
				    coes, result, inputBitOffset);
			}
			else if (type == SyncMTypes::PDO_SLAVE_TO_MASTER)
			{
				// These are referred to as TxPDOs in EtherCAT-speak
				newErrors = readPDOAssignStruct(slave, syncMIndex, datatypes::PDODirection::OUTPUT,
				    coes, result, outputBitOffset);
			}

			errors.insert(errors.end(), newErrors.begin(), newErrors.end());
			if (!containsOnlyLowSeverityErrors(newErrors))
			{
				return { result, errors };
			}
		}

		return { result, errors };
	}

	/*!
	 * \brief Add the PDO objects from one ESIPDOObject list to the in-progress result.
	 * \param slave the slave to assign the PDOs to
	 * \param strings the Strings section of the ESI data of that slave
	 * \param objects the ESIPDOObject list to read from
	 * \param direction the direction to assign to the PDOs
	 * \param[in,out] inProgressResult the result to store the PDOs and their offsets in
	 */
	void addESIPDOObjectsToPDOResult(unsigned int slave, const datatypes::ESIStrings& strings,
	    const std::vector<datatypes::ESIPDOObject>& objects, datatypes::PDODirection direction,
	    PDOResult& inProgressResult)
	{
		size_t bitOffset = 0;
		for (const datatypes::ESIPDOObject& pdo : objects)
		{
			for (const datatypes::ESIPDOEntry& pdoEntry : pdo.entries)
			{
				inProgressResult.first.push_back(
				    datatypes::PDO(slave, std::string(strings[pdoEntry.nameIdx]),
				        static_cast<datatypes::EtherCATDataTypeEnum>(pdoEntry.dataType),
				        inProgressResult.first.size(), direction));
				inProgressResult.second[inProgressResult.first.back()]
				    = { bitOffset, pdoEntry.bitLength };
				bitOffset += pdoEntry.bitLength;
			}
		}
	}

	MayError<PDOResult> readPDOsViaESI(unsigned int slave, const datatypes::ESIData& esiData)
	{
		PDOResult result;

		addESIPDOObjectsToPDOResult(
		    slave, esiData.strings, esiData.txPDO, datatypes::PDODirection::INPUT, result);
		addESIPDOObjectsToPDOResult(
		    slave, esiData.strings, esiData.rxPDO, datatypes::PDODirection::OUTPUT, result);

		return { result, {} };
	}

} // namespace etherkitten::reader::bSInformant
