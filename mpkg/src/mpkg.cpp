/***********************************************************************
 * 	$Id: mpkg.cpp,v 1.2 2006/12/17 19:34:57 i27249 Exp $
 * 	MOPSLinux packaging system
 * ********************************************************************/
#include "mpkg.h"

/** Scans database and do actions. Actually, packages will install in SYS_ROOT folder (for testing).
 * In real systems, set SYS_ROOT to "/"
 * @**/

mpkgDatabase::mpkgDatabase()
{
	CheckDatabaseIntegrity();
}
mpkgDatabase::~mpkgDatabase(){}

string mpkgDatabase::get_file_md5(string filename)
{
	debug("get_file_md5 start");
	string tmp_md5=get_tmp_file();

	string sys="md5sum "+filename+" > "+tmp_md5;
	system(sys.c_str());
	FILE* md5=fopen(tmp_md5.c_str(), "r");
	if (!md5)
	{
		fprintf(stderr, "Unable to open md5 temp file\n");
		return "";
	}
	char _c_md5[1000];
	fscanf(md5, "%s", &_c_md5);
	string md5str;
	md5str=_c_md5;
	fclose(md5);
	return md5str;
}

void mpkgDatabase::commit_actions()
{
	// First: removing required packages
	PACKAGE_LIST remove_list;
	get_packagelist("select * from packages where package_status='"+IntToStr(PKGSTATUS_REMOVE)+"';", &remove_list);
	for (int i=0;i<remove_list.size();i++)
	{
		remove_package(remove_list.get_package(i));
	}


	// Second: installing required packages
	PACKAGE_LIST install_list;
	get_packagelist("select * from packages where package_status='"+IntToStr(PKGSTATUS_INSTALL)+"';", &install_list);
	for (int i=0; i<install_list.size(); i++)
	{
		fetch_package(install_list.get_package(i));
	}
	for (int i=0;i<install_list.size();i++)
	{
		install_package(install_list.get_package(i));
	}
}

