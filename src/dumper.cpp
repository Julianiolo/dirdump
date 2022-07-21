#include <dumper.h>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include <atomic>
#include <algorithm>
#include <chrono>

#define PATH_MAX 1024
#include "dirent.h"


template<typename T>
class ThreadQueue {
private:
    std::mutex mutex;
    std::queue<T> data;


    void waitLock() {
        while (!mutex.try_lock()) {

        }
    }
public:

    void pushSafe(const T& t) {
        waitLock();
        data.push(t);
        mutex.unlock();
    }

    void pushUnsafe(const T& t) {
        data.push(t);
    }

    T& front() {
        return data.front();
    }

    void pop() {
        data.pop();
    }

    bool empty() const {
        return data.empty();
    }

    bool emptyLockifNot() {
        waitLock();
        bool e = data.empty();
        if (e)
            mutex.unlock();
        return e;
    }

    void lock() {
        waitLock();
    }

    void unlock() {
        mutex.unlock();
    }
};


namespace dirdump {
    std::atomic<uint64_t> fileCnt = 0;
    std::atomic<uint64_t> folderCnt = 0;
    std::atomic<uint64_t> folderErrCnt = 0;

    struct WorkPacket {
        std::string path;
        uint64_t parentID;
        uint32_t depth;
    };

    bool workerActive = false;
    ThreadQueue<WorkPacket> workQueue;
    FolderScanData res;

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

    struct Worker {
        std::vector<Folder> folders;
        StringTable stable;
        uint8_t workerID;
        uint64_t idCnt = 0;
        bool nothingTodo = false;


        std::unique_ptr<std::thread> thread;

        Worker(uint8_t workerID) : folders(), workerID(workerID){

        }

        void start() {
            thread = std::make_unique<std::thread>(&Worker::run, this);
        }

        uint64_t genID() {
            return (idCnt++) | ((uint64_t)workerID << 48);
        }

        void scanFolder(const WorkPacket& wp) {
            uint64_t folderID = genID();
            folders.push_back(Folder(stable.addStr(wp.path.c_str()), folderID, wp.parentID, wp.depth));
            Folder& folder = folders.back();

            std::vector<WorkPacket> toScan;

            DIR *dir = opendir(wp.path.c_str());
            if (!dir) {
                folder.error = true;
                folderErrCnt++;
                return;
            }

            size_t size = 0;
            while (true) {
                dirent entry;
                dirent *entryPtr = &entry;
                if (readdir_r(dir, entryPtr, &entryPtr) != /*OK*/0) {
                    folder.error = true;
                    break;
                }

                /* Stop if we already read the last directory entry */
                if (entryPtr == NULL)
                    break;

                if (entry.d_name[0] == '.' && (entry.d_name[1] == 0 || (entry.d_name[1] == '.' && entry.d_name[2] == 0)))
                    continue;

                std::string entryPath = wp.path + entry.d_name;

#if 0
                StringTable::str entryPath = stable.addStr(stable.getStr(wp.path), 0, false);
                stable.addStr("/", 0, false);
                StringTable::str onlyName = stable.addStr(entry.d_name);
#endif

                if (entry.d_type == DT_DIR) {
                    toScan.push_back({ entryPath + "/", folderID, wp.depth + 1});
                    folderCnt++;
                }
                else {
                    folder.files.push_back(File());
                    File& file = folder.files.back();

                    file.name = stable.addStr(entry.d_name);

                    
#ifdef _MSC_VER
                    struct _stat64 st;
                    int ret = __stat64(entryPath.c_str(), &st);
#else
                    struct stat st;
                    int ret = stat(entryPath.c_str(), &st);
#endif
                    if (ret == 0) {
                        uint64_t size = 0;
                        size = st.st_size;
                        file.size = size;
                        folder.size += size;
                    }
                    else {
                        int a = 0;
                    }
                    
                    fileCnt++;
                }
            }
            closedir(dir);

            if (toScan.size() > 0) {
                if (toScan.size() > 1) {
                    workQueue.lock();
                    for (size_t i = 1; i < toScan.size(); i++) {
                        workQueue.pushUnsafe(toScan[i]);
                    }
                    workQueue.unlock();
                }

                scanFolder(toScan[0]);
            }
        }

        void run() {
            while (workerActive) {
                if (!workQueue.emptyLockifNot()) {
                    nothingTodo = false;
                    WorkPacket work = workQueue.front();
                    workQueue.pop();
                    workQueue.unlock();

                    scanFolder(work);
                }
                else {
                    nothingTodo = true;
                }
            }
        }
    };

    std::vector<Worker> worker;

