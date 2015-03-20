/**
 * @file  stream_archfile.hpp
 * @brief Provide a stream that accesses a file within an Archive instance.
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

#ifndef _CAMOTO_STREAM_ARCHFILE_HPP_
#define _CAMOTO_STREAM_ARCHFILE_HPP_

#include <camoto/gamearchive/manager.hpp>
#include <camoto/stream_sub.hpp>
#include <camoto/gamearchive/archive-fat.hpp>

namespace camoto {
namespace gamearchive {

/// Substream parts in common with read and write
class archfile_core: virtual public stream::sub_core
{
	protected:
		archfile_core(const Archive::FileHandle& id);

	public:
		/// Move the substream's start point within the parent stream.
		/**
		 * @param off
		 *   Distance to move the stream.  Negative values move closer to
		 *   the start of the parent stream.
		 */
		void relocate(stream::delta off);

		/// Alter the size of the substream without affecting any data.
		/**
		 * This function should only be called by the creator of the stream if the
		 * parent stream has been modified outside of the substream.
		 *
		 * Normally output_sub::truncate() would be used to correctly resize the
		 * substream.  If in doubt, don't use this function!
		 *
		 * @param len
		 *   New size of the substream.
		 */
		void resize(stream::len len);

		/// Get the current offset into the parent stream.
		/**
		 * @return Current offset, relative to start of parent stream, where first
		 *   byte in the substream sits.
		 */
		virtual stream::pos sub_start() const;

		/// Get the current size of the window into the parent stream.
		/**
		 * @return Current size of substream, in bytes.  The last byte in the
		 *   parent stream that can be read is at offset start() + size() - 1.
		 */
		virtual stream::len sub_size() const;

		/// File handle for resizing/truncating.
		Archive::FileHandle id;

		/// FATEntry cast of id
		/**
		 * This avoids doing a dynamic_cast on each and every read/write.
		 */
		const Archive_FAT::FATEntry *fat;
};

/// Read-only stream to access a section within another stream.
class input_archfile:
	virtual public stream::input_sub,
	virtual public archfile_core
{
	public:
		/// Substream representing a file within an Archive.
		/**
		 * @param id
		 *   FileHandle of file being opened.  Used to get current offset/size
		 *   values for substream window.
		 *
		 * @param content
		 *   Stream containing archive's raw content.  No filters should be applied,
		 *   and this stream can be shared amongst other files, although at the time
		 *   of writing this is not thread-safe.
		 */
		input_archfile(const Archive::FileHandle& id,
			std::shared_ptr<stream::input> content);

		using stream::input_sub::size;
};

/// Write-only stream to access a section within another stream.
class output_archfile:
	virtual public stream::output_sub,
	virtual public archfile_core
{
	public:
		/// Substream representing a file within an Archive.
		/**
		 * @param archive
		 *   Archive containing the file to open.  Used to call resize() method
		 *   if stream is ever resized.
		 *
		 * @param id
		 *   FileHandle of file being opened.  Passed to Archive::resize() if
		 *   stream is resized.  Also kept and used to get current offset/size
		 *   values for substream window.
		 *
		 * @param content
		 *   Stream containing archive's raw content.  No filters should be applied,
		 *   and this stream can be shared amongst other files, although at the time
		 *   of writing this is not thread-safe.
		 */
		output_archfile(std::shared_ptr<Archive> archive, Archive::FileHandle id,
			std::shared_ptr<stream::output> content);

		virtual void truncate(stream::len size);
		virtual void flush();

		/// Set the original (decompressed) size of this stream.
		/**
		 * This is just a convenience function to call Archive::resize().
		 */
		void setRealSize(stream::len newRealSize);

	protected:
		/// Archive handle for resizing/truncating.
		std::shared_ptr<Archive> archive;
};

/// Read/write stream accessing a file within an Archive.
class archfile:
	virtual public stream::sub,
	virtual public input_archfile,
	virtual public output_archfile
{
	public:
		/// Substream representing a file within an Archive.
		/**
		 * @copydetails output_archfile::output_archfile()
		 */
		archfile(std::shared_ptr<Archive> archive, Archive::FileHandle id,
			std::shared_ptr<stream::inout> content);
};

std::unique_ptr<stream::inout> applyFilter(std::unique_ptr<archfile> s,
	const std::string& filter);

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_STREAM_ARCHFILE_HPP_
