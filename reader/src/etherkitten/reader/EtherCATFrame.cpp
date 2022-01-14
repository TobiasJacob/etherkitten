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


#include "EtherCATFrame.hpp"

namespace etherkitten::reader
{
	EtherCATFrameIterator::EtherCATFrameIterator(
	    EtherCATFrameList* list, size_t startIndex, size_t count)
	    : list(list)
	    , nextIndex(startIndex)
	    , remaining(count)
	{
	}

	std::pair<EtherCATFrame*, EtherCATFrameMetaData*> EtherCATFrameIterator::operator*() const
	{
		return { &list->list[nextIndex].first, &list->list[nextIndex].second };
	}

	bool EtherCATFrameIterator::hasCompletedLoop() const { return nextIndex == 0; }

	EtherCATFrameIterator& EtherCATFrameIterator::operator++()
	{
		if (remaining > 0)
		{
			nextIndex = (nextIndex + 1) % list->list.size();
			--remaining;
		}
		return *this;
	}

	bool EtherCATFrameIterator::atEnd() const { return remaining == 0; }
} // namespace etherkitten::reader