    size_t findFolderByID(const std::vector<Folder>& folders, uint64_t id) {
        size_t from = 0;
        size_t to = folders.size();

        while (from != to) {
            size_t mid = from + (to - from) / 2;
            uint64_t mid_id = folders[mid].id;

            if (mid_id == id) {
                return mid;
            }
            else if (mid_id < id) {
                if (mid == from)
                    return -1;
                from = mid;
            }
            else {
                if (mid == to)
                    return -1;
                to = mid;
            }
        }
        return -1;
    }
    

    uint64_t updateSize(std::vector<Folder>& folders, size_t ind) {
        for (size_t i = 0; i < folders[ind].subfolders.size(); i++) {
            folders[ind].size += updateSize(folders, folders[ind].subfolders[i]);
        }
        return folders[ind].size;
    }

    void processResult() {
        std::vector<Folder> folders;
        

        size_t size = 0;
        for (size_t i = 0; i < worker.size(); i++) {
            size += worker[i].folders.size();
        }

        folders.reserve(size);

        StringTable table;
        for (size_t i = 0; i < worker.size(); i++) {
            size_t off = table.addTable(worker[i].stable);

            if (off > 0) {
                for (size_t f = 0; f < worker[i].folders.size(); f++) {
                    worker[i].folders[f].path += off;

                    for (size_t j = 0; j < worker[i].folders[f].files.size(); j++) {
                        worker[i].folders[f].files[j].name += off;
                    }
                }
            }

            folders.insert(
                folders.end(),
                std::make_move_iterator(worker[i].folders.begin()),
                std::make_move_iterator(worker[i].folders.end())
            );
        }

        std::sort(folders.begin(), folders.end());

        uint64_t rootInd = -1;
        uint32_t maxDepth = 0;
        for (size_t i = 0; i < folders.size(); i++) {
            if (folders[i].parentID != (uint64_t)-1) {
                size_t parentInd = findFolderByID(folders, folders[i].parentID);
                folders[parentInd].subfolders.push_back(i);
            }
            else {
                rootInd = i;
            }

            if (folders[i].depth > maxDepth) {
                maxDepth = folders[i].depth;
            }
        }

        updateSize(folders,rootInd);


        res = FolderScanData{ table, folders, rootInd, maxDepth };
    }

    uint64_t updateTotalOff(const FolderScanData& scan, FolderScanDataEx& ex, size_t ind, uint64_t off) {
        const Folder& folder = scan.folders[ind];

        ex.ex[ind].totalOff = off;
        for (size_t i = 0; i < folder.subfolders.size(); i++) {
            off = updateTotalOff(scan, ex, folder.subfolders[i], off);
        }

        ex.ex[ind].filesTotalOff.resize(folder.files.size());
        for (size_t i = 0; i < scan.folders[ind].files.size(); i++) {
            ex.ex[ind].filesTotalOff[i] = off;
            off += folder.files[i].size;
        }
        return off;
    }
}

dirdump::FolderScanData dirdump::getData() {
    return res;
}


dirdump::FolderScanDataEx dirdump::generateExtraData(const FolderScanData& scan) {
    FolderScanDataEx res;
    res.depths.resize(scan.maxDepth+1);
    for (size_t i = 0; i < scan.folders.size(); i++) {
        res.depths[scan.folders[i].depth].push_back(i);
    }

    res.ex.resize(scan.folders.size());
    uint64_t off = updateTotalOff(scan, res, scan.rootInd, 0);

    return res;
}


void dirdump::startDump(const char* path, uint32_t numThreads) {
    start = std::chrono::high_resolution_clock::now();

    workerActive = true;

    res.folders.clear();
    res.maxDepth = -1;
    res.rootInd = -1;

    fileCnt = 0;
    folderCnt = 1;

    workQueue.pushUnsafe({path, (uint64_t)-1, 0});
    if (worker.size() == 0) {
        for (size_t i = 0; i < numThreads; i++) {
            worker.push_back(Worker((uint8_t)i));
        }
        for (size_t i = 0; i < worker.size(); i++) {
            worker[i].start();
        }
    }
}

void dirdump::update() {
    if (worker.size() > 0) { // => running
        workQueue.lock();
        bool stillWorking = false;
        for (size_t i = 0; i < worker.size(); i++) {
            if (!worker[i].nothingTodo) {
                stillWorking = true;
                break;
            }
        }
        workQueue.unlock();

        if (!stillWorking) {
            workerActive = false;
            for (size_t i = 0; i < worker.size(); i++) {
                worker[i].thread.get()->join();
            }

            processResult();

            worker.clear();

            end = std::chrono::high_resolution_clock::now();
        }
    }
}

dirdump::Stats dirdump::getStats() {
    Stats stats;
    stats.files = fileCnt;
    stats.folders = folderCnt;
    stats.folderErrs = folderErrCnt;
    stats.nanos = std::chrono::duration_cast<std::chrono::nanoseconds>( (workerActive ? std::chrono::high_resolution_clock::now() : end) - start).count();

    return stats;
}

bool dirdump::isBusy() {
    return worker.size() != 0;
}