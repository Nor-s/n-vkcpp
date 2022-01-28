#include "hand_input.h"

namespace hand
{
    HandInput::HandInput()
    {
#ifdef _WIN32
        sprintf(posix_map_name_, "Local\\MPipe");

#endif
        if (createMemoryMap())
        {
        }

        p_data_ = reinterpret_cast<float *>(hand_landmarks_);
    }
    HandInput::~HandInput()
    {
        freeMemoryMap();
    }
    bool HandInput::createMemoryMap()
    {
#ifdef _WIN32
        if ((file_map_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size_, posix_map_name_)) == NULL)
        {
            return false;
        }
        if ((hand_landmarks_ = MapViewOfFile(file_map_, FILE_MAP_ALL_ACCESS, 0, 0, size_)) == NULL)
        {
            CloseHandle(file_map_);
            return false;
        }
#else
        if ((shmid_ = shmget(key_, 2048, IPC_CREAT | 0666)) < 0)
        {
            return false;
        }
        if ((hand_landmarks_ = shmat(shmid_, NULL, 0)) == (void *)-1)
        {
            return false;
        }
#endif
        return is_setup_ = true;
    }
    bool HandInput::freeMemoryMap()
    {
#ifdef _WIN32
        if (file_map_)
        {
            if (hand_landmarks_)
            {
                UnmapViewOfFile(hand_landmarks_);
            }

            if (file_map_)
            {
                CloseHandle(file_map_);
            }
        }
#else
        if (shmdt(hand_landmarks_) == -1)
        {
            return false;
        }
        if (shmctl(shmid_, IPC_RMID, NULL) == -1)
        {
            return false;
        }
#endif
        is_setup_ = false;
        return true;
    }
    void HandInput::clear()
    {
        memset(hand_landmarks_, 0, size_);
    }
    const glm::vec3 HandInput::getLandmarks(int idx) const
    {
        int i = idx * 3;

        return {p_data_[i], p_data_[i + 1], p_data_[i + 2]};
    }
}
