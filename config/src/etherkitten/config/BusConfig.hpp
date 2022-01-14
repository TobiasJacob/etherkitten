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

#include <inttypes.h>
#include <unordered_set>
#include <vector>

namespace etherkitten::config
{
	/*!
	 * \brief Stores all configuration that is needed for a bus.
	 *
	 * This class can be stored in a config file by ConfigIO.
	 */
	class BusConfig
	{
	public:
		/*!
		 * \brief Create a new BusConfig with all registers invisible.
		 */
		BusConfig();

		/*!
		 * \brief Create a new BusConfig as a copy of another one.
		 *
		 * \param busConfig The BusConfig to copy.
		 */
		BusConfig(BusConfig& busConfig);

		/*!
		 * \brief Create a new BusConfig as a copy of another one.
		 * This is the const version.
		 * \param busConfig The BusConfig to copy.
		 */
		BusConfig(const BusConfig& busConfig);

		/*!
		 * \brief Set a register a visible or invisible
		 *
		 * \param registerAddr The register that is changed.
		 * \param visible If true, the register will be set visible.
		 */
		void setRegisterVisible(uint32_t registerAddr, bool visible);

		/*!
		 * \brief Return whether the register is visible.
		 *
		 * \param registerAddr The register the caller is interested in.
		 * \return true, if the register is visible, false otherwise.
		 */
		bool isRegisterVisible(uint32_t registerAddr) const;

		/*!
		 * \brief Return a list of all visible registers.
		 *
		 * \return The list of visible registers.
		 */
		std::vector<uint32_t> getVisibleRegisters() const;

	private:
		std::unordered_set<uint32_t> visibleRegisters;
	};
} // namespace etherkitten::config
