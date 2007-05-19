----------------------------------------------------------------------
--
--	MOPSLinux package system
--	Database creation script
--	$Id: create_database.sql,v 1.14 2007/05/19 17:45:20 i27249 Exp $
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
	package_installed INTEGER NOT NULL,
	package_configexist INTEGER NOT NULL,
	package_action INTEGER NOT NULL,
	package_md5 TEXT NOT NULL,
	package_filename NOT NULL
);
create index ppname on packages (package_id, package_name, package_version, package_action, package_installed, package_md5);

create table files (
	file_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	file_name TEXT NOT NULL,
	file_type INTEGER NOT NULL,
	packages_package_id INTEGER NOT NULL
);
create index pname on files (file_name, packages_package_id);

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
create index locpid on locations(packages_package_id, location_path, server_url);
create table tags (
	tags_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	tags_name TEXT NOT NULL
);
create index ptag on tags (tags_id, tags_name);

create table tags_links (
	tags_link_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	packages_package_id INTEGER NOT NULL,
	tags_tag_id INTEGER NOT NULL
);
create index ptaglink on tags_links (packages_package_id, tags_tag_id);

create table dependencies (
	dependency_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	packages_package_id INTEGER NOT NULL,
	dependency_condition INTEGER NOT NULL DEFAULT '1',
	dependency_type INTEGER NOT NULL DEFAULT '1',
	dependency_package_name TEXT NOT NULL,
	dependency_package_version TEXT NULL
);

create index pdeps on dependencies (packages_package_id, dependency_id, dependency_package_name, dependency_package_version, dependency_condition);

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
