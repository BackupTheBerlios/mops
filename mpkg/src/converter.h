/******************************************************
 * Converter for legacy Slackware packages
 * $Id: converter.h,v 1.2 2007/03/26 14:32:32 i27249 Exp $
 * ****************************************************/
#ifndef CONVERTER_H_
#define CONVERTER_H_
#include "mpkg.h"
#include "repository.h"
int slack_convert(string filename, string xml_output);
int convert_package(string filename, string output_dir);
int tag_package(string filename, string tag);
#endif
