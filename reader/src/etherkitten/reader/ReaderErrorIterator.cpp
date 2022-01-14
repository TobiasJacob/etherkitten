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

#include "ReaderErrorIterator.hpp"

#include <etherkitten/datatypes/datapoints.hpp>

namespace etherkitten::reader
{
	ReaderErrorIterator::ReaderErrorIterator(
	    std::shared_ptr<DataView<datatypes::ErrorMessage>>&& dataView)
	    : dataView(std::move(dataView))
	{
	}

	bool ReaderErrorIterator::hasNext() const { return dataView->hasNext(); }

	bool ReaderErrorIterator::isEmpty() const { return dataView->isEmpty(); }

	datatypes::DataPoint<datatypes::ErrorMessage> ReaderErrorIterator::operator*() const
	{
		return datatypes::DataPoint<datatypes::ErrorMessage>(
		    dataView->operator*(), dataView->getTime());
	}

	ReaderErrorIterator& ReaderErrorIterator::operator++()
	{
		dataView->operator++();
		return *this;
	}
} // namespace etherkitten::reader
