#ifndef __HAND_HAND_TYPE__
#define __HAND_HAND_TYPE__

#include "vkcpp.h"
#ifdef _WIN32
#include <windows.h>
#else
#define _SVID_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
namespace hand
{
    class HandInput
    {
    private:
        bool is_setup_ = false;
        void *hand_landmarks_ = nullptr;
        float *p_data_ = nullptr;
#ifdef _WIN32
        void *file_map_ = nullptr;
        const int size_ = 512;
        char posix_map_name_[64];
#else
        const int size_ = 2048;
        const key_t key_ = 777;
        int shmid_;
#endif
        bool createMemoryMap();
        bool freeMemoryMap();
        void clear();

    public:
        HandInput();
        ~HandInput();
        const glm::vec3 getLandmarks(int idx) const;
    };

}
#endif