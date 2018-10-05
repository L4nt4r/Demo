# pragma once
#include <stdint.h>
#include <QtWidgets>

#define MAXIMUM_NAMES_NUMBER 25

struct Flags {
	bool xDual;
	bool yDual;
	bool xShort;
	bool yShort;
	bool repeat;
	bool offCurve;
};

enum COMPOUND_GLYPH_FLAGS {
	ARG_1_AND_2_ARE_WORDS = 0x0001,
	ARGS_ARE_XY_VALUES = 0x0002,
	ROUND_XY_TO_GRID = 0x0004,
	WE_HAVE_A_SCALE = 0x0008,
	MORE_COMPONENTS = 0x0020,
	WE_HAVE_AN_X_AND_Y_SCALE = 0x0040,
	WE_HAVE_A_TWO_BY_TWO = 0x0080,
	WE_HAVE_INSTRUCTIONS = 0x0100,
	USE_MY_METRICS = 0x0200,
	OVERLAP_COMPOUND = 0x0400,
	SCALED_COMPONENT_OFFSET = 0x0800,
	UNSCALED_COMPONENT_OFFSET = 0x1000
};
struct TTFHeader
{
	unsigned int version;
	unsigned short numTables;
	unsigned short searchRange;
	unsigned short entrySelector;
	unsigned short rangeShift;
};
struct TableEntry
{
	unsigned int tag;
	char tagstr[5];
	unsigned int checkSum;
	unsigned int offsetPos;
	unsigned int length;

};
struct HeadTable
{
	float tableVersion;
	float fontRevision;
	unsigned int checkSumAdjustment;
	unsigned int magicNumber;//0x5F0F3CF5
	unsigned short flags;
	unsigned short unitsPerEm;
	long long createdDate;
	long long modifiedData;
	short xMin;
	short yMin;
	short xMax;
	short yMax;
	unsigned short macStyle;
	unsigned short lowestRecPPEM;
	short fontDirectionHintl;
	short indexToLocFormat;
	short glyphDataFormat;

};
struct MaximumProfile
{
	float version;
	unsigned short numGlyphs;
	unsigned short maxPoints;
	unsigned short maxContours;
	unsigned short maxCompositePoints;
	unsigned short maxCompositeContours;
	unsigned short maxZones;
	unsigned short maxTwilightPoints;
	unsigned short maxStorage;
	unsigned short maxFunctionDefs;
	unsigned short maxInstructionDefs;
	unsigned short maxStackElements;
	unsigned short maxSizeOfInstructions;
	unsigned short maxComponentElements;
	unsigned short maxComponentDepth;

};
struct NameValue {
	unsigned short platformID;
	unsigned short encodingID;
	unsigned short languageID;
	unsigned short nameID;
	unsigned short length;
	unsigned short offset_value;

};
struct NameTable {
	unsigned short format;
	unsigned short count;
	unsigned short stringOffset;
	QVector<NameValue> nameRecord;
};
struct HHEATable {
	unsigned short majorVersion;
	unsigned short minorVersion;
	short	Ascender;
	short	Descender;
	short	LineGap;
	unsigned short advanceWidthMax;
	short	minLeftSideBearing;
	short	minRightSideBearing;
	short	xMaxExtent;
	short	caretSlopeRise;
	short	caretSlopeRun;
	short	caretOffset;
	short	metricDataFormat;
	unsigned short numberOfHMetrics;

};
struct Curve
{
	QPointF p0;
	QPointF p1;//Bezier control point or random off glyph point
	QPointF p2;
	bool is_curve;
};

struct Path {
	QVector<Curve> curves;
};

struct Glyph {
	unsigned int character;
	short glyph_index;
	short num_contours;
	QVector<Path> path_list;
	unsigned short advance_width;
	short left_side_bearing;
	short bounding_box[4];
	unsigned int num_triangles;
};

struct FontMetaData {
	unsigned short unitsPerEm;
	short Ascender;
	short Descender;
	short LineGap;
};

