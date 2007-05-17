/******************************************************
 * Converter for legacy Slackware packages
 * $Id: converter.h,v 1.3 2007/05/17 15:12:36 i27249 Exp $
 * ****************************************************/
#ifndef CONVERTER_H_
#define CONVERTER_H_
#include "debug.h"
#include "mpkg.h"
#include "repository.h"
int slack_convert(string filename, string xml_output);
int convert_package(string filename, string output_dir);
int tag_package(string filename, string tag);
#endif
