// DTDefs.h: error definitions.

// Code by Karel Donk
// Contact me at karel@miraesoft.com for more information
// May be modified and used freely

#pragma once

// Function return values
#define DT_OK						1 // Function was successful
#define DT_ERROR					0 // Function was not successful
#define DT_NOT_INITIALIZED			2 // Object has not yet been initialized
#define DT_COULD_NOT_OPEN_FILE		3 // File could not be found or opened
#define DT_FILE_READ_ERROR			4 // Could not read from file
#define DT_FILE_WRITE_ERROR		5 // Could not write to file
#define DT_COMPRESSION_ERROR		6 // Compression error
#define DT_DECOMPRESSION_ERROR		7 // Decompresison error
#define DT_INVALID_FORMAT			8 // Invalid data format

