/**
 * @file  camoto/gamearchive/filtertype.hpp
 * @brief Declaration of top-level FilterType class, for performing certain
 *        processing operations on data streams (compression/decompression,
 *        encryption, etc.)
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_GAMEARCHIVE_FILTERTYPE_HPP_
#define _CAMOTO_GAMEARCHIVE_FILTERTYPE_HPP_

#include <memory>
#include <vector>
#include <camoto/filter.hpp>
#include <camoto/stream.hpp>
#include <camoto/stream_filtered.hpp>

namespace camoto {
namespace gamearchive {

/// Primary interface to a filter.
/**
 * This class represents a filter.  Its functions are used to manipulate C++
 * iostreams so that the data passing through the stream is altered in some
 * way, such as by being compressed or decompressed.
 */
class FilterType
{
	public:
		/// Get a short code to identify this filter, e.g. "cmp-zone66"
		/**
		 * This can be useful for command-line arguments.
		 *
		 * @return The filter's short name/ID.
		 */
		virtual std::string code() const = 0;

		/// Get the filter name, e.g. "Zone 66 compression"
		/**
		 * @return The filter name.
		 */
		virtual std::string friendlyName() const = 0;

		/// Get a list of games using this format.
		/**
		 * @return A vector of game names, such as "Zone 66".
		 */
		virtual std::vector<std::string> games() const = 0;

		/// Apply the algorithm to an iostream.
		/**
		 * This function takes in a target iostream and applies the algorithm to
		 * it.  The target stream can be empty.
		 *
		 * Any data written to the returned stream will have the algorithm applied
		 * (e.g. data is compressed) and then written to the target stream.
		 *
		 * Any data read from the returned stream will read data from the target
		 * stream and apply the algorithm in reverse (e.g. data is decompressed.)
		 *
		 * @param target
		 *   Target stream where the filtered data exists or is to end up.
		 *
		 * @param resize
		 *   Notification function called when the stream is resized.  This function
		 *   doesn't have to do anything (and can be NULL) but it is used in cases
		 *   where a game archive stores both a file's compressed and decompressed
		 *   size.  Here the callback will be notified of the decompressed size
		 *   during the flush() call.  There is no notification for the compressed
		 *   size, as this is known from the amount of data that was written to
		 *   the stream.
		 *
		 * @return Clear/plaintext stream providing data from target after
		 *   processing.
		 */
		virtual std::unique_ptr<stream::inout> apply(std::shared_ptr<stream::inout> target,
			stream::fn_truncate_filter resize) const = 0;

		/// Apply the algorithm to an input stream.
		/**
		 * @sa apply(std::shared_ptr<stream::inout>)
		 */
		virtual std::unique_ptr<stream::input> apply(std::shared_ptr<stream::input> target) const = 0;

		/// Apply the algorithm to an output stream.
		/**
		 * @sa apply(std::shared_ptr<stream::inout>)
		 */
		virtual std::unique_ptr<stream::output> apply(std::shared_ptr<stream::output> target,
			stream::fn_truncate_filter resize) const = 0;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_FILTERTYPE_HPP_
