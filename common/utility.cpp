/* utility.cpp
 
 Copyright (c) 2016, Nikolaj Schlej. All rights reserved.
 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
 */

#include <cstdio>
#include <cctype>
#include <cstring>

#include "treemodel.h"
#include "utility.h"
#include "ffs.h"
#include "Tiano/EfiTianoCompress.h"
#include "Tiano/EfiTianoDecompress.h"
#include "LZMA/LzmaCompress.h"
#include "LZMA/LzmaDecompress.h"

// Returns bytes as string when all bytes are ascii visible, hex representation otherwise
UString visibleAsciiOrHex(UINT8* bytes, UINT32 length)
{
    bool ascii = true;
    UString asciiString;
    UString hexString;
    
    for (UINT32 i = 0; i < length; i++) {
        hexString += usprintf("%02X", bytes[i]);
        
        if (ascii && i > 0 && bytes[i] == '\x00') { // Check for the rest of the buffer being zeroes, and make the whole previous string visible, if so
            for (UINT32 j = i + 1; j < length; j++) {
                if (bytes[j] != '\x00') {
                    ascii = false;
                    break;
                }
            }
            
            if (ascii) {
                // No need to continue iterating over every symbol, we did it already
                break;
            }
        }
        else if (bytes[i] < '\x20' || bytes[i] > '\x7E') {  // Explicit ascii codes to avoid locale dependency
            ascii = false;
        }
        
        if (ascii) {
            asciiString += usprintf("%c", bytes[i]);
        }
    }
    
    if (ascii) {
        return asciiString;
    }
    
    return hexString;
}

// Returns unique name string based for tree item
UString uniqueItemName(const UModelIndex & index)
{
    // Sanity check
    if (!index.isValid())
        return UString("InvalidIndex");
    
    // Get model from index
    const TreeModel* model = (const TreeModel*)index.model();
    
    // Construct the name
    UString itemName = model->name(index);
    UString itemText = model->text(index);
    
    // Default name
    UString name = itemName;
    switch (model->type(index)) {
        case Types::NvarEntry:
        case Types::VssEntry:
        case Types::SysFEntry:
        case Types::EvsaEntry:
        case Types::PhoenixFlashMapEntry:
        case Types::InsydeFlashDeviceMapEntry:
        case Types::File:
            name = itemText.isEmpty() ? itemName : itemName + '_' + itemText;
            break;
        case Types::Section: {
            // Get parent file name
            UModelIndex fileIndex = model->findParentOfType(index, Types::File);
            UString fileText = model->text(fileIndex);
            name = fileText.isEmpty() ? model->name(fileIndex) : model->name(fileIndex) + '_' + fileText;
            
            // Special case of GUIDed sections
            if (model->subtype(index) == EFI_SECTION_GUID_DEFINED || model->subtype(index) == EFI_SECTION_FREEFORM_SUBTYPE_GUID) {
                name = model->name(index) +'_' + name;
            }
        } break;
    }
    
    // Populate subtypeString
    UString subtypeString = itemSubtypeToUString(model->type(index), model->subtype(index));
    
    // Create final name
    name = itemTypeToUString(model->type(index))
    + (subtypeString.length() ? ('_' + subtypeString) : UString())
    + '_' + name;
    
    fixFileName(name, true);
    
    return name;
}

// Makes the name usable as a file name
void fixFileName(UString &name, bool replaceSpaces)
{
    // Replace some symbols with underscores for compatibility
    const char table[] = {
        '/', // Banned in *nix and Windows
        '<', '>', ':', '\"', '\\', '|', '?', '*', // Banned in Windows
    };
    int nameLength = (int)name.length(); // Note: Qt uses int for whatever reason.
    for (int i = 0; i < nameLength; i++) {
        if (
            name[i] < (char)0x20 || // ASCII control characters, banned in Windows, hard to work with in *nix
            name[i] > (char)0x7f || // high ASCII characters
            (replaceSpaces && name[i] == ' ') // Provides better readability
            ) {
                name[i] = '_';
                continue;
            }
        for (size_t j = 0; j < sizeof(table); j++) {
            if (name[i] == table[j]) {
                name[i] = '_';
                break;
            }
        }
    }
    if (!nameLength) {
        name = "_";
    }
}

