#include <iostream>
#include <camoto/stream_file.hpp>
#include <camoto/util.hpp>
#include <camoto/gamearchive.hpp>

using namespace camoto;
using namespace camoto::gamearchive;

int main(void)
{
	// Use the manager to look up a particular archive format
	auto archiveType = ArchiveManager::byCode("grp-duke3d");

	// Open an archive file on disk
	auto file = std::make_unique<stream::file>();
	file->open("duke3d.grp");

	// We cheat here - we should check and load any supplementary files, but
	// for the sake of keeping this example simple we know this format doesn't
	// need any supps.
	camoto::SuppData supps;

	// Use the archive format handler to read in the file we opened as an archive
	auto arch = archiveType->open(std::move(file), supps);

	// Get a list of all the files in the archive
	auto& contents = arch->files();

	// Print the size of the list (the number of files in the archive)
	std::cout << "Found " << contents.size() << " files.\n";

	// Run through the list of files and show the filename
	for (auto& file : contents) {
		std::cout << file->strName << "\n";
	}
	std::cout << "Done." << std::endl;

	// No cleanup required because all the Ptr variables are shared pointers,
	// which get destroyed automatically when they go out of scope (and nobody
	// else is using them!)

	return 0;
}
