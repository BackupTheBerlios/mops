// $Id: package.cpp,v 1.2 2007/10/23 22:43:55 i27249 Exp $

#include "package.h"
BinaryPackage::BinaryPackage()
{
	extracted=false;
}

BinaryPackage::BinaryPackage(string in_file)
{
	extracted=false;
	input_file = in_file;
}

BinaryPackage::~BinaryPackage()
{
	if (extracted) printf("Warning! destroyed an package object without packing back. Probably you're lost your package...\n");
}

bool BinaryPackage::isExtracted()
{
	return extracted;
}

bool BinaryPackage::fileOk()
{
	if (access(input_file.c_str(), R_OK)==0) return true;
	else return false;
}
bool BinaryPackage::createWorkingDirectory()
{
	pkg_dir = get_tmp_file();
	unlink(pkg_dir.c_str());
	if (mkdir(pkg_dir.c_str(), 755)!=0) {
		extracted=false;
		return false;
	}	
	else {
		extracted=true;
	       	return true;
	}
}
bool BinaryPackage::createFolderStructure()
{
	if (extracted)
	{
		string _dinstall = pkg_dir + "/install";
		if (mkdir(_dinstall.c_str(), 755)==0) return true;
		else return false;
	}
	else {
		mError("Mmmm... Where to create, yep?");
		return false;
	}
}
bool BinaryPackage::createNew()
{
	if (createWorkingDirectory() && createFolderStructure()) return true;
	else {
		mError("Unable to create");
		return false;
	}
	return true;
}

bool BinaryPackage::setInputFile(string in_file)
{
	if (extracted) {
		printf("Warning: assigning a new filename while previous was extracted\n");
	}
	input_file = in_file;
	return fileOk();
}

bool BinaryPackage::unpackFile()
{
	if (extracted) {
		mError("Unable to extract package, because it is already extracted\n");
		return false;
	}
	if (!fileOk()) {
		mError("Unable to extract package: cannot find or read the archive");
		return false;
	}
	// All seems to be ok?
	
	createWorkingDirectory();
	if (system("tar zxvf " + input_file + " -C " + pkg_dir)!=0) {
		mError("Error while extracting archive");
		return false;
	}
	extracted=true;
	return true;
}

bool BinaryPackage::packFile(string output_directory)
{
	string oldinput = input_file;
	if (output_directory.empty())
	{
		if (input_file.empty()) {
			mError("Don't know where to write");
			return false;
		}
		else output_directory = getDirectory(input_file);
	}
	if (system("(cd " + pkg_dir+"; buildpkg " + output_directory+")")==0) {
		if (!oldinput.empty()) unlink(oldinput.c_str());
		return true;
	}
	else {
		mError("Failed to build a package");
		return false;
	}
}

string BinaryPackage::getExtractedPath()
{
	return pkg_dir;
}
string BinaryPackage::getDataFilename()
{
	return pkg_dir+"/install/data.xml";
}

bool BinaryPackage::setPreinstallScript(string script_text)
{
	if (extracted) {

		createFolderStructure();
		if (script_text.empty())
		{
			string n=pkg_dir + "/install/preinst.sh";
			unlink(n.c_str());
			return true;
		}
		if (WriteFile(pkg_dir+"/install/preinst.sh", script_text)==0) return true;
		else {
			mError("Error writing script");
			return false;
		}
	}
	else {
		mError("No working directory");
		return false;
	}
}
bool BinaryPackage::setPostinstallScript(string script_text)
{
	if (extracted) {

		createFolderStructure();
		if (script_text.empty())
		{
			string n=pkg_dir + "/install/doinst.sh";
			unlink(n.c_str());
			return true;
		}

		if (WriteFile(pkg_dir+"/install/doinst.sh", script_text)==0) return true;
		else {
			mError("Error writing script");
			return false;
		}
	}
	else {
		mError("No working directory");
		return false;
	}
}
bool BinaryPackage::setPreremoveScript(string script_text)
{
	if (extracted) {

		createFolderStructure();
		if (script_text.empty())
		{
			string n=pkg_dir + "/install/preremove.sh";
			unlink(n.c_str());
			return true;
		}

		if (WriteFile(pkg_dir+"/install/preremove.sh", script_text)==0) return true;
		else {
			mError("Error writing script");
			return false;
		}
	}
	else {
		mError("No working directory");
		return false;
	}
}
bool BinaryPackage::setPostremoveScript(string script_text)
{
	if (extracted) {

		createFolderStructure();
		if (script_text.empty())
		{
			string n=pkg_dir + "/install/postremove.sh";
			unlink(n.c_str());
			return true;
		}

		if (WriteFile(pkg_dir+"/install/postremove.sh", script_text)==0) return true;
		else {
			mError("Error writing script");
			return false;
		}
	}
	else {
		mError("No working directory");
		return false;
	}
}

