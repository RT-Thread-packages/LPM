/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020.12.11     wanghaijing  first version
 */

#include "lpm.h"

extern struct lpm lpm;

#include "x_mem.h"

struct lpm_dev *lpm_dev_find(const char *dev_name)
{
    rt_slist_t *node = RT_NULL;
    struct lpm_dev *lpm_dev;

    rt_slist_for_each(node, &(lpm.device_list))
    {
        lpm_dev = rt_slist_entry(node, struct lpm_dev, dev_list);
        if (rt_strncmp(lpm_dev->name, dev_name, RT_NAME_MAX) == 0)
        {
            return lpm_dev;
        }
    }
    return RT_NULL;
}

/* 底层接口 */
int lpm_dev_append(struct lpm_dev *dev)
{
    x_mem_t lpm_mem;

    RT_ASSERT(dev);

    lpm_mem = x_create((void *) 0, (void *) dev->sector_count, 1); //todo 这里是因为 block 的大小与 sector 一致
    if (lpm_mem == RT_NULL)
    {
        return -RT_ERROR;
    }

    dev->mem_ptr = lpm_mem;

    lpm_part_alloc(dev, 2);

    lpm_part_info_load(dev);

    rt_slist_insert(&(lpm.device_list), &(dev->dev_list));

    return RT_EOK;
}

/* dump all device */
int lpm_dump_dev(void)
{
    rt_slist_t *node = RT_NULL;
    struct lpm_dev *lpm_dev = RT_NULL;

    rt_kprintf("==================== LPM Device table =======================\r\n");
    rt_kprintf("|  dev_name  | bytes_per_sector | sector_count | block_size |\r\n");
    rt_kprintf("-------------------------------------------------------------\r\n");

    rt_slist_for_each(node, &(lpm.device_list))
    {
        lpm_dev = rt_slist_entry(node, struct lpm_dev, dev_list);

        rt_kprintf("|  %-*.*s  |    0x%08lx    |  0x%08lx  | 0x%08lx |\r\n", RT_NAME_MAX, RT_NAME_MAX, lpm_dev->name, \
                lpm_dev->bytes_per_sector, lpm_dev->sector_count, lpm_dev->block_size);
    }

    rt_kprintf("=============================================================\r\n");

    if (lpm_dev != RT_NULL)
    {
        return RT_EOK;
    }
    return -RT_ERROR;
}

int lpm_dev_read(struct lpm_dev *dev, uint32_t sector_no, uint8_t *buf, size_t size)
{
    RT_ASSERT(dev);
    RT_ASSERT(dev->ops);
    RT_ASSERT(dev->ops->read);

    return dev->ops->read(dev, sector_no, buf, size);
}

int lpm_dev_write(struct lpm_dev *dev, uint32_t sector_no, const uint8_t *buf, size_t size)
{
    RT_ASSERT(dev);
    RT_ASSERT(dev->ops);
    RT_ASSERT(dev->ops->write);

    return dev->ops->write(dev, sector_no, buf, size);
}

int lpm_dev_erase(const struct lpm_dev *dev, uint32_t block_no, size_t num)
{
    return RT_EOK;
}

int lpm_dev_erase_all(const struct lpm_dev *dev)
{
    return RT_EOK;
}
