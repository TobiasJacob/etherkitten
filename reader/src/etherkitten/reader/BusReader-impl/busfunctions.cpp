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

#include "busfunctions.hpp"

#include "../endianness.hpp"

namespace etherkitten::reader::bReader
{
	std::vector<uint16_t> getSlaveConfiguredAddresses()
	{
		std::vector<uint16_t> configuredAddresses;
		configuredAddresses.reserve(ec_slavecount);
		for (int i = 1; i <= ec_slavecount; ++i)
		{
			configuredAddresses.push_back(ec_slave[i].configadr); // NOLINT
		}
		return configuredAddresses;
	}

	std::pair<int, int> sendAndReceiveEtherCATFrame(const EtherCATFrame* frame, size_t frameLength,
	    uint16_t slaveConfiguredAddress, const std::vector<size_t>& slaveAddressOffsets)
	{
		int bufferIndex = ecx_getindex(ecx_context.port);
		uint8_t* buffer
		    = reinterpret_cast<uint8_t*>(&(ecx_context.port->txbuf[bufferIndex])); // NOLINT

		// The Ethernet header will already be placed in the buffer, so skip it
		EtherCATFrame* framePlacement
		    = reinterpret_cast<EtherCATFrame*>(buffer + ETH_HEADERSIZE); // NOLINT
		std::memcpy(framePlacement, frame, frameLength);

		framePlacement->pduArea[1] = bufferIndex;
		for (size_t offset : slaveAddressOffsets)
		{
			*reinterpret_cast<uint16_t*>(framePlacement->pduArea + offset) // NOLINT
			    = flipBytesIfBigEndianHost(slaveConfiguredAddress);
		}

		ecx_context.port->txbuflength[bufferIndex] = ETH_HEADERSIZE + frameLength; // NOLINT

		static constexpr int registerTimeoutus = 100;
		// This is blocking, which seems good enough for now
		int workingCounter = ecx_srconfirm(ecx_context.port, bufferIndex, registerTimeoutus);

		return { workingCounter, bufferIndex };
	}

	std::optional<datatypes::ErrorMessage> convertECErrorToMessage()
	{
		std::string message = ec_elist2string();
		if (message.empty())
		{
			return {};
		}
		// Getting the correct slave here would involve regex or reimplementing the SOEM method.
		return { { "SOEM error: " + message, datatypes::ErrorSeverity::MEDIUM } };
	}
} // namespace etherkitten::reader::bReader