// Returns text representation of error code
UString errorCodeToUString(USTATUS errorCode)
{
    // TODO: improve
    switch (errorCode) {
        case U_SUCCESS:                         return UString("Success");
        case U_NOT_IMPLEMENTED:                 return UString("Not implemented");
        case U_INVALID_PARAMETER:               return UString("Function called with invalid parameter");
        case U_BUFFER_TOO_SMALL:                return UString("Buffer too small");
        case U_OUT_OF_RESOURCES:                return UString("Out of resources");
        case U_OUT_OF_MEMORY:                   return UString("Out of memory");
        case U_FILE_OPEN:                       return UString("File can't be opened");
        case U_FILE_READ:                       return UString("File can't be read");
        case U_FILE_WRITE:                      return UString("File can't be written");
        case U_ITEM_NOT_FOUND:                  return UString("Item not found");
        case U_UNKNOWN_ITEM_TYPE:               return UString("Unknown item type");
        case U_INVALID_FLASH_DESCRIPTOR:        return UString("Invalid flash descriptor");
        case U_INVALID_REGION:                  return UString("Invalid region");
        case U_EMPTY_REGION:                    return UString("Empty region");
        case U_BIOS_REGION_NOT_FOUND:           return UString("BIOS region not found");
        case U_VOLUMES_NOT_FOUND:               return UString("UEFI volumes not found");
        case U_INVALID_VOLUME:                  return UString("Invalid UEFI volume");
        case U_VOLUME_REVISION_NOT_SUPPORTED:   return UString("Volume revision not supported");
        case U_UNKNOWN_FFS:                     return UString("Unknown file system");
        case U_INVALID_FILE:                    return UString("Invalid file");
        case U_INVALID_SECTION:                 return UString("Invalid section");
        case U_UNKNOWN_SECTION:                 return UString("Unknown section");
        case U_STANDARD_COMPRESSION_FAILED:     return UString("Standard compression failed");
        case U_CUSTOMIZED_COMPRESSION_FAILED:   return UString("Customized compression failed");
        case U_STANDARD_DECOMPRESSION_FAILED:   return UString("Standard decompression failed");
        case U_CUSTOMIZED_DECOMPRESSION_FAILED: return UString("Customized decompression failed");
        case U_UNKNOWN_COMPRESSION_TYPE:        return UString("Unknown compression type");
        case U_UNKNOWN_EXTRACT_MODE:            return UString("Unknown extract mode");
        case U_UNKNOWN_REPLACE_MODE:            return UString("Unknown replace mode");
        case U_UNKNOWN_IMAGE_TYPE:              return UString("Unknown executable image type");
        case U_UNKNOWN_PE_OPTIONAL_HEADER_TYPE: return UString("Unknown PE optional header type");
        case U_UNKNOWN_RELOCATION_TYPE:         return UString("Unknown relocation type");
        case U_COMPLEX_BLOCK_MAP:               return UString("Block map structure too complex for correct analysis");
        case U_DIR_ALREADY_EXIST:               return UString("Directory already exists");
        case U_DIR_CREATE:                      return UString("Directory can't be created");
        case U_DIR_CHANGE:                      return UString("Change directory failed");
        case U_DEPEX_PARSE_FAILED:              return UString("Dependency expression parsing failed");
        case U_TRUNCATED_IMAGE:                 return UString("Image is truncated");
        case U_INVALID_CAPSULE:                 return UString("Invalid capsule");
        case U_STORES_NOT_FOUND:                return UString("Stores not found");
        case U_INVALID_STORE_SIZE:              return UString("Invalid store size");
        case U_INVALID_STORE:                   return UString("Invalid store");
        default:                                return usprintf("Unknown error %02lX", errorCode);
    }
}

