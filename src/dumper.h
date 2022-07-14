#ifndef __DUMPER_H__
#define __DUMPER_H__

#include <stdint.h>
#include <vector>
#include <string>
#include "utils/StringTable.h"

namespace dirdump {
    struct File {
        StringTable::str name;

        uint32_t depth;
        uint64_t size;
    };
    struct Folder {
        uint64_t id;
        uint64_t parentID;

        StringTable::str path;

        std::vector<size_t> subfolders;
        std::vector<File> files;

        uint32_t depth = -1;
        uint64_t size = 0;

        bool error = false;

        inline Folder(StringTable::str path, uint64_t id, uint64_t parentID, uint32_t depth) : path(path), id(id), parentID(parentID), depth(depth) {

        }

        inline bool operator<(const Folder& rhs) {
            return id < rhs.id;
        }
    };

    struct FolderScanData {
        StringTable stable;
        std::vector<Folder> folders;
        size_t rootInd;
        uint32_t maxDepth;
    };
    FolderScanData getData();

    struct FolderScanDataEx {
        struct ExData {
            uint64_t totalOff;
            std::vector<uint64_t> filesTotalOff;

        };
        std::vector<ExData> ex;
        std::vector<std::vector<size_t>> depths;
    };
    FolderScanDataEx generateExtraData(const FolderScanData& scan);

	struct Stats {
		uint64_t files;
		uint64_t folders;
        uint64_t nanos;
	};
	Stats getStats();
	void startDump(const char* path);
	void update();

	bool isBusy();
}

#endif