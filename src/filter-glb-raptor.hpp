/**
 * @file  filter-glb-raptor.hpp
 * @brief Filter that encrypts and decrypts data in Raptor GLB archives.
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

#ifndef _CAMOTO_FILTER_GLB_RAPTOR_HPP_
#define _CAMOTO_FILTER_GLB_RAPTOR_HPP_

#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

/// Raptor .GLB decryption algorithm.
class filter_glb_decrypt: virtual public filter
{
	protected:
		int lenBlock;       ///< Length of each encryption block, 0 for unlimited
		std::string key;    ///< Encryption key
		int lenKey;         ///< strlen(key) for efficiency
		int posKey;         ///< Current index into key
		stream::len offset; ///< Current offset (number of bytes processed)
		uint8_t lastByte;   ///< Previous byte read

	public:
		/// Create a new encryption filter with the given options.
		/**
		 * @param key
		 *   Encryption key.
		 *
		 * @param lenBlock
		 *   Number of bytes to crypt before resetting key to the initial state.
		 *   0 means no reset mid-sequence.
		 */
		filter_glb_decrypt(const std::string& key, int lenBlock);
		virtual ~filter_glb_decrypt();

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);
};

/// Raptor .GLB encryption algorithm.
class filter_glb_encrypt: virtual public filter
{
	protected:
		int lenBlock;       ///< Length of each encryption block, 0 for unlimited
		std::string key;    ///< Encryption key
		int lenKey;         ///< strlen(key) for efficiency
		int posKey;         ///< Current index into key
		stream::len offset; ///< Current offset (number of bytes processed)
		uint8_t lastByte;   ///< Previous byte read

	public:
		/// Create a new encryption filter with the given options.
		/**
		 * @param key
		 *   Encryption key.
		 *
		 * @param lenBlock
		 *   Number of bytes to crypt before resetting key to the initial state.
		 *   0 means no reset mid-sequence.
		 */
		filter_glb_encrypt(const std::string& key, int lenBlock);
		virtual ~filter_glb_encrypt();

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);
};

/// Decrypt .GLB FAT using Raptor's GLB cipher.
class FilterType_GLB_Raptor_FAT: virtual public FilterType
{
	public:
		FilterType_GLB_Raptor_FAT();
		~FilterType_GLB_Raptor_FAT();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> games() const;
		virtual std::unique_ptr<stream::inout> apply(std::shared_ptr<stream::inout> target,
			stream::fn_truncate_filter resize) const;
		virtual std::unique_ptr<stream::input> apply(std::shared_ptr<stream::input> target) const;
		virtual std::unique_ptr<stream::output> apply(std::shared_ptr<stream::output> target,
			stream::fn_truncate_filter resize) const;
};

/// Decrypt a file inside a .GLB archive.
class FilterType_GLB_Raptor_File: virtual public FilterType
{
	public:
		FilterType_GLB_Raptor_File();
		~FilterType_GLB_Raptor_File();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> games() const;
		virtual std::unique_ptr<stream::inout> apply(std::shared_ptr<stream::inout> target,
			stream::fn_truncate_filter resize) const;
		virtual std::unique_ptr<stream::input> apply(std::shared_ptr<stream::input> target) const;
		virtual std::unique_ptr<stream::output> apply(std::shared_ptr<stream::output> target,
			stream::fn_truncate_filter resize) const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_GLB_RAPTOR_HPP_
