CMAKE_MINIMUM_REQUIRED(VERSION 3.22)

PROJECT(UEFIFind)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

SET(PROJECT_SOURCES
 uefifind_main.cpp
 uefifind.cpp
 ../common/guiddatabase.cpp
 ../common/types.cpp
 ../common/filesystem.cpp
 ../common/descriptor.cpp
 ../common/ffs.cpp
 ../common/nvram.cpp
 ../common/nvramparser.cpp
 ../common/ffsparser.cpp
 ../common/fitparser.cpp
 ../common/peimage.cpp
 ../common/treeitem.cpp
 ../common/treemodel.cpp
 ../common/utility.cpp
 ../common/LZMA/LzmaDecompress.c
 ../common/LZMA/SDK/C/Bra.c
 ../common/LZMA/SDK/C/Bra86.c
 ../common/LZMA/SDK/C/CpuArch.c
 ../common/LZMA/SDK/C/LzmaDec.c
 ../common/Tiano/EfiTianoDecompress.c
 ../common/ustring.cpp
 ../common/bstrlib/bstrlib.c
 ../common/bstrlib/bstrwrap.cpp
 ../common/generated/ami_nvar.cpp
 ../common/generated/apple_sysf.cpp
 ../common/generated/edk2_vss.cpp
 ../common/generated/edk2_vss2.cpp
 ../common/generated/edk2_ftw.cpp
 ../common/generated/insyde_fdc.cpp
 ../common/generated/insyde_fdm.cpp
 ../common/generated/ms_slic_marker.cpp
 ../common/generated/ms_slic_pubkey.cpp
 ../common/generated/phoenix_flm.cpp
 ../common/generated/phoenix_evsa.cpp
 ../common/generated/intel_acbp_v1.cpp
 ../common/generated/intel_acbp_v2.cpp
 ../common/generated/intel_keym_v1.cpp
 ../common/generated/intel_keym_v2.cpp
 ../common/generated/intel_acm.cpp
 ../common/kaitai/kaitaistream.cpp
 ../common/digest/sha1.c
 ../common/digest/sha256.c
 ../common/digest/sha512.c
 ../common/digest/sm3.c
 ../common/zlib/adler32.c
 ../common/zlib/compress.c
 ../common/zlib/crc32.c
 ../common/zlib/deflate.c
 ../common/zlib/gzclose.c
 ../common/zlib/gzlib.c
 ../common/zlib/gzread.c
 ../common/zlib/gzwrite.c
 ../common/zlib/inflate.c
 ../common/zlib/infback.c
 ../common/zlib/inftrees.c
 ../common/zlib/inffast.c
 ../common/zlib/trees.c
 ../common/zlib/uncompr.c
 ../common/zlib/zutil.c
)

ADD_EXECUTABLE(UEFIFind ${PROJECT_SOURCES} uefifind.manifest)

IF(UNIX)
 SET_TARGET_PROPERTIES(UEFIFind PROPERTIES OUTPUT_NAME uefifind)
ENDIF()

INSTALL(
 TARGETS UEFIFind
 RUNTIME DESTINATION bin
)
