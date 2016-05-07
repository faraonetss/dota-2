/***********************************************************************************************************************
*  FOR DOTA, FOR ICEFROG!
*
*  Copyright (c) xiaomaoshi, all rights reserved.
*
*  Email: mym_cocacola@163.com
*
* File name: dota_time.c
* Describe: 
* Author: xiaomaoshi
* Action: create file
* Date: 2015/05/20
* History:
***********************************************************************************************************************/

#include "dota_time.h"

/* 
 * ʱ�侫����100���룬ÿ��1��ʾʱ������100���룬
 * ��ʵ˵��ÿ��1��ʾһ�ֻغϽ��������ʡ�����ĳ
 * ��Ч������2�룬��2000���룬��˵�ɳ���20���غ�
 * �����С�
 */
static unsigned long long g_dota_time = 1;

/* ÿ����һ�θú�����ʱ������100���� */
void time_run(void)
{
    g_dota_time++;
    return;
}

/* ��ȡʱ��ĺ��������ص�ǰʱ��ֵ */
unsigned long long get_current_time(void)
{
    return g_dota_time;
}
