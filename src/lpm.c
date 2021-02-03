/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020.12.11     wanghaijing  first version
 */

#include <string.h>
#include <stdlib.h>
#include "lpm.h"

struct lpm lpm;

/* 初始化 */
int lpm_init(void)
{
    rt_slist_init(&lpm.device_list);

    /* init lpm */
    return RT_EOK;
}
static void lpmm(uint8_t argc, char **argv)
{
//    struct lpm_dev *lpm_dev;
    struct lpm_partition *part = NULL;
    size_t i = 0, j = 0;

#define CMD_PROBE_INDEX  0
#define CMD_READ_INDEX   1
#define CMD_WRITE_INDEX  2
#define CMD_CREATE_INDEX 3
#define CMD_DELETE_INDEX 4
#define CMD_DUMP_INDEX   5
#define HEXDUMP_WIDTH    16
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

    const char* help_info[] = {
            [CMD_PROBE_INDEX]  = "lpmm probe  dev_name part_name                 - e.g: lpmm probe norflash app",
            [CMD_READ_INDEX]   = "lpmm read   dev_name part_name sector_no size  - e.g: lpmm read norflash app 0 1",
            [CMD_WRITE_INDEX]  = "lpmm write  dev_name part_name sector_no size  - e.g: lpmm write norflash app 0 1",
            [CMD_CREATE_INDEX] = "lpmm create dev_name part_name size            - e.g: lpmm create norflash app 10",
            [CMD_DELETE_INDEX] = "lpmm delete dev_name part_name                 - e.g: lpmm delete norflash app",
            [CMD_DUMP_INDEX]   = "lpmm dump   dev_name info or part_name info    - e.g: lpmm dump or lpm dump part", };

    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
        {
            rt_kprintf("%s\n", help_info[i]);
        }
        rt_kprintf("\n");
    }

    else
    {
        const char *operator = argv[1];

        if (!strcmp(operator, "probe"))
        {
            if (argc >= 4)
            {
                char* dev_name = argv[2];
                char* part_name = argv[3];

                if ((part = lpm_partition_find(dev_name, part_name)) == RT_NULL)
                {
                    rt_kprintf("Device %s Partition %s NOT found. Probe failed.\n", dev_name, part_name);
                    part = RT_NULL;
                }
                else
                {
                    rt_kprintf("Probed a block device | %s | offset: %ld | size: %d |.\n", part->name, part->offset,
                            part->size);
                }
            }
        }
        else if (!strcmp(operator, "create"))
        {
            if (argc >= 5)
            {
                char* dev_name = argv[2];
                char* part_name = argv[3];
                uint32_t part_size = atoi(argv[4]);

                if ((part = lpm_partition_create(dev_name, part_name, part_size)) == RT_NULL)
                {
                    rt_kprintf("Create %s Partition %s failed.\n", dev_name, part_name);
                    part = RT_NULL;
                }
                else
                {
                    rt_kprintf("Create block device | %s | offset: %ld | size: %d |.\n", part->name, part->offset,
                            part->size);
                }
            }
        }
        else if (!strcmp(operator, "delete"))
        {
            if (argc >= 4)
            {
                if(!strcmp(argv[3], "all"))
                {
                    lpm_partition_delete_all(argv[2]);
                }
                else
                {
                    char* dev_name = argv[2];
                    char* part_name = argv[3];

                    if (lpm_partition_delete(dev_name, part_name) != RT_EOK)
                    {
                        rt_kprintf("Delete %s Partition %s failed.\n", dev_name, part_name);
                    }
                    else
                    {
                        rt_kprintf("Delete %s Partition %s Success.\n", dev_name, part_name);
                    }
                }
            }
        }
        else if (!strcmp(operator, "read"))
        {
            if (argc >= 6)
            {
                struct lpm_partition *lpm_par;

                char* dev_name = argv[2];
                char* part_name = argv[3];
                uint32_t sector_no = atoi(argv[4]);
                uint32_t size = atoi(argv[5]);
                uint8_t *read_buf = RT_NULL;
                int result;

                lpm_par = lpm_partition_find(dev_name, part_name);
                if (lpm_par == RT_NULL)
                {
                    rt_kprintf("Error: the Device name (%s) or the partition name (%s) is not found.", dev_name,
                            part_name);
                    return;
                }
                else
                {
                    read_buf = rt_malloc(lpm_par->dev->block_size);
                    if (read_buf == RT_NULL)
                    {
                        rt_kprintf("Low memory!\n");
                    }

                    result = lpm_partition_read(lpm_par, sector_no, read_buf, size);
                    if (result >= 0)
                    {
                        size = size * (lpm_par->dev->block_size);

                        rt_kprintf("Read data success. Start sector_no from 0x%08X, size is %ld. The data is:\n",
                                sector_no, size);
                        rt_kprintf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
                        for (i = 0; i < size; i += HEXDUMP_WIDTH)
                        {
                            rt_kprintf("[%08X] ", sector_no + i);
                            /* dump hex */
                            for (j = 0; j < HEXDUMP_WIDTH; j++)
                            {
                                if (i + j < size)
                                {
                                    rt_kprintf("%02X ", read_buf[i + j]);
                                }
                                else
                                {
                                    rt_kprintf("   ");
                                }
                            }
                            /* dump char for hex */
                            for (j = 0; j < HEXDUMP_WIDTH; j++)
                            {
                                if (i + j < size)
                                {
                                    rt_kprintf("%c", __is_print(read_buf[i + j]) ? read_buf[i + j] : '.');
                                }
                            }
                            rt_kprintf("\n");
                        }
                        rt_kprintf("\n");
                        rt_free(read_buf);
                    }
                    else
                    {
                        rt_free(read_buf);
                    }
                }
            }
            else
            {
                rt_kprintf("Please enter the correct parameters!\r\n ");
            }
        }
        else if (!strcmp(operator, "write"))
        {
            if (argc >= 6)
            {
                struct lpm_partition *lpm_par;

                char* dev_name = argv[2];
                char* part_name = argv[3];
                uint32_t sector_no = atoi(argv[4]);
                uint32_t size = atoi(argv[5]); //要写的 block 数量
                uint8_t *write_buf = RT_NULL;
                int result;
                uint32_t num;

                if(!size)
                {
                    rt_kprintf("Please set 'size' greater than 0!\n");
                    return;
                }

                lpm_par = lpm_partition_find(dev_name, part_name);
                if (lpm_par == RT_NULL)
                {
                    rt_kprintf("Error: the Device name (%s) or the partition name (%s) is not found.", dev_name,
                            part_name);
                    return;
                }
                else
                {
                    if(lpm_par->size < size )
                    {
                        rt_kprintf("The 'size' of the input is larger than the 'size of the partition'!\n");
                        return;
                    }

                    write_buf = rt_calloc(1, size*(lpm_par->dev->block_size));
                    if (write_buf == RT_NULL)
                    {
                        rt_kprintf("Low memory!\n");
                    }

                    num = size * (lpm_par->dev->block_size);

                    for (i = 0; i < num; i++)
                    {
                        write_buf[i] = i & 0xFF;
                    }

                    result = lpm_partition_write(lpm_par, sector_no, write_buf, size);
                    if (result >= 0)
                    {
                        rt_kprintf("LPM Write Success at the Device name (%s)  the partition name (%s) .\r\n", dev_name,
                                part_name);
                    }
                    else
                    {
                        rt_kprintf("LPM Write Failed at the Device name (%s)  the partition name (%s) .\r\n", dev_name,
                                part_name);
                    }
                    rt_free(write_buf);
                }
            }
        }
        else if (!strcmp(operator, "dump"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[2], "part"))
                {
                    lpm_dump_partition();
                }
                else
                {
                    lpm_dump_dev();
                }
            }
        }
        else if (!strcmp(operator, "free"))
        {
            rt_kprintf("LPM device: %08s , residue 0x%08x.\r\n", argv[2], lpm_free(argv[2]));
        }
        else
        {
            rt_kprintf("Please enter the correct parameters!\r\n ");
        }
    }
}
MSH_CMD_EXPORT(lpmm, LPM (Logical partition management) operate.);
