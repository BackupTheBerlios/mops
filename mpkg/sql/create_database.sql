----------------------------------------------------------------------
--
--	MOPSLinux package system
--	Database creation script
--	$Id: create_database.sql,v 1.12 2007/05/18 07:35:33 i27249 Exp $
--
----------------------------------------------------------------------

create table packages (
	package_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	package_name TEXT NOT NULL,
	package_version TEXT NOT NULL,
	package_arch TEXT NOT NULL,
	package_build TEXT NULL,
	package_compressed_size TEXT NOT NULL,
	package_installed_size TEXT NOT NULL,
	package_short_description TEXT NULL,
	package_description TEXT NULL, 
	package_changelog TEXT NULL,
	package_packager TEXT NULL,
	package_packager_email TEXT NULL,
	package_available INTEGER NOT NULL,
	package_installed INTEGER NOT NULL,
	package_configexist INTEGER NOT NULL,
	package_action INTEGER NOT NULL,
	package_md5 TEXT NOT NULL,
	package_filename NOT NULL
);
create index ppname on packages (package_name);
create index ppaction on packages(package_action);

create table files (
	file_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	file_name TEXT NOT NULL,
	file_type INTEGER NOT NULL,
	packages_package_id INTEGER NOT NULL
);
create index pname on files (file_name);
create index fpid on files(packages_package_id);

create table conflicts (
	conflict_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	conflict_file_name TEXT NOT NULL,
	backup_file TEXT NOT NULL,
	conflicted_package_id INTEGER NOT NULL
);



create table locations (
	location_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	packages_package_id INTEGER NOT NULL,
	server_url TEXT NOT NULL,
	location_path TEXT NOT NULL
);
create index locpid on locations(packages_package_id);
create table tags (
	tags_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	tags_name TEXT NOT NULL
);

create table tags_links (
	tags_link_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	packages_package_id INTEGER NOT NULL,
	tags_tag_id INTEGER NOT NULL
);

create table dependencies (
	dependency_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	packages_package_id INTEGER NOT NULL,
	dependency_condition INTEGER NOT NULL DEFAULT '1',
	dependency_type INTEGER NOT NULL DEFAULT '1',
	dependency_package_name TEXT NOT NULL,
	dependency_package_version TEXT NULL
);
-- INTERNATIONAL SUPPORT

--create table descriptions (
--	description_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
--	packages_package_id INTEGER NOT NULL,
--	description_language TEXT NOT NULL,
--	description_text TEXT NOT NULL,
--	short_description_text TEXT NOT NULL
--);

--create table changelogs (
--	changelog_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
--	packages_package_id INTEGER NOT NULL,
--	changelog_language TEXT NOT NULL,
--	changelog_text TEXT NOT NULL
--);

-- RATING SYSTEM - SUPPORT FOR FUTURE
--create table ratings (
--	rating_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
--	rating_value INTEGER NOT NULL,
--	packages_package_name TEXT NOT NULL
--);
