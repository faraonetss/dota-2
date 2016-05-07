/***********************************************************************************************************************
*  FOR DOTA, FOR ICEFROG
*
*  Copyright (c) xiaomaoshi, all rights reserved.
*
*  Email: mym_cocacola@163.com
*
* File name: dota_frame.c
* Describe:
* Author: xiaomaoshi
* Action: create file
* Date: 2015/07/20
* History: 
***********************************************************************************************************************/

#include "dota_data.h"
#include "dota_list.h"
#include "dota_enum.h"
#include "dota_time.h"
#include <time.h>

extern void time_run(void);
extern int axe_register(void);
extern int es_register(void);
extern int centaur_register(void);

LIST_HEAD_S all_life;

OPER0_PF all_register[] = {
    axe_register,
    es_register,
    centaur_register,
    NULL,
};

int dota_schedule(void)
{
    int result = DOTA_INVALID;
    int over   = 0;
    LIFE_S      *life = NULL;
    LIST_HEAD_S *item = NULL;

    while (!over) {
        /*
         * ���ȼٶ�ս�������������ñ�Ǻ��汻��Ϊ0��
         * ˵���ٶ�������������ѭ����
         */
        over = 1;

        /*
         * ����ע���˳�����ε���ÿ��life����������run���е�����
         * �غ��Ƶ���Ϸ������ĳ��run���൱�ڽ����life�Ļغϣ�����
         * ��λ��ʱ�������κζ��������е�run��ִ��һ���൱��һ�ֻ�
         * �Ͻ�����
         */
        LIST_FOR_EACH(item, &all_life) {
            life = LIST_ENTRY(item, LIFE_S, life_list);
            life->ops->kill(life);

            if (LIFE_DEAD == life->life_state)
                continue;

            if (LIFE_ZOMBIE == life->life_state) {
                life->ops->death(life);
                continue;
            }

            result = life->ops->run(life);
            if (ERR_SHOULD_OVER == result)
                continue;

            /*
             * ����һ��˵��ս����δ������ս�������������ǣ�����ȫ��
             * �������������е�λȫ��������
             */
            over = 0;
        }

        /* 
         * һ��ѭ��������ʱ������100���룬��ÿ��100�������������
         * �Ⱥ�˳���(����ע���˳��)����͸�����CPU˳����Ƚ�����
         * һ���ģ�΢�۴��У���۲��С���ʵ˵��һ�ֻغϽ��������ʡ�
         */
        time_run();
    }
    return DOTA_SUCCESS;
}


int dota_finish(void)
{
    LIFE_S      *life = NULL;
    LIST_HEAD_S *item = NULL;

    LIST_FOR_EACH(item, &all_life) {
        life = LIST_ENTRY(item, LIFE_S, life_list);
        if (life->ops->clean)
            life->ops->clean(life);
    }
    return DOTA_SUCCESS;
}


int dota_register(void)
{
    OPER0_PF *reg_func = all_register;

    INIT_LIST_HEAD(&all_life);

    while (*reg_func)
        (*reg_func++)();

    srand((unsigned)time(NULL));
    return DOTA_SUCCESS;
}