// Compression routines
USTATUS decompress(const UByteArray & compressedData, const UINT8 compressionType, UINT8 & algorithm, UINT32 & dictionarySize, UByteArray & decompressedData, UByteArray & efiDecompressedData)
{
    const UINT8* data;
    UINT32 dataSize;
    UINT8* decompressed;
    UINT8* efiDecompressed;
    UINT32 decompressedSize = 0;
    UINT8* scratch;
    UINT32 scratchSize = 0;
    const EFI_TIANO_HEADER* header;
    
    // For all but LZMA dictionary size is 0
    dictionarySize = 0;
    
    switch (compressionType)
    {
        case EFI_NOT_COMPRESSED: {
            decompressedData = compressedData;
            algorithm = COMPRESSION_ALGORITHM_NONE;
            return U_SUCCESS;
        }
        case EFI_STANDARD_COMPRESSION: {
            // Set default algorithm to unknown
            algorithm = COMPRESSION_ALGORITHM_UNKNOWN;
            
            // Get buffer sizes
            data = (UINT8*)compressedData.data();
            dataSize = (UINT32)compressedData.size();
            
            // Check header to be valid
            header = (const EFI_TIANO_HEADER*)data;
            if (header->CompSize + sizeof(EFI_TIANO_HEADER) != dataSize)
                return U_STANDARD_DECOMPRESSION_FAILED;
            
            // Get info function is the same for both algorithms
            if (U_SUCCESS != EfiTianoGetInfo(data, dataSize, &decompressedSize, &scratchSize))
                return U_STANDARD_DECOMPRESSION_FAILED;
            
            // Allocate memory
            decompressed = (UINT8*)malloc(decompressedSize);
            efiDecompressed = (UINT8*)malloc(decompressedSize);
            scratch = (UINT8*)malloc(scratchSize);
            if (!decompressed || !efiDecompressed || !scratch) {
                free(decompressed);
                free(efiDecompressed);
                free(scratch);
                return U_STANDARD_DECOMPRESSION_FAILED;
            }
            
            // Decompress section data using both algorithms
            USTATUS result = U_SUCCESS;
            // Try Tiano
            USTATUS TianoResult = TianoDecompress(data, dataSize, decompressed, decompressedSize, scratch, scratchSize);
            // Try EFI 1.1
            USTATUS EfiResult = EfiDecompress(data, dataSize, efiDecompressed, decompressedSize, scratch, scratchSize);
            
            if (decompressedSize > INT32_MAX) {
                result = U_STANDARD_DECOMPRESSION_FAILED;
            }
            else if (EfiResult == U_SUCCESS && TianoResult == U_SUCCESS) { // Both decompressions are OK
                algorithm = COMPRESSION_ALGORITHM_UNDECIDED;
                decompressedData = UByteArray((const char*)decompressed, (int)decompressedSize);
                efiDecompressedData = UByteArray((const char*)efiDecompressed, (int)decompressedSize);
            }
            else if (TianoResult == U_SUCCESS) { // Only Tiano is OK
                algorithm = COMPRESSION_ALGORITHM_TIANO;
                decompressedData = UByteArray((const char*)decompressed, (int)decompressedSize);
            }
            else if (EfiResult == U_SUCCESS) { // Only EFI 1.1 is OK
                algorithm = COMPRESSION_ALGORITHM_EFI11;
                decompressedData = UByteArray((const char*)efiDecompressed, (int)decompressedSize);
            }
            else { // Both decompressions failed
                result = U_STANDARD_DECOMPRESSION_FAILED;
            }
            
            free(decompressed);
            free(efiDecompressed);
            free(scratch);
            return result;
        }
        case EFI_CUSTOMIZED_COMPRESSION: {
            // Set default algorithm to unknown
            algorithm = COMPRESSION_ALGORITHM_UNKNOWN;
            
            // Get buffer sizes
            data = (const UINT8*)compressedData.constData();
            dataSize = (UINT32)compressedData.size();
            
            // Get info as normal LZMA section
            if (U_SUCCESS != LzmaGetInfo(data, dataSize, &decompressedSize)) {
                // Get info as Intel legacy LZMA section
                data += sizeof(UINT32);
                if (U_SUCCESS != LzmaGetInfo(data, dataSize, &decompressedSize)) {
                    return U_CUSTOMIZED_DECOMPRESSION_FAILED;
                }
                else {
                    algorithm = COMPRESSION_ALGORITHM_LZMA_INTEL_LEGACY;
                }
            }
            else {
                algorithm = COMPRESSION_ALGORITHM_LZMA;
            }
            
            // Allocate memory
            decompressed = (UINT8*)malloc(decompressedSize);
            if (!decompressed) {
                return U_OUT_OF_MEMORY;
            }
            
            // Decompress section data
            if (U_SUCCESS != LzmaDecompress(data, dataSize, decompressed)) {
                free(decompressed);
                return U_CUSTOMIZED_DECOMPRESSION_FAILED;
            }
            
            if (decompressedSize > INT32_MAX) {
                free(decompressed);
                return U_CUSTOMIZED_DECOMPRESSION_FAILED;
            }
            
            dictionarySize = readUnaligned((UINT32*)(data + 1)); // LZMA dictionary size is stored in bytes 1-4 of LZMA properties header
            decompressedData = UByteArray((const char*)decompressed, (int)decompressedSize);
            free(decompressed);
            return U_SUCCESS;
        }
        case EFI_CUSTOMIZED_COMPRESSION_LZMAF86: {
            // Set default algorithm to unknown
            algorithm = COMPRESSION_ALGORITHM_UNKNOWN;
            
            // Get buffer sizes
            data = (const UINT8*)compressedData.constData();
            dataSize = (UINT32)compressedData.size();
            
            // Get info as normal LZMA section
            if (U_SUCCESS != LzmaGetInfo(data, dataSize, &decompressedSize)) {
                return U_CUSTOMIZED_DECOMPRESSION_FAILED;
            }
            algorithm = COMPRESSION_ALGORITHM_LZMAF86;
            
            // Allocate memory
            decompressed = (UINT8*)malloc(decompressedSize);
            if (!decompressed) {
                return U_OUT_OF_MEMORY;
            }
            
            // Decompress section data
            if (U_SUCCESS != LzmaDecompress(data, dataSize, decompressed)) {
                free(decompressed);
                return U_CUSTOMIZED_DECOMPRESSION_FAILED;
            }
            
            if (decompressedSize > INT32_MAX) {
                free(decompressed);
                return U_CUSTOMIZED_DECOMPRESSION_FAILED;
            }
            
            // TODO: need to correctly handle non-x86 architecture of the FW image
            // After LZMA decompression, the data need to be converted to the raw data.
            UINT32 state = 0;
            z7_BranchConvSt_X86_Dec(decompressed, decompressedSize, 0, &state);
            
            dictionarySize = readUnaligned((UINT32*)(data + 1)); // LZMA dictionary size is stored in bytes 1-4 of LZMA properties header
            decompressedData = UByteArray((const char*)decompressed, (int)decompressedSize);
            free(decompressed);
            return U_SUCCESS;
        }
        default: {
            algorithm = COMPRESSION_ALGORITHM_UNKNOWN;
            return U_UNKNOWN_COMPRESSION_TYPE;
        }
    }
}