string BinaryPackage::readPreinstallScript()
{
	return ReadFile(pkg_dir+"/install/preinst.sh");
}

string BinaryPackage::readPostinstallScript()
{
	return ReadFile(pkg_dir+"/install/doinst.sh");
}
string BinaryPackage::readPreremoveScript()
{
	return ReadFile(pkg_dir+"/install/preremove.sh");
}
string BinaryPackage::readPostremoveScript()
{
	return ReadFile(pkg_dir+"/install/postremove.sh");
}

bool SourcePackage::createFolderStructure()
{
	if (extracted)
	{
		string _dinstall = pkg_dir + "/install";
		string _patchdir = pkg_dir + "/patches";
		string _builddir = pkg_dir + "/build_data";
		if (mkdir(_dinstall.c_str(), 755)==0 && \
			mkdir(_patchdir.c_str(), 755)==0 && \
			mkdir(_builddir.c_str(), 755)) return true;
		else return false;
	}
	else {
		mError("Mmmm... Where to create, yep?");
		return false;
	}
}

bool SourcePackage::embedPatch(string filename)
{
	if (extracted) {
		return copyFile(filename, pkg_dir + "/patches/");
	}
	else return false;
}

bool SourcePackage::embedSource(string filename)
{
	if (extracted) {
		if (!source_filename.empty()) removeSource();
		if (filename.find("/")==std::string::npos) source_filename = filename;
		else {
			if (filename.find_last_of("/")<filename.length()-1) source_filename = filename.substr(filename.find_last_of("/")+1);
			else {
				mError("Invalid filename");
				return false;
			}
		}
		return copyFile(filename, pkg_dir + "/");
	}
	else return false;
}

bool SourcePackage::removeSource()
{
	if (extracted)
	{
		if (source_filename.empty()) return false;
		if (unlink(source_filename.c_str())==0) return true;
		else return false;
	}
	else return false;
}

bool SourcePackage::removeAllPatches()
{
	if (extracted)
	{
		if (system("rm " + pkg_dir + "/patches/*")==0) return true;
		else return false;
	}
	else return false;
}

bool SourcePackage::removePatch(string patch_name)
{
	if (extracted)
	{
		string n = pkg_dir + "/patches/" + patch_name;
		if (!FileExists(n)) return false;
		if (unlink(n.c_str())==0) return true;
		else return false;
	}
	else return false;
}

bool SourcePackage::setBuildScript(string script_text)
{
	if (extracted)
	{
		createFolderStructure();
		if (script_text.empty()) {
			string n = pkg_dir + "/build_data/build.sh";
			unlink(n.c_str());
			return true;
		}

		if (WriteFile(pkg_dir+"/build_data/build.sh", script_text)==0) return true;
		else return false;
	}
	else return true;
}

string SourcePackage::readBuildScript()
{
	return ReadFile(pkg_dir+"/build_data/build.sh");
}

bool SourcePackage::unpackFile()
{
	if (extracted) {
		mError("Unable to extract package, because it is already extracted\n");
		return false;
	}
	if (!fileOk()) {
		mError("Unable to extract package: cannot find or read the archive");
		return false;
	}
	// All seems to be ok?
	
	createWorkingDirectory();
	if (system("tar xvf " + input_file + " -C " + pkg_dir)!=0) {
		mError("Error while extracting archive");
		return false;
	}
	extracted=true;
	return true;
}
bool SourcePackage::packFile(string output_directory)
{
	string oldinput = input_file;
	if (output_directory.empty())
	{
		if (input_file.empty()) {
			mError("Don't know where to write");
			return false;
		}
		else output_directory = getDirectory(input_file);
	}
	if (system("(cd " + pkg_dir+"; buildsrcpkg " + output_directory+")")==0) {
		if (!oldinput.empty()) unlink(oldinput.c_str());
		return true;
	}
	else {
		mError("Failed to build a package");
		return false;
	}
}

