/* utility.h

Copyright (c) 2016, Nikolaj Schlej. All rights reserved.
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/

#ifndef UTILITY_H
#define UTILITY_H

#include <vector>

#include "../common/zlib/zlib.h"

#include "basetypes.h"
#include "ustring.h"
#include "treemodel.h"
#include "parsingdata.h"

// Returns text representation of 4CC value
UString fourCC(const UINT32 value);

// Returns bytes as string when all bytes are ascii visible, hex representation otherwise
UString visibleAsciiOrHex(UINT8* bytes, UINT32 length);

// Returns unique name for tree item
UString uniqueItemName(const UModelIndex & index);

// Makes the name usable as a file name
void fixFileName(UString &name, bool replaceSpaces);

// Converts error code to UString
UString errorCodeToUString(USTATUS errorCode);

// EFI/Tiano/LZMA decompression routine
USTATUS decompress(const UByteArray & compressed, const UINT8 compressionType, UINT8 & algorithm, UINT32 & dictionarySize, UByteArray & decompressed, UByteArray & efiDecompressed);

// GZIP decompression routine
USTATUS gzipDecompress(const UByteArray & compressed, UByteArray & decompressed);

// ZLIB decompression routine
USTATUS zlibDecompress(const UByteArray& compressed, UByteArray& decompressed);

// 8bit sum calculation routine
UINT8 calculateSum8(const UINT8* buffer, UINT32 bufferSize);

// 8bit checksum calculation routine
UINT8 calculateChecksum8(const UINT8* buffer, UINT32 bufferSize);

// 16bit checksum calculation routine
UINT16 calculateChecksum16(const UINT16* buffer, UINT32 bufferSize);

// 32bit checksum calculation routine
UINT32 calculateChecksum32(const UINT32* buffer, UINT32 bufferSize);

// Return padding type from it's contents
UINT8 getPaddingType(const UByteArray & padding);

// Make pattern from a hexstring with an assumption of . being any char
bool makePattern(const CHAR8 *textPattern, std::vector<UINT8> &pattern, std::vector<UINT8> &patternMask);

// Find pattern in a binary blob
INTN findPattern(const UINT8 *pattern, const UINT8 *patternMask, UINTN patternSize,
    const UINT8 *data, UINTN dataSize, UINTN dataOff);

// Safely dereferences misaligned pointers
template <typename T>
inline T readUnaligned(const T *v) {
    T tmp = {};
    memcpy(reinterpret_cast<void*>(&tmp), reinterpret_cast<const void*>(v), sizeof(T));
    return tmp;
}

#endif // UTILITY_H
