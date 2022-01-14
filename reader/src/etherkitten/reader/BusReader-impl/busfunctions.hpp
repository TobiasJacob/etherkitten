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
/*!
 * \file
 * \brief Defines functions used by the BusReader to interact with the EtherCAT bus.
 */

/*!
 * \namespace etherkitten::reader::bReader
 * \brief Defines helper methods for the implementation of the BusReader.
 */

#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

extern "C"
{
#include <ethercat.h>
}

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>

#include "../EtherCATFrame.hpp"

namespace etherkitten::reader::bReader
{
	/*!
	 * \brief Get the configured addresses of all the slaves on the bus in a vector.
	 *
	 * The slaves are 1-indexed, so the matching configured address will be at index 0
	 * in the returned vector.
	 * \return a vector of all the configured addresses of the slaves
	 */
	std::vector<uint16_t> getSlaveConfiguredAddresses();

	/*!
	 * \brief Sends an EtherCAT frame over the EtherCAT bus, receives it and returns the
	 * index of the SOEM buffer the resulting frame was placed in along with the working
	 * counter returned by SOEM.
	 *
	 * This method will automatically place the given slave address at the given offsets
	 * in the final EtherCAT frame in the buffer. Pass an empty offset vector if you do not
	 * require this.
	 * \param frame the EtherCAT frame to send over the bus
	 * \param frameLength the length of the EtherCAT frame in bytes
	 * \param slaveConfiguredAddress the configured address of the slave to send the frame to,
	 * if it needs to be set in the frame
	 * \param slaveAddressOffsets the offsets in the EtherCAT frame to write the configured
	 * address of the slave to
	 * \return the working counter of the frame as returned by SOEM and the index of the SOEM
	 * rx frame buffer the received frame has been placed in.
	 */
	std::pair<int, int> sendAndReceiveEtherCATFrame(const EtherCATFrame* frame, size_t frameLength,
	    uint16_t slaveConfiguredAddress, const std::vector<size_t>& slaveAddressOffsets);

	/*!
	 * \brief Pop a SOEM error from the error stack and convert it to an ErrorMessage.
	 * \return an error message, or nothing if the SOEM stack was empty
	 */
	std::optional<datatypes::ErrorMessage> convertECErrorToMessage();

	static constexpr size_t byteSize = 8;

