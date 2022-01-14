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

#include "TooltipFormatter.hpp"
#include <QApplication>

namespace etherkitten::gui
{
	TooltipFormatter::TooltipFormatter()
	    : tooltip("")
	{
	}

	QString TooltipFormatter::getTooltip() const { return tooltip; }

	void TooltipFormatter::handlePDO(const datatypes::PDO& object)
	{
		(void)object;
		tooltip = "";
	}

	void TooltipFormatter::handleCoE(const datatypes::CoEObject& object)
	{
		(void)object;
		tooltip = "";
	}

	void TooltipFormatter::handleErrorStatistic(const datatypes::ErrorStatistic& statistic)
	{
		switch (statistic.getStatisticType())
		{
		case datatypes::ErrorStatisticType::TOTAL_SLAVE_FRAME_ERROR:
			tooltip = "Sum of all frame errors for this slave";
			break;
		case datatypes::ErrorStatisticType::TOTAL_SLAVE_PHYSICAL_ERROR:
			tooltip = "Sum of all physical errors for this slave";
			break;
		case datatypes::ErrorStatisticType::TOTAL_SLAVE_PREVIOUS_ERROR:
			tooltip = "Sum of all errors detected by predecessor of this slave";
			break;
		case datatypes::ErrorStatisticType::TOTAL_SLAVE_LINK_LOST_ERROR:
			tooltip = "Sum of all link lost errors for this slave";
			break;
		case datatypes::ErrorStatisticType::TOTAL_SLAVE_MALFORMAT_FRAME_ERROR:
			tooltip = "Sum of all malformat frame errors of this slave";
			break;
		case datatypes::ErrorStatisticType::TOTAL_SLAVE_LOCAL_PROBLEM_ERROR:
			tooltip = "Sum of all \"local problems\" of this slave";
			break;
		case datatypes::ErrorStatisticType::TOTAL_FRAME_ERROR:
			tooltip = "Sum of all frame errors for all slaves";
			break;
		case datatypes::ErrorStatisticType::TOTAL_PHYSICAL_ERROR:
			tooltip = "Sum of all physical errors for all slaves";
			break;
		case datatypes::ErrorStatisticType::TOTAL_PREVIOUS_ERROR:
			tooltip = "Sum of all errors detected by predecessors of all slaves";
			break;
		case datatypes::ErrorStatisticType::TOTAL_LINK_LOST_ERROR:
			tooltip = "Sum of all link lost errors for all slaves";
			break;
		case datatypes::ErrorStatisticType::TOTAL_MALFORMAT_FRAME_ERROR:
			tooltip = "Sum of all malformat frame errors of all slaves";
			break;
		case datatypes::ErrorStatisticType::TOTAL_LOCAL_PROBLEM_ERROR:
			tooltip = "Sum of all \"local problems\" of all slaves";
			break;
		case datatypes::ErrorStatisticType::FREQ_SLAVE_FRAME_ERROR:
			tooltip = "Frequency of all frame errors for this slave";
			break;
		case datatypes::ErrorStatisticType::FREQ_SLAVE_PHYSICAL_ERROR:
			tooltip = "Frequency of all physical errors for this slave";
			break;
		case datatypes::ErrorStatisticType::FREQ_SLAVE_PREVIOUS_ERROR:
			tooltip = "Frequency of all errors detected by predecessor of this slave";
			break;
		case datatypes::ErrorStatisticType::FREQ_SLAVE_LINK_LOST_ERROR:
			tooltip = "Frequency of all link lost errors for this slave";
			break;
		case datatypes::ErrorStatisticType::FREQ_SLAVE_MALFORMAT_FRAME_ERROR:
			tooltip = "Frequency of all malformat frame errors of this slave";
			break;
		case datatypes::ErrorStatisticType::FREQ_SLAVE_LOCAL_PROBLEM_ERROR:
			tooltip = "Frequency of all \"local problems\" of this slave";
			break;
		case datatypes::ErrorStatisticType::FREQ_FRAME_ERROR:
			tooltip = "Frequency of all frame errors for all slaves";
			break;
		case datatypes::ErrorStatisticType::FREQ_PHYSICAL_ERROR:
			tooltip = "Frequency of all physical errors for all slaves";
			break;
		case datatypes::ErrorStatisticType::FREQ_PREVIOUS_ERROR:
			tooltip = "Frequency of all errors detected by predecessors of all slaves";
			break;
		case datatypes::ErrorStatisticType::FREQ_LINK_LOST_ERROR:
			tooltip = "Frequency of all link lost errors for all slaves";
			break;
		case datatypes::ErrorStatisticType::FREQ_MALFORMAT_FRAME_ERROR:
			tooltip = "Frequency of all malformat frame errors of all slaves";
			break;
		case datatypes::ErrorStatisticType::FREQ_LOCAL_PROBLEM_ERROR:
			tooltip = "Frequency of all \"local problems\" of all slaves";
			break;
		default:
			tooltip = "";
		}
	}

