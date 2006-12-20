/*******************************************************
 * File operations
 * $Id: file_routines.cpp,v 1.1 2006/12/20 20:00:38 i27249 Exp $
 * ****************************************************/

#include "file_routines.h"

vector<string> temp_files;
extern int errno;

string get_tmp_file()
{
	string tmp_fname;
	debug("get_tmp_file start");
	char *t=tmpnam(NULL);
	if ( t == NULL  )
		return NULL;

	tmp_fname=t;
	debug("get_tmp_file end");
	//free(t);
	temp_files.resize(temp_files.size()+1);
	temp_files[temp_files.size()-1]=tmp_fname;
	return tmp_fname;
}

void delete_tmp_files()
{
	debug("preparing to remove temp files");

	for ( int i = 0; i < temp_files.size(); i++ ) {
		if ( unlink( temp_files[i].c_str() ) != 0 ) {
			debug("cannot delete temp file ");
			debug( temp_files[i] );
			debug("\n");
			perror( strerror( errno ) );

		}		
	}

	temp_files.clear(); // Clean-up list - for future use
}

bool FileExists(string filename)
{
	FILE *test=fopen(filename.c_str(),"r");
	if (test)
	{
		fclose(test);
		return true;
	}
	else
	{
		fclose(test);
		return false;
	}
}

string ReadFile(string filename)
{
	FILE *src=fopen(filename.c_str(),"r");
	string ret;
	char buf;
	if (src)
	{
		for (int i=0;buf!=EOF;i++)
		{
			buf=fgetc(src);
			if (buf!=EOF) ret+=buf;
		}
		fclose(src);
		return ret;
	}
	else
	{
		printf("Cannot open file %s\n", filename.c_str());
		return "";
	}
}

int WriteFile(string filename, string data)
{
	FILE* output=fopen(filename.c_str(),"w");
	if (output)
	{
		for (int i=0;i<data.length();i++)
		{
			fputc(data[i],output);
		}
		fclose(output);
		return 0;
	}
	else
	{
		fclose(output);
		printf("Unable to write file %s\n", filename.c_str());
		return 1;
	}
}