// 8bit sum calculation routine
UINT8 calculateSum8(const UINT8* buffer, UINT32 bufferSize)
{
    if (!buffer)
        return 0;
    
    UINT8 counter = 0;
    
    while (bufferSize--)
        counter += buffer[bufferSize];
    
    return counter;
}

// 8bit checksum calculation routine
UINT8 calculateChecksum8(const UINT8* buffer, UINT32 bufferSize)
{
    if (!buffer)
        return 0;
    
    return (UINT8)(0x100U - calculateSum8(buffer, bufferSize));
}

// 16bit checksum calculation routine
UINT16 calculateChecksum16(const UINT16* buffer, UINT32 bufferSize)
{
    if (!buffer)
        return 0;
    
    UINT16 counter = 0;
    UINT32 index = 0;
    
    bufferSize /= sizeof(UINT16);
    
    for (; index < bufferSize; index++) {
        counter = (UINT16)(counter + buffer[index]);
    }
    
    return (UINT16)(0x10000 - counter);
}

// 32bit checksum calculation routine
UINT32 calculateChecksum32(const UINT32* buffer, UINT32 bufferSize)
{
    if (!buffer)
        return 0;
    
    UINT32 counter = 0;
    UINT32 index = 0;
    
    bufferSize /= sizeof(UINT32);
    
    for (; index < bufferSize; index++) {
        counter = (UINT32)(counter + buffer[index]);
    }
    
    return (UINT32)(0x100000000ULL - counter);
}

// Get padding type for a given padding
UINT8 getPaddingType(const UByteArray & padding)
{
    if (padding.count('\x00') == padding.size())
        return Subtypes::ZeroPadding;
    if (padding.count('\xFF') == padding.size())
        return Subtypes::OnePadding;
    return Subtypes::DataPadding;
}

static inline int char2hex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c == '.')
        return -2;
    return -1;
}

