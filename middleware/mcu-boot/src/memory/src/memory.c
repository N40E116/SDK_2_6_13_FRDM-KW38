/*
 * Copyright (c) 2013-2015 Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bootloader_common.h"
#include "bootloader/bl_context.h"
#include "memory/memory.h"
#include "utilities/fsl_assert.h"

//! @addtogroup memif
//! @{

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

//! @brief This variable is used to do flush operation, it is bind to write operation.

static status_t (*s_flush)(void) = (void *)0u;
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
static status_t (*s_finalize)(void) = (void *)0u;
#endif // BL_FEATURE_EXPAND_MEMORY

//! @brief Interface to generic memory operations.
const memory_interface_t g_memoryInterface = {
    mem_init,     mem_read, mem_write,
#if !(defined(BL_FEATURE_MIN_PROFILE) && BL_FEATURE_MIN_PROFILE) || \
    defined(BL_FEATURE_FILL_MEMORY) && BL_FEATURE_FILL_MEMORY
    mem_fill,
#else
    (void *)0u,
#endif // !BL_FEATURE_MIN_PROFILE
    mem_flush,
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
    mem_finalize,
#else
    (void *)0u,
#endif // BL_FEATURE_EXPAND_MEMORY
    mem_erase,
};

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

// See memory.h for documentation on this function.
status_t mem_read(uint32_t address, uint32_t length, uint8_t *buffer, uint32_t memoryId)
{
    status_t status = (int32_t)kStatus_Success;

    if (length != 0u)
    {
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
        switch (GROUPID(memoryId))
        {
            case kGroup_Internal:
            {
#endif // BL_FEATURE_EXPAND_MEMORY
                const memory_map_entry_t *mapEntry;
                 status = find_map_entry(address, length, &mapEntry);
                if (status == (int32_t)kStatus_Success)
                {
                    status = mapEntry->memoryInterface->read(address, length, buffer);
                }
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
            }
            break;
            case kGroup_External:
            {
                const external_memory_map_entry_t *mapEntry;
                status = find_external_map_entry(address, length, memoryId, &mapEntry);
                if (status == (int32_t)kStatus_Success)
                {
                    s_finalize = mapEntry->memoryInterface->finalize;
                    status = mapEntry->memoryInterface->read(address, length, buffer);
                }
            }
            break;
            default:
                status = (int32_t)kStatusMemoryRangeInvalid;
                break;
        }
#endif // BL_FEATURE_EXPAND_MEMORY
    }

    return status;
}

// See memory.h for documentation on this function.
status_t mem_write(uint32_t address, uint32_t length, const uint8_t *buffer, uint32_t memoryId)
{
    status_t status = (int32_t)kStatus_Success;
    if (length != 0u)
    {
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
        switch (GROUPID(memoryId))
        {
            case kGroup_Internal:
            {
#endif // BL_FEATURE_EXPAND_MEMORY
                if (mem_is_block_reserved(address, length))
                {
                    status = (int32_t)kStatusMemoryRangeInvalid;
                }
                else
                {
                    const memory_map_entry_t *mapEntry;
                    status = find_map_entry(address, length, &mapEntry);

                    if (status == (int32_t)kStatus_Success)
                    {
                        status = mapEntry->memoryInterface->write(address, length, buffer);

                        if (status == (int32_t)kStatus_Success)
                        {
                            s_flush = mapEntry->memoryInterface->flush;
                        }
                        else
                        {
                            s_flush = (void *)0u;
                        }
                    }
                }
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
            }
            break;
            case kGroup_External:
            {
                const external_memory_map_entry_t *mapEntry;
                status = find_external_map_entry(address, length, memoryId, &mapEntry);
                if (status == (int32_t)kStatus_Success)
                {
                    s_finalize = mapEntry->memoryInterface->finalize;
                    status = mapEntry->memoryInterface->write(address, length, buffer);
                    if (status == (int32_t)kStatus_Success)
                    {
                        s_flush = mapEntry->memoryInterface->flush;
                    }
                    else
                    {
                        s_flush = (void *)0u;
                    }
                }
            }
            break;
            default:
                status = (int32_t)kStatusMemoryRangeInvalid;
                break;
        }
#endif // BL_FEATURE_EXPAND_MEMORY
    }

    return status;
}

status_t mem_erase(uint32_t address, uint32_t length, uint32_t memoryId)
{
    status_t status = (int32_t)kStatus_Success;

#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
    switch (GROUPID(memoryId))
    {
        case kGroup_Internal:
        {
#endif // BL_FEATURE_EXPAND_MEMORY
            const memory_map_entry_t *mapEntry;
            status = find_map_entry(address, length, &mapEntry);
 
            if (status == (int32_t)kStatus_Success)
            {
                // In this case, it means that bootloader tries to erase a range of memory
                // which doesn't support erase operaton
                if (mapEntry->memoryInterface->erase == (void *)0u)
                {
                    status = (int32_t)kStatusMemoryUnsupportedCommand;
                }
                else
                {
                    if (mem_is_block_reserved(address, length))
                    {
                        status = (int32_t)kStatusMemoryRangeInvalid;
                    }
                    else
                    {
                        status = mapEntry->memoryInterface->erase(address, length);
                    }
                }
            }
            else if (length == 0u)
            {
                // if length = 0, return kStatus_Success regardless of memory address
                status = (int32_t)kStatus_Success;
            }
            else
            {
                // doing nothing
            }
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
        }
        break;
        case kGroup_External:
        {
            const external_memory_map_entry_t *mapEntry;
            status = find_external_map_entry(address, length, memoryId, &mapEntry);
            if (status == (int32_t)kStatus_Success)
            {
                if (mapEntry->memoryInterface->erase == (void *)0u)
                {
                    status = (int32_t)kStatusMemoryUnsupportedCommand;
                }
                else
                {
                    status = mapEntry->memoryInterface->erase(address, length);
                }
            }
            else if (length == 0u)
            {
                status = (int32_t)kStatus_Success;
            }
            else
            {
                // doing nothing
            }
        }
        break;
        default:
            status = (int32_t)kStatusMemoryRangeInvalid;
            break;
    }
#endif // BL_FEATURE_EXPAND_MEMORY
    return status;
}

// See memory.h for documentation on this function.
status_t mem_fill(uint32_t address, uint32_t length, uint32_t pattern)
{
    status_t status = (int32_t)kStatus_Success;
    if (length != 0u)
    {
        if (mem_is_block_reserved(address, length))
        {
            status = (int32_t)kStatusMemoryRangeInvalid;
        }
        else
        {
            const memory_map_entry_t *mapEntry;
            status = find_map_entry(address, length, &mapEntry);

            if (status == (int32_t)kStatus_Success)
            {
                status = mapEntry->memoryInterface->fill(address, length, pattern);
            }
        }
    }
    return status;
}

//! @brief Flush buffered data into target memory
//! @note  1. This function should be called immediately after one write-memory command(either
//!        received in command packet or in sb file), only in this way, given data can be programmed
//!        at given address as expected.
//!
//!        2. So far, flush() is only implemented in qspi memory interface, for other memory
//!        interfaces, it is not available and mem_flush() just returns kStatus_Success if it is
//!        called.
//!
//!        3. This function is designed to flush buffered data into target memory, please call it
//!        only if it is required to do so. For example, write 128 bytes to qspi flash, while the
//!        page size is 256 bytes, that means data might not be written to qspi memory immediately,
//!        since the internal buffer of qspi memory interface is not full, if no data are expected
//!        to write to left area of the same page, this function can be used to force to write
//!        immediately, otherwise, keep in mind that any calls should be avoided. If users voilate
//!        this rules, it would make the left area of the same page cannot be programmed.
//!
//! @return An error code or kStatus_Success
status_t mem_flush(void)
{
    status_t status = (int32_t)kStatus_Success;

    if (s_flush != (void *)0u)
    {
        status = s_flush();
        s_flush = (void *)0u; // Clear this variable after performing flush operation
    }

    return status;
}

//! @brief Reset the state machine of memory interface when a read/write sequence is finished.
//!
//! @return An error code or kStatus_Success
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
status_t mem_finalize(void)
{
    status_t status = (int32_t)kStatus_Success;

    if (s_finalize != (void *)0u)
    {
        status = s_finalize();
        s_finalize = (void *)0u; // Clear this variable after performing finalize operation
    }

    return status;
}
#endif // BL_FEATURE_EXPAND_MEMORY

//! @brief Find a map entry that matches address and length.
//!
//! @param address Start address for the memory operation.
//! @param length Number of bytes on which the operation will act.
//! @param map The matching map entry is returned through this pointer if the return status
//!     is #kStatus_Success.
//!
//! @retval #kStatus_Success A valid map entry was found and returned through @a map.
//! @retval #kStatusMemoryRangeInvalid The requested address range does not match an entry, or
//!     the length extends past the matching entry's end address.
status_t find_map_entry(uint32_t address, uint32_t length, const memory_map_entry_t **map)
{
    status_t status = (int32_t)kStatusMemoryRangeInvalid;

    // Set starting entry.
    assert(map);
    if (map != (void *)0u)
    {
        *map = &g_bootloaderContext.memoryMap[0];
    }

    // Scan memory map array looking for a match.
    while ((length > 0u) && (map != (void *)0u) && (*map != (void *)0u))
    {
        if (((*map)->startAddress == 0u) && ((*map)->endAddress == 0u) && ((*map)->memoryInterface == (void *)0u))
        {
            break;
        }
        // Check if the start address is within this entry's address range.
        if ((address >= (*map)->startAddress) && (address <= (*map)->endAddress))
        {
            // Check that the length fits in this entry's address range.
            if ((address + length - 1u) <= (*map)->endAddress)
            {
                status = (int32_t)kStatus_Success;
            }
            break;
        }
        ++(*map);
    }

    return status;
}

// See memory.h for documentation on this function.
#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
status_t find_external_map_entry(uint32_t address,
                                 uint32_t length,
                                 uint32_t memory_id,
                                 const external_memory_map_entry_t **map)
{
    status_t status = (int32_t)kStatusMemoryRangeInvalid;

    // Set starting entry.
    assert(map);
    if (map)
    {
        *map = &g_bootloaderContext.externalMemoryMap[0];
    }

    // Scan memory map array looking for a match.
    while ((length > 0u) && (map != (void *)0u) && (*map != (void *)0u))
    {
        if (((*map)->memoryId == 0u) && ((*map)->status == 0u) && ((*map)->basicUnitCount == 0u) &&
            ((*map)->basicUnitSize == 0u) && ((*map)->memoryInterface == (void ()0u))
        {
            break;
        }

        // Check if the memory id is matched.
        if (memory_id == (*map)->memoryId)
        {
            // Check that the length fits in this entry's address range.
            if (((uint64_t)address + length) <= ((uint64_t)(*map)->basicUnitCount * (*map)->basicUnitSize))
            {
                status = (int32_t)kStatus_Success;
            }
            break;
        }
        ++(*map);
    }

    return status;
}

status_t find_external_map_index(uint32_t memoryId, uint32_t *index)
{
    status_t status = (int32_t)kStatus_InvalidArgument;

    const external_memory_map_entry_t *map;
    uint32_t searchingIndex = 0u;

    if (index != (void*)0u)
    {
        map = &g_bootloaderContext.externalMemoryMap[0];
        // Scan memory map array looking for a match.
        while(map && (map->memoryId != 0u) && (map->memoryInterface != (void *)0u))
        {
            if (memoryId == map->memoryId)
            {
                *index = searchingIndex;
                // Find the correct index.
                status = (int32_t)kStatus_Success;
                break;
            }
            searchingIndex++;
            map++;
        };
    }
    return status;
}
#endif // BL_FEATURE_EXPAND_MEMORY

// See memory.h for documentation on this function.
bool mem_is_block_reserved(uint32_t address, uint32_t length)
{
    uint32_t end = address + length - 1u;
    uint32_t start = 0u;
    bool retValue = (_Bool)false;

    for (uint32_t i = 0u; i < (uint32_t)kProperty_ReservedRegionsCount; ++i)
    {
        reserved_region_t *region = &g_bootloaderContext.propertyInterface->store->reservedRegions[i];

        start = (&g_bootloaderContext.memoryMap[i])->startAddress;
        if ((region->startAddress == start) && (region->endAddress == start))
        {
            // Special case, empty region
            continue;
        }

        if ((address <= region->endAddress) && (end >= region->startAddress))
        {
            retValue = true;
        }
    }
    return retValue;
}

// See memory.h for documentation on this function.
status_t mem_init(void)
{
    status_t status = (int32_t)kStatus_Success;

    const memory_map_entry_t *map = &g_bootloaderContext.memoryMap[0];

    while (map->memoryInterface != (void *)0u)
    {
        if (map->memoryInterface->init != (void *)0u)
        {
            (void)map->memoryInterface->init();
        }
        ++map;
    }

#if defined(BL_FEATURE_EXPAND_MEMORY) && BL_FEATURE_EXPAND_MEMORY
    external_memory_map_entry_t *extMap = (external_memory_map_entry_t *)&g_bootloaderContext.externalMemoryMap[0];
    while (extMap->memoryInterface != (void *)0u)
    {
        if (extMap->memoryInterface->init != (void *)0u)
        {
            status = extMap->memoryInterface->init();
        }
        ++extMap;
    }
#endif // BL_FEATURE_EXPAND_MEMORY

    return status;
}

// See memory.h for documentation on this function.
bool mem_is_erased(uint32_t address, uint32_t length)
{
    const uint8_t *start = (const uint8_t *)address;
    bool isMemoryErased = true;

    while (length != 0u)
    {
        if (*start != 0xFFu)
        {
            isMemoryErased = false;
            break;
        }
        else
        {
            length--;
            start++;
        }
    }

    return isMemoryErased;
}

//! @}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
