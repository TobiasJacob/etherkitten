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
 * \brief Defines data objects that represent data sources on an EtherCAT bus.
 */

#include "DataObject.hpp"

namespace etherkitten::datatypes
{
	/*!
	 * \brief The RegisterEnum enum encodes the slave registers that every slave offers.
	 *
	 * Not all registers are in this enum; a subset has been selected based on the registers' value
	 * in debugging.
	 *
	 * The low 16 bits of a register's value in this enum corresponds to its address in a slave's
	 * memory. If multiple registers are located in the same byte, the next 3 bits encode the
	 * register's offset in the byte.
	 */
	enum class RegisterEnum
	{
		TYPE = 0x0,
		REVISION = 0x1,
		BUILD = 0x2,
		RAM_SIZE = 0x6,
		PORT0_DESCRIPTOR = 0x00007,
		PORT1_DESCRIPTOR = 0x20007,
		PORT2_DESCRIPTOR = 0x40007,
		PORT3_DESCRIPTOR = 0x60007,
		FMMU_BIT_NOT_SUPPORTED = 0x00008,
		NO_SUPPORT_RESERVED_REGISTER = 0x10008,
		DC_SUPPORTED = 0x20008,
		DC_RANGE = 0x30008,
		LOW_JITTER_EBUS = 0x40008,
		ENHANCED_LINK_DETECTION_EBUS = 0x50008,
		ENHANCED_LINK_DETECTION_MII = 0x60008,
		SEPARATE_FCS_ERROR_HANDLING = 0x70008,
		CONFIGURED_STATION_ADDRESS = 0x10,
		CONFIGURED_STATION_ALIAS = 0x12,
		DLS_USER_OPERATIONAL = 0x00110,
		LINK_STATUS_PORT_0 = 0x40110,
		LINK_STATUS_PORT_1 = 0x50110,
		LINK_STATUS_PORT_2 = 0x60110,
		LINK_STATUS_PORT_3 = 0x70110,
		LOOP_STATUS_PORT_0 = 0x00111,
		SIGNAL_DETECTION_PORT_0 = 0x10111,
		LOOP_STATUS_PORT_1 = 0x20111,
		SIGNAL_DETECTION_PORT_1 = 0x30111,
		LOOP_STATUS_PORT_2 = 0x40111,
		SIGNAL_DETECTION_PORT_2 = 0x50111,
		LOOP_STATUS_PORT_3 = 0x60111,
		SIGNAL_DETECTION_PORT_3 = 0x70111,
		STATUS_CONTROL = 0x120,
		DLS_USER_R2 = 0x121,
		STATUS = 0x130,
		DLS_USER_R4 = 0x131,
		DLS_USER_R5 = 0x132,
		DLS_USER_R6 = 0x134,
		DLS_USER_R7 = 0x140,
		DLS_USER_R8 = 0x150,
		DLS_USER_R9 = 0x141,
		FRAME_ERROR_COUNTER_PORT_0 = 0x300,
		PHYSICAL_ERROR_COUNTER_PORT_0 = 0x301,
		FRAME_ERROR_COUNTER_PORT_1 = 0x302,
		PHYSICAL_ERROR_COUNTER_PORT_1 = 0x303,
		FRAME_ERROR_COUNTER_PORT_2 = 0x304,
		PHYSICAL_ERROR_COUNTER_PORT_2 = 0x305,
		FRAME_ERROR_COUNTER_PORT_3 = 0x306,
		PHYSICAL_ERROR_COUNTER_PORT_3 = 0x307,
		PREVIOUS_ERROR_COUNTER_PORT_0 = 0x308,
		PREVIOUS_ERROR_COUNTER_PORT_1 = 0x309,
		PREVIOUS_ERROR_COUNTER_PORT_2 = 0x30A,
		PREVIOUS_ERROR_COUNTER_PORT_3 = 0x30B,
		MALFORMAT_FRAME_COUNTER = 0x30C,
		LOCAL_PROBLEM_COUNTER = 0x30D,
		LOST_LINK_COUNTER_PORT_0 = 0x310,
		LOST_LINK_COUNTER_PORT_1 = 0x311,
		LOST_LINK_COUNTER_PORT_2 = 0x312,
		LOST_LINK_COUNTER_PORT_3 = 0x313,
		SII_READ_OPERATION = 0x00503,
		SII_WRITE_OPERATION = 0x10503,
		SII_RELOAD_OPERATION = 0x20503,
		SII_CHECKSUM_ERROR = 0x30503,
		SII_DEVICE_INFO_ERROR = 0x40503,
		SII_COMMAND_ERROR = 0x50503,
		SII_WRITE_ERROR = 0x60503,
		SII_BUSY = 0x70503,
		SYSTEM_TIME = 0x910,
	};