class TTFFile {
public:
	TTFFile() { endian_tested = false, get4b = &get4b_little_endian, get2b = &get2b_little_endian, get8b = &get8b_little_endian; }
	typedef  void(*MemoryCopyFunction)(void*, const char*);
	int parse(const char* file_name, /*TTFFontParser::FontData* font_data, TTFFontParser::TTF_FONT_PARSER_CALLBACK callback,*/ void* args);
	int parse_data(QByteArray Blob);
	
	inline float to_2_14_float(unsigned short value)
	{
		return (float(value & 0x3fff) / float(1 << 14)) + (-2 * ((value >> 15) & 0x1) + ((value >> 14) & 0x1));
	}
	
	MemoryCopyFunction get2b;
	MemoryCopyFunction get4b;
	MemoryCopyFunction get8b;
	inline void get1b(void* dst, const char* src){
		reinterpret_cast<unsigned char*>(dst)[0] = src[0];
	}

private:
	bool endian_tested;
	static inline void get4b_little_endian(void* dst, const char* src){
		reinterpret_cast<unsigned char*>(dst)[0] = src[3];
		reinterpret_cast<unsigned char*>(dst)[1] = src[2];
		reinterpret_cast<unsigned char*>(dst)[2] = src[1];
		reinterpret_cast<unsigned char*>(dst)[3] = src[0];
	}
	static inline void get4b_big_endian(void* dst, const char* src){
		reinterpret_cast<unsigned char*>(dst)[0] = src[0];
		reinterpret_cast<unsigned char*>(dst)[1] = src[1];
		reinterpret_cast<unsigned char*>(dst)[2] = src[2];
		reinterpret_cast<unsigned char*>(dst)[3] = src[3];
	}
	static inline void get8b_little_endian(void* dst, const char* src){
		reinterpret_cast<unsigned char*>(dst)[0] = src[7];
		reinterpret_cast<unsigned char*>(dst)[1] = src[6];
		reinterpret_cast<unsigned char*>(dst)[2] = src[5];
		reinterpret_cast<unsigned char*>(dst)[3] = src[4];
		reinterpret_cast<unsigned char*>(dst)[4] = src[3];
		reinterpret_cast<unsigned char*>(dst)[5] = src[2];
		reinterpret_cast<unsigned char*>(dst)[6] = src[1];
		reinterpret_cast<unsigned char*>(dst)[7] = src[0];
	}
	static inline void get8b_big_endian(void* dst, const char* src){
		reinterpret_cast<unsigned char*>(dst)[0] = src[0];
		reinterpret_cast<unsigned char*>(dst)[1] = src[1];
		reinterpret_cast<unsigned char*>(dst)[2] = src[2];
		reinterpret_cast<unsigned char*>(dst)[3] = src[3];
		reinterpret_cast<unsigned char*>(dst)[4] = src[4];
		reinterpret_cast<unsigned char*>(dst)[5] = src[5];
		reinterpret_cast<unsigned char*>(dst)[6] = src[6];
		reinterpret_cast<unsigned char*>(dst)[7] = src[7];
	}
	static inline void get2b_little_endian(void* dst, const char* src){
		reinterpret_cast<unsigned char*>(dst)[0] = src[1];
		reinterpret_cast<unsigned char*>(dst)[1] = src[0];
	}
	static inline void get2b_big_endian(void* dst, const char* src){
		reinterpret_cast<unsigned char*>(dst)[0] = src[0];
		reinterpret_cast<unsigned char*>(dst)[1] = src[1];
	}
protected:
	void parse(TTFHeader &header, QByteArray Blob);
	void parse(HeadTable &headTable , QByteArray Blob, unsigned int offset);
	void parse(MaximumProfile &maxProfile, QByteArray Blob, unsigned int offset);
	void parse(NameTable &nameTable, QByteArray Blob, unsigned int offset);

	QHash<char[5], TableEntry> tableMap;
	unsigned int fileNameHash;
	QString fullFontName;
	QString namesList[MAXIMUM_NAMES_NUMBER];
	QHash<unsigned int, unsigned short> kearningTable;
	QHash<unsigned short, Glyph> glyphs;
	QHash<unsigned int, unsigned short> glyphMap;
	FontMetaData meta_data;
	unsigned long long lastUsed;

};

