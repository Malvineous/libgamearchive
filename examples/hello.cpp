#include <iostream>
#include <camoto/stream_file.hpp>
#include <camoto/gamearchive.hpp>

using namespace camoto;
using namespace camoto::gamearchive;

int main(void)
{
	// Get hold of the Manager class
	ManagerPtr manager = getManager();

	// Use the manager to look up a particular archive format
	ArchiveTypePtr archiveType = manager->getArchiveTypeByCode("grp-duke3d");

	// Open an archive file on disk
	stream::file_sptr file(new stream::file());
	file->open("duke3d.grp");

	// We cheat here - we should check and load any supplementary files, but
	// for the sake of keeping this example simple we know this format doesn't
	// need any supps.
	camoto::SuppData supps;

	// Use the archive format handler to read in the file we opened as an archive
	ArchivePtr arch = archiveType->open(file, supps);

	// Get a list of all the files in the archive
	const Archive::VC_ENTRYPTR& contents = arch->getFileList();

	// Print the size of the list (the number of files in the archive)
	std::cout << "Found " << contents.size() << " files.\n";

	// Run through the list of files and show the filename
	for (Archive::VC_ENTRYPTR::const_iterator i = contents.begin(); i != contents.end(); i++) {
		const Archive::EntryPtr subfile = *i;
		std::cout << subfile->strName << "\n";
	}
	std::cout << "Done." << std::endl;

	// No cleanup required because all the Ptr variables are shared pointers,
	// which get destroyed automatically when they go out of scope (and nobody
	// else is using them!)

	return 0;
}
