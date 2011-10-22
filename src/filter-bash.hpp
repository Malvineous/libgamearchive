/**
 * @file   filter-bash.hpp
 * @brief  Filter implementation for decompressing Monster Bash files.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_FILTER_BASH_HPP_
#define _CAMOTO_FILTER_BASH_HPP_

#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

/// Monster Bash decompression filter.
class BashFilterType: virtual public FilterType {

	public:
		BashFilterType()
			throw ();

		~BashFilterType()
			throw ();

		virtual std::string getFilterCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual stream::inout_sptr apply(stream::inout_sptr target)
			throw (filter_error, stream::read_error);

		virtual stream::input_sptr apply(stream::input_sptr target)
			throw (filter_error, stream::read_error);

		virtual stream::output_sptr apply(stream::output_sptr target)
			throw (filter_error);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_BASH_HPP_