	/*!
	 * \brief The RegisterInfo struct holds additional information on one register.
	 */
	struct RegisterInfo
	{
		std::string name;
		EtherCATDataTypeEnum enumType;
		int bitLength;
	};

	template<EtherCATDataTypeEnum E>
	RegisterInfo makeInfo(const std::string& name)
	{
		return { name, E, EtherCATDataType::bitLength<typename TypeMap<E>::type> };
	}

	/*!
	 * \brief Holds additional information on all defined registers.
	 */
	const std::unordered_map<RegisterEnum, RegisterInfo> registerMap = {
		{ RegisterEnum::TYPE, makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Type") },
		{ RegisterEnum::REVISION, makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Revision") },
		{ RegisterEnum::BUILD, makeInfo<EtherCATDataTypeEnum::UNSIGNED16>("Build") },
		{ RegisterEnum::RAM_SIZE, makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Ram size") },
		{ RegisterEnum::PORT0_DESCRIPTOR,
		    makeInfo<EtherCATDataTypeEnum::BIT2>("Port 0 descriptor") },
		{ RegisterEnum::PORT1_DESCRIPTOR,
		    makeInfo<EtherCATDataTypeEnum::BIT2>("Port 1 descriptor") },
		{ RegisterEnum::PORT2_DESCRIPTOR,
		    makeInfo<EtherCATDataTypeEnum::BIT2>("Port 2 descriptor") },
		{ RegisterEnum::PORT3_DESCRIPTOR,
		    makeInfo<EtherCATDataTypeEnum::BIT2>("Port 3 descriptor") },
		{ RegisterEnum::FMMU_BIT_NOT_SUPPORTED,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("FMMU bit operation not supported") },
		{ RegisterEnum::NO_SUPPORT_RESERVED_REGISTER,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("No support for reserved register") },
		{ RegisterEnum::DC_SUPPORTED, makeInfo<EtherCATDataTypeEnum::BIT1>("DC supported") },
		{ RegisterEnum::DC_RANGE, makeInfo<EtherCATDataTypeEnum::BIT1>("DC range") },
		{ RegisterEnum::LOW_JITTER_EBUS, makeInfo<EtherCATDataTypeEnum::BIT1>("Low jitter EBUS") },
		{ RegisterEnum::ENHANCED_LINK_DETECTION_EBUS,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Enhanced link detection EBUS") },
		{ RegisterEnum::ENHANCED_LINK_DETECTION_MII,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Enhanced link detection MII") },
		{ RegisterEnum::SEPARATE_FCS_ERROR_HANDLING,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Separate fcs error handling") },
		{ RegisterEnum::CONFIGURED_STATION_ADDRESS,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED16>("Configured station address") },
		{ RegisterEnum::CONFIGURED_STATION_ALIAS,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED16>("Configured station alias") },
		{ RegisterEnum::DLS_USER_OPERATIONAL,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("DLS user operational") },
		{ RegisterEnum::LINK_STATUS_PORT_0,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Link status port 0") },
		{ RegisterEnum::LINK_STATUS_PORT_1,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Link status port 1") },
		{ RegisterEnum::LINK_STATUS_PORT_2,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Link status port 2") },
		{ RegisterEnum::LINK_STATUS_PORT_3,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Link status port 3") },
		{ RegisterEnum::LOOP_STATUS_PORT_0,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Loop status port 0") },
		{ RegisterEnum::SIGNAL_DETECTION_PORT_0,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Signal detection port 0") },
		{ RegisterEnum::LOOP_STATUS_PORT_1,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Loop status port 1") },
		{ RegisterEnum::SIGNAL_DETECTION_PORT_1,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Signal detection port 1") },
		{ RegisterEnum::LOOP_STATUS_PORT_2,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Loop status port 2") },
		{ RegisterEnum::SIGNAL_DETECTION_PORT_2,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Signal detection port 2") },
		{ RegisterEnum::LOOP_STATUS_PORT_3,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Loop status port 3") },
		{ RegisterEnum::SIGNAL_DETECTION_PORT_3,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("Signal detection port 3") },
		{ RegisterEnum::STATUS_CONTROL,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Status control") },
		{ RegisterEnum::DLS_USER_R2, makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("DLS user R2") },
		{ RegisterEnum::STATUS, makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Status") },
		{ RegisterEnum::DLS_USER_R4, makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("DLS user R4") },
		{ RegisterEnum::DLS_USER_R5, makeInfo<EtherCATDataTypeEnum::UNSIGNED16>("DLS user R5") },
		{ RegisterEnum::DLS_USER_R6, makeInfo<EtherCATDataTypeEnum::UNSIGNED16>("DLS user R6") },
		{ RegisterEnum::DLS_USER_R7, makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("DLS user R7") },
		{ RegisterEnum::DLS_USER_R8, makeInfo<EtherCATDataTypeEnum::UNSIGNED32>("DLS user R8") },
		{ RegisterEnum::DLS_USER_R9, makeInfo<EtherCATDataTypeEnum::BIT7>("DLS user R9") },
		{ RegisterEnum::FRAME_ERROR_COUNTER_PORT_0,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Frame error counter port 0") },
		{ RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_0,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Physical error counter port 0") },
		{ RegisterEnum::FRAME_ERROR_COUNTER_PORT_1,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Frame error counter port 1") },
		{ RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_1,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Physical error counter port 1") },
		{ RegisterEnum::FRAME_ERROR_COUNTER_PORT_2,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Frame error counter port 2") },
		{ RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_2,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Physical error counter port 2") },
		{ RegisterEnum::FRAME_ERROR_COUNTER_PORT_3,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Frame error counter port 3") },
		{ RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_3,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Physical error counter port 3") },
		{ RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_0,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Previous error counter port 0") },
		{ RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_1,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Previous error counter port 1") },
		{ RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_2,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Previous error counter port 2") },
		{ RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_3,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Previous error counter port 3") },
		{ RegisterEnum::MALFORMAT_FRAME_COUNTER,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Malformat frame counter") },
		{ RegisterEnum::LOCAL_PROBLEM_COUNTER,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Local problem counter") },
		{ RegisterEnum::LOST_LINK_COUNTER_PORT_0,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Lost link counter port 0") },
		{ RegisterEnum::LOST_LINK_COUNTER_PORT_1,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Lost link counter port 1") },
		{ RegisterEnum::LOST_LINK_COUNTER_PORT_2,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Lost link counter port 2") },
		{ RegisterEnum::LOST_LINK_COUNTER_PORT_3,
		    makeInfo<EtherCATDataTypeEnum::UNSIGNED8>("Lost link counter port 3") },
		{ RegisterEnum::SII_READ_OPERATION,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("SII read operation") },
		{ RegisterEnum::SII_WRITE_OPERATION,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("SII write operation") },
		{ RegisterEnum::SII_RELOAD_OPERATION,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("SII reload operation") },
		{ RegisterEnum::SII_CHECKSUM_ERROR,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("SII checksum error") },
		{ RegisterEnum::SII_DEVICE_INFO_ERROR,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("SII device info error") },
		{ RegisterEnum::SII_COMMAND_ERROR,
		    makeInfo<EtherCATDataTypeEnum::BIT1>("SII command error") },
		{ RegisterEnum::SII_WRITE_ERROR, makeInfo<EtherCATDataTypeEnum::BIT1>("SII write error") },
		{ RegisterEnum::SII_BUSY, makeInfo<EtherCATDataTypeEnum::BIT1>("SII busy") },
		{ RegisterEnum::SYSTEM_TIME, makeInfo<EtherCATDataTypeEnum::UNSIGNED64>("System time") },
	};

	/*!
	 * \brief Get the length of a register in bytes, rounded up.
	 * \param reg the register to get the length of
	 * \return the length of the register
	 */
	size_t getRegisterByteLength(datatypes::RegisterEnum reg);

	/*!
	 * \brief The Register class uniquely identifies one register of a slave on an EtherCAT bus.
	 */
	class Register : public DataObject
	{
	public:
		/*!
		 * \brief Construct a new Register with the given parameters.
		 * \param slaveID the ID of the slave this register belongs to
		 * \param registerType the register of the slave this Register identifies
		 */
		Register(unsigned int slaveID, RegisterEnum registerType);

		void acceptVisitor(DataObjectVisitor& visitor) const override;

		/*!
		 * \brief Get which register on the slave is identified by this Register.
		 * \return the register identified by this Register
		 */
		RegisterEnum getRegister() const;

	private:
		RegisterEnum type;
	};
} // namespace etherkitten::datatypes
