/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   guiBack_1_jpg;
    const int            guiBack_1_jpgSize = 22133;

    extern const char*   ambience_wav;
    const int            ambience_wavSize = 1590034;

    extern const char*   hall_wav;
    const int            hall_wavSize = 1763410;

    extern const char*   hall2_wav;
    const int            hall2_wavSize = 1381798;

    extern const char*   nonlin_wav;
    const int            nonlin_wavSize = 1523986;

    extern const char*   plate_wav;
    const int            plate_wavSize = 1975258;

    extern const char*   plate2_wav;
    const int            plate2_wavSize = 1466194;

    extern const char*   reversed_wav;
    const int            reversed_wavSize = 1548754;

    extern const char*   room_wav;
    const int            room_wavSize = 1638190;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 9;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
