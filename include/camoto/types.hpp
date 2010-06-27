/**
 * @file   types.hpp
 * @brief  Various types used across the libraries.
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

#ifndef _CAMOTO_TYPES_HPP_
#define _CAMOTO_TYPES_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <iostream>

namespace camoto {

/// Shared pointer to an iostream
typedef boost::shared_ptr<std::iostream> iostream_sptr;

/// Truncate function callback (to truncate an iostream)
/**
 * @see gamearchive::Archive::fnTruncate
 */
typedef boost::function<void(unsigned long)> FN_TRUNCATE;

} // namespace camoto

#endif // _CAMOTO_TYPES_HPP_
