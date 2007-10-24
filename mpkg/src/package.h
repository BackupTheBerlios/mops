// $Id: package.h,v 1.1 2007/10/24 22:00:12 i27249 Exp $

#ifndef PACKAGE_H_
#define PACKAGE_H_
#include "libmpkg.h"
enum {
	DATATYPE_UNKNOWN = 0,
	DATATYPE_NEW,
	DATATYPE_BINARYPACKAGE,
	DATATYPE_SOURCEPACKAGE,
	DATATYPE_XML,
	DATATYPE_CURRENTDIR
};
class BinaryPackage
{
	public:
		BinaryPackage();
		BinaryPackage(string in_file);
		~BinaryPackage();
		bool isExtracted();
		bool extracted;

		bool createNew();
		bool createWorkingDirectory();
		bool createFolderStructure();
		
		bool fileOk();
		string input_file;

		bool setInputFile(string in_file); // return true if file exists and readable, false if not

		string pkg_dir; // Extracted package root (some tmp if new
		bool unpackFile(); // Extract contents to a temp directory
	       	bool packFile(string output_directory=""); // Pack a structure.

		string getExtractedPath();
		
		string getDataFilename(); // Filename of data.xml
		
		bool setPreinstallScript(string script_text);
		bool setPostinstallScript(string script_text);
		bool setPreremoveScript(string script_text);
		bool setPostremoveScript(string script_text);


		string readPreinstallScript();
		string readPostinstallScript();
		string readPreremoveScript();
		string readPostremoveScript();

};

class SourcePackage: public BinaryPackage
{
	public:
		bool unpackFile();
		bool packFile(string output_directory="");
		bool createFolderStructure();

		bool embedPatch(string filename); // Copy patch file to a package structure
		bool embedSource(string filename); // Copy sources for a package structure
		string source_filename;
		bool removeSource();
		bool removeAllPatches();
		bool removePatch(string patch_name); // specify patch filename (without path) to remove from structure

		bool setBuildScript(string script_text);
		string readBuildScript();

};

#endif
