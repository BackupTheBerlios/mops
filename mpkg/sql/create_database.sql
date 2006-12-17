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
	package_status INTEGER NOT NULL DEFAULT '0',
	package_md5 TEXT NOT NULL,
	package_filename NOT NULL
);

create table files (
	file_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	file_name TEXT NOT NULL,
	file_size TEXT NOT NULL,
	packages_package_id INTEGER NOT NULL
);

create table locations (
	location_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	packages_package_id INTEGER NOT NULL,
	servers_server_id INTEGER NOT NULL,
	location_path TEXT NOT NULL
);

create table servers (
	server_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	server_url TEXT NOT NULL,
	server_priority INTEGER NOT NULL DEFAULT '1' 
);

create table tags (
	tags_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	tags_name TEXT NOT NULL
);

create table server_tags (
	server_tag_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	server_tag_name TEXT NOT NULL
);

create table server_tags_links (
	server_tags_link_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
	servers_server_id INTEGER NOT NULL,
	server_tags_server_tag_id INTEGER NOT NULL
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