	/*!
	 * \brief Writes the data in an AbstractDataPoint to an arbitrary destination.
	 *
	 * To be used with the dataTypeMap without strings.
	 * \tparam E the type of the data point
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class AbstractDataPointWriter
	{
	public:
		using product_t = std::function<void(const datatypes::AbstractDataPoint&, void* dest,
		    size_t inByteOffset, size_t bitLength)>;

		static product_t eval()
		{
			return [](const datatypes::AbstractDataPoint& point, void* dest, size_t inByteOffset,
			           size_t bitLength) {
				using T = typename datatypes::TypeMap<E>::type;
				const datatypes::DataPoint<T>& realPoint
				    = dynamic_cast<const datatypes::DataPoint<T>&>(point);
				uint64_t asUInt = 0;
				if constexpr (datatypes::is_bitset<T>())
				{
					asUInt = realPoint.getValue().to_ulong();
				}
				else
				{
					asUInt = realPoint.getValue();
				}
				if (inByteOffset > 0 || bitLength % byteSize != 0)
				{
					uint64_t existingValue = 0;
					std::memcpy(&existingValue, dest, sizeof(uint64_t));
					asUInt <<= inByteOffset;
					uint64_t mask = (~0);
					mask ^= ((1 << (inByteOffset + bitLength)) - 1);
					mask |= ((1 << inByteOffset) - 1);
					uint64_t maskedValue = (existingValue & mask) | asUInt;
					std::memcpy(dest, &maskedValue, sizeof(uint64_t));
					if (inByteOffset + bitLength > sizeof(uint64_t) * byteSize)
					{
						uint64_t* overhangPointer = (uint64_t*)dest + 1; // NOLINT
						size_t overhangingBits
						    = inByteOffset + bitLength - sizeof(uint64_t) * byteSize;
						uint8_t eightByteOverhang = asUInt >> (sizeof(uint64_t) - overhangingBits);
						uint8_t byteMask = (~(uint8_t)0) ^ ((1 << overhangingBits) - 1);
						uint8_t byteExistingValue = 0;
						std::memcpy(&byteExistingValue, overhangPointer, sizeof(uint8_t));
						uint8_t maskedValue = (byteMask & byteExistingValue) | eightByteOverhang;
						std::memcpy(overhangPointer, &maskedValue, sizeof(uint8_t));
					}
				}
				else
				{
					T value = realPoint.getValue();
					std::memcpy(dest, &value, bitLength / byteSize);
				}
			};
		}
	};

	/*!
	 * \brief Reads a CoE Object from the bus and saves the data in the given AbstractDataPoint.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type of the CoE Object and the data point
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class CoEReadRequestHandler
	{
	public:
		using product_t = std::function<bool(
		    const datatypes::CoEObject&, datatypes::AbstractDataPoint&, size_t)>;

		static product_t eval()
		{
			return [](const datatypes::CoEObject& object, datatypes::AbstractDataPoint& point,
			           size_t bitLength) -> bool {
				using T = typename datatypes::TypeMap<E>::type;
				size_t byteLength = 0;
				if (bitLength < byteSize)
				{
					byteLength = 1;
				}
				byteLength = bitLength / byteSize;
				std::unique_ptr<uint8_t[]> buffer // NOLINT
				    = std::make_unique<uint8_t[]>(byteLength); // NOLINT
				std::memset(buffer.get(), 0, byteLength);
				int bufferSize = byteLength;
				if (ec_SDOread(object.getSlaveID(), object.getIndex(), object.getSubIndex(),
				        (boolean) false, &bufferSize, buffer.get(), EC_TIMEOUTRXM)
				    != 1)
				{
					return false;
				}
				T result;
				if constexpr (std::is_same<T, std::string>())
				{
					if (*(buffer.get() + byteLength - 1) == '\0')
					{
						result = std::string(reinterpret_cast<char*>(buffer.get())); // NOLINT
					}
					else
					{
						result.insert(result.end(), buffer.get(), buffer.get() + bufferSize);
					}
				}
				else if constexpr (std::is_same<T, std::vector<uint8_t>>())
				{
					result.insert(result.end(), buffer.get(), buffer.get() + bufferSize);
				}
				else
				{
					uint64_t resultAsUInt = 0;
					std::memcpy(&resultAsUInt, buffer.get(), bufferSize);
					result = T(resultAsUInt);
				}
				datatypes::DataPoint<T>& realPoint = dynamic_cast<datatypes::DataPoint<T>&>(point);
				realPoint = datatypes::DataPoint<T>(result, datatypes::now());
				return true;
			};
		}
	};

	/*!
	 * \brief Writes a CoE Object to a slave from the given AbstractDataPoint.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type of the CoE Object and the data point
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class CoEWriteRequestHandler
	{
	public:
		using product_t = std::function<bool(
		    const datatypes::CoEObject&, const datatypes::AbstractDataPoint&, size_t)>;

		static product_t eval()
		{
			return [](const datatypes::CoEObject& object, const datatypes::AbstractDataPoint& point,
			           size_t bitLength) -> bool {
				using T = typename datatypes::TypeMap<E>::type;
				T value = dynamic_cast<const datatypes::DataPoint<T>&>(point).getValue();
				size_t byteLength = 0;
				if (bitLength < byteSize)
				{
					byteLength = 1;
				}
				byteLength = bitLength / byteSize;
				std::unique_ptr<uint8_t[]> buffer // NOLINT
				    = std::make_unique<uint8_t[]>(byteLength); // NOLINT
				int bufferSize = byteLength;
				if constexpr (std::is_same<T, std::string>())
				{
					std::memcpy(buffer.get(), value.c_str(), value.size());
					bufferSize = value.size();
				}
				else if constexpr (std::is_same<T, std::vector<uint8_t>>())
				{
					std::memcpy(buffer.get(), value.data(), value.size());
					bufferSize = value.size();
				}
				else if constexpr (datatypes::is_bitset<T>())
				{
					uint64_t valueAsUInt = value.to_ulong();
					std::memcpy(buffer.get(), &valueAsUInt, byteLength);
				}
				else
				{
					std::memcpy(buffer.get(), &value, byteLength);
				}
				return ec_SDOwrite(object.getSlaveID(), object.getIndex(), object.getSubIndex(),
				           (boolean) false, bufferSize, buffer.get(), EC_TIMEOUTRXM)
				    == 1;
			};
		}
	};
} // namespace etherkitten::reader::bReader
