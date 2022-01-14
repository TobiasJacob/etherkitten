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


#include "esi.hpp"

#include <vector>

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief Read the EEPROM of the given slave from address 0 to the given address.
	 *
	 * This method does not fail when it should because SOEM doesn't return errors
	 * from the functions used here. Instead, it returns undefined values.
	 * \param slaveConfiguredAddress the configured address of the slave to read the EEPROM of
	 * \param toSIIAddress which address to read to (exclusive if aligned to the packet size,
	 * inclusive otherwise)
	 * \param has64BitPackets whether the slave supports reading 64 bits or only 32 bits in one
	 * request
	 * \return the bytes read from the EEPROM, or undefined bytes in case of read failures
	 */
	std::vector<std::byte> readFromEEPROM(
	    uint16_t slaveConfiguredAddress, uint16_t toSIIAddress, bool has64BitPackets)
	{
		std::vector<std::byte> result;

		// Some slaves return data from the EEPROM in 4 byte (2 word) units,
		// others in 8 byte (4 word) units.
		uint16_t addressIncr = 2;
		if (has64BitPackets)
		{
			addressIncr = 4;
		}

		result.reserve(toSIIAddress * addressIncr * 2);

		for (uint16_t currentAddress = 0; currentAddress < toSIIAddress;
		     currentAddress += addressIncr)
		{
			uint64_t eepromData
			    = ec_readeepromFP(slaveConfiguredAddress, currentAddress, EC_TIMEOUTEEP);

			// Repack the result into some std::bytes
			for (int i = 0; i < addressIncr * 2; ++i)
			{
				static const int byteLength = 8;
				static const int byteMask = 0xFF;
				result.push_back(std::byte((eepromData >> (i * byteLength)) & byteMask));
			}
		}

		return result;
	}

	MayError<std::vector<std::byte>> readESIBinary(unsigned int slave)
	{
		std::vector<ErrorMessage> errors;

		// First, we find the "end category" section marker.
		// esiEnd will point to the byte AFTER the section marker.
		static const int16_t endCategoryMarker = 0xFFFF;
		uint16_t esiEnd = ec_siifind(slave, endCategoryMarker) + 2;
		if (esiEnd == 0)
		{
			// This is a bug in the slave.
			errors.emplace_back("Failed to read this slave's ESI - could not find"
			                    " the required end category (category 0xFF)."
			                    " Some features may not be available for this slave.",
			    slave, ErrorSeverity::MEDIUM);
			addSOEMErrorsToVector(errors, ErrorSeverity::MEDIUM);
			return { {}, errors };
		}

		// SOEM starts reading the EEPROM at word address 0x40 for some reason,
		// so we can't use ec_esidump here or we'd be missing the header.

		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		bool pdiHadEEPROMControl = (bool)ec_slave[slave].eep_pdi;
		if (ec_eeprom2master(slave) == 0)
		{
			errors.emplace_back("Failed to read this slave's ESI -"
			                    " could not get control of the EEPROM."
			                    " Some features may not be available for this slave.",
			    slave, ErrorSeverity::MEDIUM);
			addSOEMErrorsToVector(errors, ErrorSeverity::MEDIUM);
			return { {}, errors };
		}

		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		uint16_t slaveConfiguredAddress = ec_slave[slave].configadr;

		std::vector<std::byte> result = readFromEEPROM(
		    slaveConfiguredAddress, esiEnd, ec_slave[slave].eep_8byte > 0); // NOLINT

		if (pdiHadEEPROMControl)
		{
			ec_eeprom2pdi(slave);
		}

		return { result, errors };
	}
} // namespace etherkitten::reader::bSInformant