INTN findPattern(const UINT8 *pattern, const UINT8 *patternMask, UINTN patternSize,
                 const UINT8 *data, UINTN dataSize, UINTN dataOff)
{
    if (patternSize == 0 || dataSize == 0 || dataOff >= dataSize || dataSize - dataOff < patternSize)
        return -1;
    
    while (dataOff + patternSize <= dataSize) {
        bool matches = true;
        for (UINTN i = 0; i < patternSize; i++) {
            if ((data[dataOff + i] & patternMask[i]) != pattern[i]) {
                matches = false;
                break;
            }
        }
        
        if (matches)
            return static_cast<INTN>(dataOff);
        
        dataOff++;
    }
    
    return -1;
}

bool makePattern(const CHAR8 *textPattern, std::vector<UINT8> &pattern, std::vector<UINT8> &patternMask)
{
    UINTN len = std::strlen(textPattern);
    
    if (len == 0 || len % 2 != 0)
        return false;
    
    len /= 2;
    
    pattern.resize(len);
    patternMask.resize(len);
    
    for (UINTN i = 0; i < len; i++) {
        int v1 = char2hex(std::toupper(textPattern[i * 2]));
        int v2 = char2hex(std::toupper(textPattern[i * 2 + 1]));
        
        if (v1 == -1 || v2 == -1)
            return false;
        
        if (v1 != -2) {
            patternMask[i] = 0xF0;
            pattern[i] = static_cast<UINT8>(v1) << 4;
        }
        
        if (v2 != -2) {
            patternMask[i] |= 0x0F;
            pattern[i] |= static_cast<UINT8>(v2);
        }
    }
    
    return true;
}

USTATUS gzipDecompress(const UByteArray & input, UByteArray & output)
{
    output.clear();
    
    if (input.size() == 0)
        return U_SUCCESS;
    
    z_stream stream = {};
    stream.next_in = (z_const Bytef *)input.data();
    stream.avail_in = (uInt)input.size();
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    
    // 15 for the maximum history buffer, 16 for gzip only input
    int ret = inflateInit2(&stream, 15U | 16U);
    if (ret != Z_OK)
        return U_GZIP_DECOMPRESSION_FAILED;
    
    while (ret == Z_OK) {
        Bytef out[0x1000] = {};
        stream.next_out = out;
        stream.avail_out = sizeof(out);
        
        ret = inflate(&stream, Z_NO_FLUSH);
        if ((ret == Z_OK || ret == Z_STREAM_END) && stream.avail_out != sizeof(out))
            output += UByteArray((char *)out, sizeof(out) - stream.avail_out);
    }
    
    inflateEnd(&stream);
    return ret == Z_STREAM_END ? U_SUCCESS : U_GZIP_DECOMPRESSION_FAILED;
}

USTATUS zlibDecompress(const UByteArray& input, UByteArray& output)
{
    output.clear();

    if (input.size() == 0)
        return U_SUCCESS;

    z_stream stream = {};
    stream.next_in = (z_const Bytef*)input.data();
    stream.avail_in = (uInt)input.size();
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    // 15 for the maximum history buffer
    int ret = inflateInit2(&stream, 15U);
    if (ret != Z_OK)
        return U_ZLIB_DECOMPRESSION_FAILED;

    while (ret == Z_OK) {
        Bytef out[0x1000] = {};
        stream.next_out = out;
        stream.avail_out = sizeof(out);

        ret = inflate(&stream, Z_NO_FLUSH);
        if ((ret == Z_OK || ret == Z_STREAM_END) && stream.avail_out != sizeof(out))
            output += UByteArray((char*)out, sizeof(out) - stream.avail_out);
    }

    inflateEnd(&stream);
    return ret == Z_STREAM_END ? U_SUCCESS : U_ZLIB_DECOMPRESSION_FAILED;
}

UString fourCC(const UINT32 value)
{
    const UINT8 byte0 = (const UINT8)(value & 0xFF);
    const UINT8 byte1 = (const UINT8)((value & 0xFF00) >> 8);
    const UINT8 byte2 = (const UINT8)((value & 0xFF0000) >> 16);
    const UINT8 byte3 = (const UINT8)((value & 0xFF000000) >> 24);
    return usprintf("%c%c%c%c", byte0, byte1, byte2, byte3);
}