	void TooltipFormatter::handleRegister(const datatypes::Register& reg)
	{
		switch (reg.getRegister())
		{
		case datatypes::RegisterEnum::TYPE:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Type");
			break;
		case datatypes::RegisterEnum::REVISION:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Revision");
			break;
		case datatypes::RegisterEnum::BUILD:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Build");
			break;
		case datatypes::RegisterEnum::RAM_SIZE:
			tooltip = QApplication::translate("TooltipFormatter", "Register:RAM Size");
			break;
		case datatypes::RegisterEnum::PORT0_DESCRIPTOR:
		case datatypes::RegisterEnum::PORT1_DESCRIPTOR:
		case datatypes::RegisterEnum::PORT2_DESCRIPTOR:
		case datatypes::RegisterEnum::PORT3_DESCRIPTOR:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Port Descriptor");
			break;
		case datatypes::RegisterEnum::FMMU_BIT_NOT_SUPPORTED:
			tooltip
			    = QApplication::translate("TooltipFormatter", "Register:FMMU Bit Not Supported");
			break;
		case datatypes::RegisterEnum::NO_SUPPORT_RESERVED_REGISTER:
			tooltip = QApplication::translate("TooltipFormatter", "Register:No Support Reserved");
			break;
		case datatypes::RegisterEnum::DC_SUPPORTED:
			tooltip = QApplication::translate("TooltipFormatter", "Register:DC Supported");
			break;
		case datatypes::RegisterEnum::DC_RANGE:
			tooltip = QApplication::translate("TooltipFormatter", "Register:DC Range");
			break;
		case datatypes::RegisterEnum::LOW_JITTER_EBUS:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Low Jitter EBus");
			break;
		case datatypes::RegisterEnum::ENHANCED_LINK_DETECTION_EBUS:
		case datatypes::RegisterEnum::ENHANCED_LINK_DETECTION_MII:
			tooltip
			    = QApplication::translate("TooltipFormatter", "Register:Enhanced Link Detection");
			break;
		case datatypes::RegisterEnum::SEPARATE_FCS_ERROR_HANDLING:
			tooltip = QApplication::translate(
			    "TooltipFormatter", "Register:Separate FCS Error Handling");
			break;
		case datatypes::RegisterEnum::CONFIGURED_STATION_ADDRESS:
			tooltip = QApplication::translate(
			    "TooltipFormatter", "Register:Configured Station Address");
			break;
		case datatypes::RegisterEnum::CONFIGURED_STATION_ALIAS:
			tooltip
			    = QApplication::translate("TooltipFormatter", "Register:Configured Station Alias");
			break;
		case datatypes::RegisterEnum::DLS_USER_OPERATIONAL:
			tooltip = QApplication::translate("TooltipFormatter", "Register:DLS User Operational");
			break;
		case datatypes::RegisterEnum::LINK_STATUS_PORT_0:
		case datatypes::RegisterEnum::LINK_STATUS_PORT_1:
		case datatypes::RegisterEnum::LINK_STATUS_PORT_2:
		case datatypes::RegisterEnum::LINK_STATUS_PORT_3:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Link Status");
			break;
		case datatypes::RegisterEnum::LOOP_STATUS_PORT_0:
		case datatypes::RegisterEnum::LOOP_STATUS_PORT_1:
		case datatypes::RegisterEnum::LOOP_STATUS_PORT_2:
		case datatypes::RegisterEnum::LOOP_STATUS_PORT_3:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Loop Status");
			break;
		case datatypes::RegisterEnum::SIGNAL_DETECTION_PORT_0:
		case datatypes::RegisterEnum::SIGNAL_DETECTION_PORT_1:
		case datatypes::RegisterEnum::SIGNAL_DETECTION_PORT_2:
		case datatypes::RegisterEnum::SIGNAL_DETECTION_PORT_3:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Signal Detection");
			break;
		case datatypes::RegisterEnum::STATUS_CONTROL:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Status Control");
			break;
		case datatypes::RegisterEnum::STATUS:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Status");
			break;
		case datatypes::RegisterEnum::DLS_USER_R2:
		case datatypes::RegisterEnum::DLS_USER_R4:
		case datatypes::RegisterEnum::DLS_USER_R5:
		case datatypes::RegisterEnum::DLS_USER_R6:
		case datatypes::RegisterEnum::DLS_USER_R7:
		case datatypes::RegisterEnum::DLS_USER_R8:
		case datatypes::RegisterEnum::DLS_USER_R9:
			tooltip = QApplication::translate("TooltipFormatter", "Register:DLS User");
			break;
		case datatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_0:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Frame Error 0");
			break;
		case datatypes::RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_0:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Physical Error 0");
			break;
		case datatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Frame Error 1");
			break;
		case datatypes::RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_1:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Physical Error 1");
			break;
		case datatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_2:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Frame Error 2");
			break;
		case datatypes::RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_2:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Physical Error 2");
			break;
		case datatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_3:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Frame Error 3");
			break;
		case datatypes::RegisterEnum::PHYSICAL_ERROR_COUNTER_PORT_3:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Physical Error 3");
			break;
		case datatypes::RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_0:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Previous Error 0");
			break;
		case datatypes::RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_1:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Previous Error 1");
			break;
		case datatypes::RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_2:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Previous Error 2");
			break;
		case datatypes::RegisterEnum::PREVIOUS_ERROR_COUNTER_PORT_3:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Previous Error 3");
			break;
		case datatypes::RegisterEnum::MALFORMAT_FRAME_COUNTER:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Malformat Frame");
			break;
		case datatypes::RegisterEnum::LOCAL_PROBLEM_COUNTER:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Local Problem");
			break;
		case datatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_0:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Lost Link 0");
			break;
		case datatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_1:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Lost Link 1");
			break;
		case datatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_2:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Lost Link 2");
			break;
		case datatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_3:
			tooltip = QApplication::translate("TooltipFormatter", "Register:Lost Link 3");
			break;
		case datatypes::RegisterEnum::SII_READ_OPERATION:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Read Operation");
			break;
		case datatypes::RegisterEnum::SII_WRITE_OPERATION:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Write Operation");
			break;
		case datatypes::RegisterEnum::SII_RELOAD_OPERATION:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Reload Operation");
			break;
		case datatypes::RegisterEnum::SII_CHECKSUM_ERROR:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Checksum Error");
			break;
		case datatypes::RegisterEnum::SII_DEVICE_INFO_ERROR:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Device Info Error");
			break;
		case datatypes::RegisterEnum::SII_COMMAND_ERROR:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Command Error");
			break;
		case datatypes::RegisterEnum::SII_WRITE_ERROR:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Write Error");
			break;
		case datatypes::RegisterEnum::SII_BUSY:
			tooltip = QApplication::translate("TooltipFormatter", "Register:SII Busy");
			break;
		case datatypes::RegisterEnum::SYSTEM_TIME:
			tooltip = QApplication::translate("TooltipFormatter", "Register:System Time");
			break;
		default:
			tooltip = "";
		}
	}

} // namespace etherkitten::gui
