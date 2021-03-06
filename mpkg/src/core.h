/********************************************************************************
 *					core.h
 * 			Central core for MOPSLinux package system
 *					Headers
 *	$Id: core.h,v 1.14 2007/08/30 21:46:48 i27249 Exp $
 ********************************************************************************/
#ifndef CORE_H_
#define CORE_H_

#include "dataunits.h"
#include "constants.h"
#include "string.h"
//string T="', '"; // Wery helpful element for SQL queries

//#define T "', '"   // Wery helpful element for SQL queries

#define SEARCH_OR 	0x01
#define SEARCH_AND 	0x02
#define SEARCH_IN	0x03
#define EQ_LIKE 0x01
#define EQ_EQUAL 0x02
#define EQ_CUSTOMLIKE 0x03
typedef struct
{
	string fieldname;
	string value;
} SQLField;

class SQLRecord
{
	private:
		vector<SQLField> field;
		int search_type;
		int eq_type;

	public:
		int size();
		bool empty();
		void clear();
		vector<string> getRecordValues();
		string* getFieldName(unsigned int num);
		string* getValue(string fieldname);
		string* getValueI(unsigned int num); // returns the indexed field value 
		int getFieldIndex(string fieldname);
		void setSearchMode(int mode);
		int getSearchMode();
		void setEqMode(int mode);
		int getEqMode();
		void addField(string fieldname, string *value);
		void addField(string fieldname, int value);
		void addField(string fieldname);
		bool setValue(string fieldname, string *value);
		void setValue(unsigned int field_index, string *value);

		SQLRecord();
		~SQLRecord();
};

class SQLTable
{
	private:
		vector<SQLRecord> table;
	public:
		int getRecordCount(); 	// returns record count
		int size();
		bool empty();		// returns TRUE if table is empty (record count = 0), otherwise returns false
		void clear();		// clears table
		string* getValue (unsigned int num, string fieldname);	// returns value of field called fieldname in num record
		string * getValue(unsigned int num, unsigned int field_index); // returns the value of indexed field
		int getFieldIndex(string fieldname); // returns the field index
		SQLRecord* getRecord(unsigned int num);

		void addRecord(SQLRecord* record);
		SQLTable();
		~SQLTable();
};



#include "sql_pool.h"
#endif //CORE_H_

