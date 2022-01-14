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
 * \brief Defines the PDOWriteRequest, which can be used to communicate requests for
 * writes to PDOs.
 */

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/datapoints.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief Represents a request to write the value of a PDO to the recipient.
	 */
	class PDOWriteRequest
	{
	public:
		/*!
		 * \brief Create a PDORequest for writing data to the specified PDO.
		 * \param pdo the PDO to write data to
		 * \param value the value to write to the PDO
		 */
		PDOWriteRequest(
		    const datatypes::PDO& pdo, std::unique_ptr<datatypes::AbstractDataPoint>&& value);

		/*!
		 * \brief Get the PDO that will be written to.
		 * \return the PDO that will be written to
		 */
		const datatypes::PDO& getPDO() const;

		/*!
		 * \brief Get the value that will be written to the PDO.
		 * \return the value that will be written to the PDO.
		 */
		const datatypes::AbstractDataPoint* getValue() const;

	private:
		const datatypes::PDO& pdo;
		const std::unique_ptr<datatypes::AbstractDataPoint> value;
	};
} // namespace etherkitten::reader
