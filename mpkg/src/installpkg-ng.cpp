/**
	MOPSLinux packaging system    
	    installpkg-ng
	New generation of installpkg :-)
	This tool ONLY can install concrete local file
				    **/


#include "local_package.h"
#include "debug.h"
#include "mpkg.h"
int main (int argc, char **argv)
{
	if (argc!=2)
	{
		printf("usage: %s [package_file]\n[debug: argc is %d]\n", argv[0], argc);
		return 1;
	}
	
	string action=argv[0]; // We will depend on installpkg or removepkg to decide what to do
	string fname=argv[1];
	if (true)
	{
		// Part 0. Check if file exists.
		debug("opening file...");
		FILE* _f=fopen(fname.c_str(),"r");
		if (_f)
		{
			debug("File opened successfully, so it exists");
			fclose(_f);
			debug("File closed.");
		}
		else
		{
			printf("Error: file %s not found.\n", fname.c_str());
			return 1;
		}

		// Part 1. Extracts all information from file and fill the package structure

		LocalPackage lp(fname);
		lp.injectFile(); // После этого, впринципе можно считать что файл всосан полностью - осталось только добавить его в базу данных и непосредственно 
				 // установить
		if (CheckDatabaseIntegrity())
		{
			lp.data.set_status(PKGSTATUS_AVAILABLE);
			add_package_record(&lp.data);
			DependencyTracker DepTracker;
			DepTracker.merge(&lp.data);

			printf("Next packages will install:\n");
			for (int i=0;i<DepTracker.get_install_list()->size();i++)
			{
				printf("%s\n", DepTracker.get_install_list()->get_package(i)->get_name(false).c_str());
				DepTracker.PrintFailure(DepTracker.get_install_list()->get_package(i));
			}
		
			printf("Next packages will REMOVE:\n");
			for (int i=0;i<DepTracker.get_remove_list()->size();i++)
			{
				DepTracker.PrintFailure(DepTracker.get_remove_list()->get_package(i));
				printf("%s", DepTracker.get_remove_list()->get_package(i)->get_name(false).c_str());
			}
			printf("Next packages is BROKEN:\n");
			for (int i=0;i<DepTracker.get_failure_list()->size();i++)
			{
				DepTracker.PrintFailure(DepTracker.get_failure_list()->get_package(i));
				printf("%s", DepTracker.get_failure_list()->get_package(i)->get_name(false).c_str());
			}
			
			DepTracker.commitToDb();
			mpkgDatabase db;
			db.commit_actions();

		}


		/*		PhysicallyInstall(merge.install_list);		*/
		/*		PhysicallyRemove(merge.remove_list);
		 *		PhysicallyUpdate(merge.update_list);
		 *
		 */
		return 0;
	}
	
	/*if (action=="removepkg")
	{
	    printf("Removing still not implemented\n");
	
	}
	if (action!="installpkg" && action!="removepkg")
	{
	    printf("Error: the program should be named \"installpkg\" to install packages and \"removepkg\" to remove them.\n");
	}
	*/
	
	return 0;
}
