/**
 * @file   filter-epfs.hpp
 * @brief  FilterType for the East Point EPFS compression algorithm.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CAMOTO_FILTER_EPFS_HPP_
#define _CAMOTO_FILTER_EPFS_HPP_

#include <camoto/types.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

/// EPFS decompression filter.
class EPFSFilterType: virtual public FilterType {

	public:
		EPFSFilterType()
			throw ();

		~EPFSFilterType()
			throw ();

		virtual std::string getFilterCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual iostream_sptr apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
			throw (ECorruptedData);

		virtual istream_sptr apply(istream_sptr target)
			throw (ECorruptedData);

		virtual ostream_sptr apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
			throw (ECorruptedData);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_EPFS_HPP_
