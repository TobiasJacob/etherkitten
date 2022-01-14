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
 * \brief Defines helper types and methods for implementing BusSlaveInformant functionality.
 */

/*!
 * \namespace etherkitten::reader::bSInformant
 * \brief Defines helper methods for the implementation of the BusSlaveInformant.
 */

#include <algorithm>
#include <functional>
#include <vector>

extern "C"
{
#include <ethercat.h>
}

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/errors.hpp>

namespace etherkitten::reader::bSInformant
{
	using datatypes::ErrorMessage;
	using datatypes::ErrorSeverity;

	using PDOList = std::vector<datatypes::PDO>;
	using PDOResult = std::pair<PDOList,
	    std::unordered_map<datatypes::PDO, datatypes::PDOInfo, datatypes::PDOHash,
	        datatypes::PDOEqual>>;

	using CoEList = std::vector<datatypes::CoEEntry>;
	using CoEResult = std::pair<CoEList,
	    std::unordered_map<datatypes::CoEObject, datatypes::CoEInfo, datatypes::CoEObjectHash,
	        datatypes::CoEObjectEqual>>;

	/*!
	 * \brief Typifies the possibility that a function might return a value and errors from the same
	 * call.
	 * \tparam T the value type that is stored
	 */
	template<typename T>
	using MayError = std::pair<T, std::vector<ErrorMessage>>;

	/*!
	 * \brief Check whether the given list of errors contains only LOW severity errors.
	 * \param errors the list of errors to check
	 * \retval true iff only LOW severity errors are contained in the list
	 * \retval false iff any higher severity errors are contained in the list
	 */
	bool containsOnlyLowSeverityErrors(const std::vector<ErrorMessage>& errors);

	/*!
	 * \brief Check whether the given list of errors contains any FATAL severity errors.
	 * \param errors the list of errors to check
	 * \retval true iff any FATAL severity errors are contained in the list
	 * \retval false iff no FATAL severity errors are contained in the list
	 */
	bool containsFatalSeverityError(const std::vector<ErrorMessage>& errors);

	/*!
	 * \brief Add all the errors SOEM adds to a global data structure to a list
	 * \param errors the error list to add the SOEM errors to
	 * \param severity the severity to assign to the added SOEM errors
	 */
	void addSOEMErrorsToVector(std::vector<ErrorMessage>& errors, ErrorSeverity severity);

	/*!
	 * \brief Add all the errors SOEM adds to a global data structure to a list
	 * \param errors the error list to add the SOEM errors to
	 * \param slave the slave to assign to the added SOEM errors
	 * \param severity the severity to assign to the added SOEM errors
	 */
	void addSOEMErrorsToVector(
	    std::vector<ErrorMessage>& errors, unsigned int slave, ErrorSeverity severity);

} // namespace etherkitten::reader::bSInformant
