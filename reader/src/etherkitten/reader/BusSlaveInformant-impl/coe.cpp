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

#include <sstream>

#include <etherkitten/datatypes/dataobjects.hpp>

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief Read all subobjects of an entry in the object dictionary into a vector.
	 *
	 * This method does not access the EtherCAT bus. It only reads from already-fetched
	 * SOEM data structures and parses them. Both the objectDictionary and the objectEntry
	 * must already be initialized by their corresponding SOEM functions `ec_readODlist`,
	 * `ec_readODdescription` and `ec_readOE`.
	 * \param  objectDictionary the object dictionary SOEM provides
	 * \param  entryIndex the index into SOEM's object dictionary arrays (NOT the object's index
	 * in the CoE dictionary)
	 * \param  objectEntry the object entry SOEM provides
	 * \return a list of the subobjects of the given object along with additional information in a
	 * map
	 */
	std::pair<std::vector<datatypes::CoEObject>,
	    std::unordered_map<datatypes::CoEObject, datatypes::CoEInfo, datatypes::CoEObjectHash,
	        datatypes::CoEObjectEqual>>
	readSubObjects(ec_ODlistt objectDictionary, size_t entryIndex, ec_OElistt objectEntry)
	{
		std::vector<datatypes::CoEObject> list;
		std::unordered_map<datatypes::CoEObject, datatypes::CoEInfo, datatypes::CoEObjectHash,
		    datatypes::CoEObjectEqual>
		    infoMap;

		// NOLINTNEXTLINE
		for (size_t subobjectIndex = 0; subobjectIndex <= objectDictionary.MaxSub[entryIndex];
		     ++subobjectIndex)
		{
			// Skip over empty ("padding") subobjects
			if (objectEntry.DataType[subobjectIndex] == 0 // NOLINT
			    || objectEntry.BitLength[subobjectIndex] == 0) // NOLINT
			{
				continue;
			}
			datatypes::CoEObject object(objectDictionary.Slave,
			    std::string(objectEntry.Name[subobjectIndex]), // NOLINT
			    static_cast<datatypes::EtherCATDataTypeEnum>(
			        objectEntry.DataType[subobjectIndex]), // NOLINT
			    objectDictionary.Index[entryIndex], subobjectIndex, // NOLINT
			    objectEntry.ObjAccess[subobjectIndex]); // NOLINT
			list.push_back(object); // NOLINT
			infoMap[object] = { objectEntry.BitLength[subobjectIndex] }; // NOLINT
		}

		return { list, infoMap };
	}

	MayError<CoEResult> readObjectDictionary(unsigned int slave)
	{
		std::vector<datatypes::CoEEntry> list;
		std::unordered_map<datatypes::CoEObject, datatypes::CoEInfo, datatypes::CoEObjectHash,
		    datatypes::CoEObjectEqual>
		    info;
		std::vector<ErrorMessage> errors;

		ec_ODlistt objectDictionary{};

		if (ec_readODlist(slave, &objectDictionary) == 0)
		{
			errors.emplace_back("Failed to read this slave's object dictionary -"
			                    " could not read the OD list."
			                    " Some features may not be available for this slave.",
			    slave, ErrorSeverity::MEDIUM);
			addSOEMErrorsToVector(errors, ErrorSeverity::MEDIUM);
			return { { list, {} }, errors };
		}

		for (size_t entryIndex = 0; entryIndex < objectDictionary.Entries; ++entryIndex)
		{
			ec_OElistt objectEntry{};
			if (ec_readODdescription(entryIndex, &objectDictionary) == 0
			    || ec_readOE(entryIndex, &objectDictionary, &objectEntry) == 0)
			{
				std::stringstream errorStream;
				errorStream
				    << "Failed to read this slave's object description or object entry for index "
				    << std::hex << std::showbase << objectDictionary.Index[entryIndex] // NOLINT
				    << " - the slave did not respond."
				    << " This CoE index will not be available.";
				errors.emplace_back(errorStream.str(), slave, ErrorSeverity::LOW);
				addSOEMErrorsToVector(errors, ErrorSeverity::LOW);
				continue;
			}

			auto [subobjects, newInfos] = readSubObjects(objectDictionary, entryIndex, objectEntry);

			list.emplace_back(slave, objectDictionary.Index[entryIndex], // NOLINT
			    static_cast<datatypes::CoEObjectCode>(
			        objectDictionary.ObjectCode[entryIndex]), // NOLINT
			    std::string(objectDictionary.Name[entryIndex]), std::move(subobjects)); // NOLINT
			info.insert(newInfos.begin(), newInfos.end());
		}

		return { { list, info }, errors };
	}
} // namespace etherkitten::reader::bSInformant