int mpkgDatabase::fetch_package(PACKAGE *package)
{
		// Смотрим все доступные locations пакета, точнее - их сервера. 
		// Сервера подразделяются по типам на следующие группы:
		// cache - это локальный кеш пакетов. URL: cache://
		// file - по сути - псевдосервер, его наличие означает что пакет ставят напрямую с локального носителя. URL: file://
		// network - сетевой репозиторий. URL: http://, ftp://, rsync://, smb://, nfs://, https://, ftps://, и другие (какие реализуем). Пока только первые 2.
		// cdrom - по сути локальный репозиторий, но имеющий свою специфику. Вид URL: cdrom://cdrom_id/
		//
		// Тип сервера определяется по его URL методом SERVER::get_type().
		// 
		// Если тип сервера file, то сначала проверяется существование файла, после создается symlink из кеша на файл. Если файла нету - ищем дальше.
		// Если тип сервера cache, то достаточно просто проверить наличие файла в указанном месте и его md5. Если файла нет - ищем дальше.
		// 
		// Если тип сервера network - то мы запускаем выкачивание файла. Если файл скачался - проверяем его md5. Если файл не скачался или сумма не совпадает
		// то пробуем следующий сервер до тех пор пока файл не окажется в кеше. Кеш по умолчанию - SYS_CACHE.
		//
		// Если тип сервера cdrom - то мы сначала требуем вставить нужный диск в нужное устройство, монтируем его, и дальше как будто это file:
		// проверяем существование, создаем симлинк.
		//
		// Приоритет серверов определяется методом SERVER::get_priority(). Высший приоритет ВСЕГДА имеют сервера file. Умолчальная расстановка такова:
		// 1. cache (приоритеты от 3000 до 3999)
		// 2. cdrom (приоритеты от 2000 до 2999)
		// 3. network (приоритеты от 1 до 1999)
		// Сервер, имеющий приоритет 0, считается отключенным и никогда не принимает участие в выборке.
		//
		// TODO: многопотоковая загрузка с равноприоритетных серверов.

	// Step 1. Checking for local copy of file (filesystem or cache)

	LOCATION_LIST locationlist; 	// Sorted location list
	LOCATION location;
	int min_priority; int min_priority_id; int prev_min;
	min_priority=0;
	prev_min=-1;
	int server_priority;

	int _srv_type;
	string _fname;
	string _sys;
	FILE *_ftmp;



	for (int x=0; x<package->get_locations()->size();x++)
	{
		for (int i=0; i<package->get_locations()->size(); i++)
		{
			server_priority=atoi(package->get_locations()->get_location(i)->get_server()->get_priority().c_str());
			if ((i==0 || server_priority<min_priority) && server_priority>prev_min)
			{
				min_priority=server_priority;
				min_priority_id=i;
			}
		}
		for (int i=0; i<package->get_locations()->size(); i++)
		{
			if (i==min_priority_id) locationlist.add(*package->get_locations()->get_location(i));
		}
		prev_min=min_priority;
	}

#ifdef ENABLE_DEBUG
	for (int i=0; i<locationlist.size();i++)
	{
		debug("Priority: "+locationlist.get_location(i)->get_server()->get_priority());
	}
#endif
	for (int i=0; i<locationlist.size(); i++) // First, searching local file servers.
	{
		if (locationlist.get_location(i)->get_server()->get_type()==SRV_FILE)
		{
			// Checking file existance
			string _fname;
			string _sys;
			_fname=locationlist.get_location(i)->get_path()+package->get_filename();
			_ftmp=fopen(_fname.c_str(),"r");
			if (_ftmp)
			{
				debug("File "+_fname+" exists , proceeding next: checking md5");
				fclose(_ftmp);
				// Checking md5
				if (package->get_md5()==get_file_md5(_fname))
				{
					debug("md5 ok");
					_sys="ln -s "+_fname+" "+SYS_CACHE;
					debug("Creating symlink: "+_sys);
					system(_sys.c_str());
					return 0; // File successfully delivered thru symlink, we can exit function now
				}
				else
				{
					debug("md5 incorrect");
				}
			}
			else
			{
				debug("File not found, searching next server");
			}
		}
	} // End of searching local file servers

	for (int i=0; i<locationlist.size(); i++) // Second: looking for all other servers ordering by priority.
	{
		_srv_type=locationlist.get_location(i)->get_server()->get_type();
		switch (_srv_type)
		{
			case SRV_CACHE:
				// Checking file existance
				_fname = SYS_CACHE + locationlist.get_location(i)->get_path() + package->get_filename();
				_ftmp=fopen(_fname.c_str(),"r");
				if (_ftmp)
				{
					debug("File "+_fname+" exists , proceeding next: checking md5");
					fclose(_ftmp);
					// Checking md5
					if (package->get_md5()==get_file_md5(_fname))
					{
						debug("md5 ok");
						return 0; // File successfully delivered thru symlink, we can exit function now
					}
					else
					{
						debug("md5 incorrect");
					}
				}
				else
				{
					fclose(_ftmp);
					debug("File not found, searching next server");
				}
				break;
			case SRV_CDROM:
				// Not implemented yet.
				debug("Fetching from CD-COM is not implemented yet");
				break;
			case SRV_HTTP:
				// Not implemented yet.
				debug("Fetching from HTTP is not implemented yet");
				break;
			case SRV_FTP:
				// Not implemented yet.
				debug("Fetching from FTP is not implemented yet");
				break;
			case SRV_SMB:
				// Not implemented yet.
				debug("Fetching from SMB is not implemented yet");
				break;
			case SRV_HTTPS:
				// Not implemented yet.
				debug("Fetching from HTTPS is not implemented yet");
				break;
			default:
				debug ("Server type not recognized or not supported");
				break;
		}
	}
	// If we reach this point - this means that we cannot get package. Returning error...
	return 1;
} // End of mpkgDatabase::fetch_package();

int mpkgDatabase::install_package(PACKAGE* package)
{
	set_status(IntToStr(package->get_id()), PKGSTATUS_INSTALLED);
	debug("********************************\n*                                   *\n*     Package installed sussessfully     *\n********************************");
}

int mpkgDatabase::remove_package(PACKAGE* package)
{
}

