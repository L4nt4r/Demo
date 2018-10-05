#include "ttfParser.h"


int TTFFile::parse(const char* file_name, /*TTFFontParser::FontData* font_data, TTFFontParser::TTF_FONT_PARSER_CALLBACK callback,*/ void* args){
	QByteArray Blob;
	QFile file(file_name);
	if (file.isOpen()) {
		Blob = file.readAll();
		file.close();
	}
	else {
		return -1;
	}

	int result = parse_data(Blob);
	return result;
}

int TTFFile::parse_data(QByteArray Blob){
	static unsigned int little_endian_test = 0x01234567;
	if (endian_tested == false) {
		if (((*((uint8_t*)(&little_endian_test))) == 0x67) == true) {
			get2b = &get2b_little_endian;
			get4b = &get4b_little_endian;
			get8b = &get8b_little_endian;
		}
		else {
			get2b = &get2b_big_endian;
			get4b = &get4b_big_endian;
			get8b = &get8b_big_endian;
		}
		endian_tested = true;
	}

	parse(header, Blob);
	
	 
	if (!tableMap.contains("head"))
		return -2;
	parse(headTable,Blob, tableMap["head"].offsetPos);
	
	if (!tableMap.contains("maxp"))
		return -2;
	parse(maxProfile, Blob, tableMap["head"].offsetPos);

	if (!tableMap.contains("name"))
		return -2;
	parse(nameTable, Blob, tableMap["name"].offsetPos);

	fullFontName = namesList[1] + " " + namesList[2];

	if (!tableMap.contains("loca"))
		return -2;
	parse(maxProfile, Blob, tableMap["head"].offsetPos);
}

void TTFFile::parse(TTFHeader &header, QByteArray Blob){
	unsigned int offset = 0;
	get4b(&header.version, Blob.data() + offset);								offset += sizeof(int);
	get2b(&header.numTables, Blob.data() + offset);								offset += sizeof(short);

	offset += sizeof(short)* 3;
	
	TableEntry tempTableEntry;

	for (unsigned short i = 0; i < header.numTables; i++)
	{
		get4b(&tempTableEntry.tag, Blob.data() + offset);
		memcpy(&tempTableEntry.tagstr, Blob.data() + offset, sizeof(unsigned int));
		tempTableEntry.tagstr[4] = 0;											offset += sizeof(int);
		get4b(&tempTableEntry.checkSum, Blob.data() + offset);					offset += sizeof(int);
		get4b(&tempTableEntry.offsetPos, Blob.data() + offset);					offset += sizeof(int);
		get4b(&tempTableEntry.length, Blob.data() + offset);					offset += sizeof(int);

		tableMap[tempTableEntry.tagstr] = tempTableEntry;
	}
}

void TTFFile::parse(MaximumProfile &maxProfile, QByteArray Blob, unsigned int offset){

	get4b(&maxProfile.version, Blob.data() + offset);							offset += sizeof(int);
	get2b(&maxProfile.numGlyphs, Blob.data() + offset);							offset += sizeof(short);
	get2b(&maxProfile.maxPoints, Blob.data() + offset);							offset += sizeof(short);
	get2b(&maxProfile.maxContours, Blob.data() + offset);						offset += sizeof(short);
	get2b(&maxProfile.maxCompositePoints, Blob.data() + offset);				offset += sizeof(short);
	get2b(&maxProfile.maxCompositeContours, Blob.data() + offset);				offset += sizeof(short);
	get2b(&maxProfile.maxZones, Blob.data() + offset);							offset += sizeof(short);
	get2b(&maxProfile.maxTwilightPoints, Blob.data() + offset);					offset += sizeof(short);
	get2b(&maxProfile.maxStorage, Blob.data() + offset);						offset += sizeof(short);
	get2b(&maxProfile.maxFunctionDefs, Blob.data() + offset);					offset += sizeof(short);
	get2b(&maxProfile.maxInstructionDefs, Blob.data() + offset);				offset += sizeof(short);
	get2b(&maxProfile.maxStackElements, Blob.data() + offset);					offset += sizeof(short);
	get2b(&maxProfile.maxSizeOfInstructions, Blob.data() + offset);				offset += sizeof(short);
	get2b(&maxProfile.maxComponentElements, Blob.data() + offset);				offset += sizeof(short);
	get2b(&maxProfile.maxComponentDepth, Blob.data() + offset);					offset += sizeof(short);

}
void TTFFile::parse(NameTable &nameTable, QByteArray Blob, unsigned int offset){
	uint32_t offsetStart = offset;
	get2b(&nameTable.format, Blob.data() + offset);								offset += sizeof(short);
	get2b(&nameTable.count, Blob.data() + offset);								offset += sizeof(short);
	get2b(&nameTable.stringOffset, Blob.data() + offset);						offset += sizeof(short);
	nameTable.nameRecord.resize(nameTable.count);
	for (auto i = 0; i < nameTable.count; i++) {
		if (nameTable.nameRecord[i].nameID > MAXIMUM_NAMES_NUMBER)
			continue;
		get2b(&nameTable.nameRecord[i].platformID, Blob.data() + offset);		offset += sizeof(short);
		get2b(&nameTable.nameRecord[i].encodingID, Blob.data() + offset);		offset += sizeof(short);
		get2b(&nameTable.nameRecord[i].languageID, Blob.data() + offset);		offset += sizeof(short);
		get2b(&nameTable.nameRecord[i].nameID, Blob.data() + offset);			offset += sizeof(short);
		get2b(&nameTable.nameRecord[i].length, Blob.data() + offset);			offset += sizeof(short);
		get2b(&nameTable.nameRecord[i].offset_value, Blob.data() + offset);		offset += sizeof(short);
		unsigned short stringLength = nameTable.nameRecord[i].length;

		QString newName;
		newName = Blob.mid(offsetStart + nameTable.stringOffset + nameTable.nameRecord[i].offset_value, sizeof(char)* stringLength);
		//memcpy(newStringName, Blob.data() + offsetStart + nameTable.stringOffset + nameTable.nameRecord[i].offset_value, sizeof(char)* stringLength);
		

		// in case name is written like: a r i a l   n a r r o w
		if (newName[0] == 0) {
			stringLength = stringLength >> 1;
			for (unsigned short j = 0; j < stringLength; j++) {
				newName[j] = newName[j * 2 + 1];
			}
		}
		namesList[nameTable.nameRecord[i].nameID] = newName;

	}
}