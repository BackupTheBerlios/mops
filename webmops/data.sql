-- MySQL dump 10.10
--
-- Host: localhost    Database: webmops4
-- ------------------------------------------------------
-- Server version	5.0.24a-Debian_9ubuntu0.1-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `distributions`
--

DROP TABLE IF EXISTS `distributions`;
CREATE TABLE `distributions` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `description` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `distributions`
--


/*!40000 ALTER TABLE `distributions` DISABLE KEYS */;
LOCK TABLES `distributions` WRITE;
INSERT INTO `distributions` VALUES (1,'mops-current','MOPS Linux current version');
UNLOCK TABLES;
/*!40000 ALTER TABLE `distributions` ENABLE KEYS */;

--
-- Table structure for table `files`
--

DROP TABLE IF EXISTS `files`;
CREATE TABLE `files` (
  `id` int(11) NOT NULL auto_increment,
  `package_id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `files`
--


/*!40000 ALTER TABLE `files` DISABLE KEYS */;
LOCK TABLES `files` WRITE;
INSERT INTO `files` VALUES (1,10,'file-for-test-0'),(2,10,'file-for-test-1'),(3,10,'file-for-test-2'),(4,10,'file-for-test-3'),(5,10,'file-for-test-4'),(6,10,'file-for-test-5'),(7,10,'file-for-test-6'),(8,10,'file-for-test-7'),(9,10,'file-for-test-8'),(10,10,'file-for-test-9'),(11,11,'file-for-test-0'),(12,11,'file-for-test-1'),(13,11,'file-for-test-2'),(14,11,'file-for-test-3'),(15,11,'file-for-test-4'),(16,11,'file-for-test-5'),(17,11,'file-for-test-6'),(18,11,'file-for-test-7'),(19,11,'file-for-test-8'),(20,11,'file-for-test-9'),(21,12,'file-for-test-0'),(22,12,'file-for-test-1'),(23,12,'file-for-test-2'),(24,12,'file-for-test-3'),(25,12,'file-for-test-4'),(26,12,'file-for-test-5'),(27,12,'file-for-test-6'),(28,12,'file-for-test-7'),(29,12,'file-for-test-8'),(30,12,'file-for-test-9');
UNLOCK TABLES;
/*!40000 ALTER TABLE `files` ENABLE KEYS */;

--
-- Table structure for table `mirrors`
--

DROP TABLE IF EXISTS `mirrors`;
CREATE TABLE `mirrors` (
  `id` int(11) NOT NULL auto_increment,
  `distribution_id` int(11) NOT NULL default '0',
  `url` varchar(255) NOT NULL default '',
  `type` varchar(20) NOT NULL default 'HTTP',
  `name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `mirrors`
--


/*!40000 ALTER TABLE `mirrors` DISABLE KEYS */;
LOCK TABLES `mirrors` WRITE;
INSERT INTO `mirrors` VALUES (1,1,'http://test/mops-current','HTTP','test'),(2,1,'http://test2/test','HTTP','test2');
UNLOCK TABLES;
/*!40000 ALTER TABLE `mirrors` ENABLE KEYS */;

--
-- Table structure for table `packages`
--

DROP TABLE IF EXISTS `packages`;
CREATE TABLE `packages` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `arch` varchar(255) NOT NULL default 'i386',
  `build` varchar(255) NOT NULL default '1',
  `short_description` varchar(255) default '""',
  `description` text,
  `maintainer_email` varchar(255) default '',
  `maintainer` varchar(255) default '',
  `md5` varchar(255) NOT NULL default '',
  `filename` varchar(255) NOT NULL default '',
  `location` varchar(255) NOT NULL default '',
  `version` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `packages_ix_1` (`name`,`arch`),
  KEY `packages_ix_2` (`name`,`short_description`),
  KEY `package_ix_3` (`name`,`short_description`,`filename`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `packages`
--


/*!40000 ALTER TABLE `packages` DISABLE KEYS */;
LOCK TABLES `packages` WRITE;
INSERT INTO `packages` VALUES (10,'test1','i386','1','some short description','Some description...',NULL,NULL,'1337','test1-1.0.1-i386-1.tgz','./1/','1.0.1'),(11,'test1','i386','1','some short description','Some description...',NULL,NULL,'1337','test1-1.0.1-i386-1.tgz','./1/','2.3.1'),(12,'test1','i386','1','some short description','Some description...',NULL,NULL,'1337','test1-1.0.1-i386-1.tgz','./1/','0.99');
UNLOCK TABLES;
/*!40000 ALTER TABLE `packages` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

