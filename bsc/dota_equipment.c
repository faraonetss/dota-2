/***********************************************************************************************************************
*  FOR DOTA, FOR ICEFROG!
*
*  Copyright (c) xiaomaoshi, all rights reserved.
*
*  Email: mym_cocacola@163.com
*
* File name: dota_equipment.c
* Describe: 
* Author: xiaomaoshi
* Action: create file
* Date: 2015/05/20
* History: 
***********************************************************************************************************************/

#include "dota_equipment.h"
#include "dota_operate.h"
#include "dota_data.h"
#include "dota_log.h"

#if 1

/* �򵶵�Ч������ */
static int gsv_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* ������Ӱ���bitλ */
    SET_GSV_EFFECT(self);
    return DOTA_OK;
}

/* ����򵶵�Ч������ */
static int clean_gsv_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* ������Ӱ���bitλ */
    CLEAN_GSV_EFFECT(self);
    return DOTA_OK;
}


/* ���ȵ�Ч������ */
static int eul_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    SET_EUL_EFFECT(self);
    TRACE_BATTLE("%s is under EUL effect.\n", self->name);
    return DOTA_OK;
}

/* ���ȶԵз���λ��ɵ��˺� */
#define EUL_DAMAGE 50

/* ������ȵ�Ч������ */
static int clean_eul_buff(LIFE_S *self, LIFE_S *owner)
{
    unsigned short damage = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    DOTA_RETURN_IF_NULL(owner, ERR_NULL_POINTER);

    CLEAN_EUL_EFFECT(self);
    TRACE_BATTLE("%s is clean EUL effect.\n", self->name);

    /* �����Ƕ��Լ��ͷţ����ǶԵ����ͷţ�����غ����50���˺� */
    if (self == owner)
        return DOTA_OK;

    damage = HERO_MAGIC_DAMAGE(EUL_DAMAGE, self->ms.spell_rst);
    TRACE_BATTLE("%s's EUL make %hu damage to %s.\n",
                 owner->name, damage, self->name);

    if (self->cur_hmaa.health <= damage) {
        self->life_state = LIFE_ZOMBIE;
        self->murderer = owner;
        queue_in(owner->kill_queue, self);
        return DOTA_OK;
    }
    self->cur_hmaa.health -= damage;
    return DOTA_OK;
}

/* ����Ч���ĺ��� */
static int xms_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* �ҵ��������Լ� */
    self->target = self;
    self->ops->attack(self);
    SET_XMS_EFFECT(self);
    return DOTA_OK;
}

/* �������Ч���ĺ��� */
static int clean_xms_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    self->target = NULL;
    CLEAN_XMS_EFFECT(self);
    return DOTA_OK;
}
#endif

/***********************************************************************************************************************
* Function name:  EQP_GuinsoosScytheofVyse
* Equipment name: ��
* Describe: ���������100ħ������ȴʱ��35�룩��Ŀ����һֻ�����ڼ�ֻ�ܱ�
*           �����ֱ�������Ч���Ҳ��ܹ�����ֻ�л������ƶ��ٶȣ�����3.5��
* Mana Cost: 100��
* Cold Down: 35��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int eqp_guinsoos_sv(LIFE_S *self)
{
    BUFF_NODE_S *buff_node = NULL;
    EQUIPMENT_S *eqp = NULL;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    eqp = &(g_shop[EQP_GUINSOOS_SCYTHE_OF_VYSE]);

    TRACE_BATTLE("%s use %s to %s.\n",
                 self->name,
                 eqp->skl_attr.name,
                 target->name);

    /* �½�һ��buff�ڵ� */
    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->buff_name = eqp->skl_attr.name;
    buff_node->duration = eqp->skl_attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = 0;
    buff_node->do_buff = gsv_buff;
    buff_node->clean_buff = clean_gsv_buff;
    buff_node->owner = self;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_MIN;

    /* Ч�����õ��ӣ�����ԭ����gsv buff�������еĻ� */
    (void)clean_buff_byname(target, buff_node->buff_name);
    add_buff_node(buff_node, &(target->buff_list));
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name:  EQP_EulScepterofDivinity
* Equipment name: ����
* Describe: ʹ�Է�����3�룬���Զ��Լ�ʹ�ã�ʩ������700����ȴʱ��30�롣������紵��ĵз���λ���ʱ���ܵ�50���˺�
����        �����ɶԴ���ħ��״̬�µ�����ʩ��
* Mana Cost: 75��
* Cold Down: 30��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int eqp_eul_sd(LIFE_S *self)
{
    BUFF_NODE_S *buff_node = NULL;
    EQUIPMENT_S *eqp = NULL;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    eqp = &(g_shop[EQP_EUL_SCEPTER_OF_DIVINITY]);

    /* �����Ƕ��Լ��ͷţ���Ҫ�ж�Ŀ���Ƿ�ħ������ */
    if (self != target) {
        if (GET_CAMP(self) == GET_CAMP(target)) {
            TRACE_BATTLE("EUL only work to enemy or self.\n");
            return ERR_WRONG_OBJ;
        }

        if (IS_MAIGC_IMMUNITY(target)) {
            TRACE_BATTLE("use EUL failed, target magic immunity.\n");
            return ERR_MAGIC_IMMUNITY;
        }
    }

    TRACE_BATTLE("%s use %s to %s.\n",
                 self->name,
                 eqp->skl_attr.name,
                 target->name);

    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->buff_name = eqp->skl_attr.name;
    buff_node->duration = eqp->skl_attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = 0;
    buff_node->do_buff = eul_buff;
    buff_node->clean_buff = clean_eul_buff;
    buff_node->owner = self;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_MIN;

    /* ��ӵ�target��buff������ */
    add_buff_node(buff_node, &(target->buff_list));
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name:  eqp_icefrog
* Equipment name: ���ܵ���Ʒ
* Describe: 
*
* Mana Cost:
* Cold Down:
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int eqp_icefrog(LIFE_S *self)
{
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name:  EQP_Xiaomaoshi
* Equipment name: Сëʩ��а�񵰵�
* Describe: ����󣬻���Ŀ�군�ۣ�����3�롣�����ڼ䣬Ŀ����������Լ��������ҵ���
* Mana Cost: 50��
* Cold Down: 40��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int eqp_xiaomaoshi_ee(LIFE_S *self)
{
    BUFF_NODE_S *buff_node = NULL;
    EQUIPMENT_S *eqp = NULL;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    eqp = &(g_shop[EQP_XIAOMAOSHI_EGGS_OF_EVIL]);

    TRACE_BATTLE("%s use %s to %s.\n",
                 self->name,
                 eqp->skl_attr.name,
                 target->name);

    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->buff_name = eqp->skl_attr.name;
    buff_node->duration = eqp->skl_attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = 0;
    buff_node->do_buff = xms_buff;
    buff_node->clean_buff = clean_xms_buff;
    buff_node->owner = self;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_MIN;

    /* �����ԭ����Сëʩа�񵰵�Ч�� */
    (void)clean_buff_byname(target, buff_node->buff_name);
    add_buff_node(buff_node, &(target->buff_list));
    return DOTA_SUCCESS;
}


/* �̵� */
EQUIPMENT_S g_shop[] = {
    {{10, 10, 35}, {NAME_GSV, SKILL_NORMAL,
     0, 100, 350, 35, 800, eqp_guinsoos_sv}},

    {{0,  0,  10}, {NAME_EUL, SKILL_EUL_LIKE,
     0, 75, 300, 25, 700, eqp_eul_sd}},

    {{1,  1,  1 }, {NAME_XMS, SKILL_NORMAL,
     0, 50,  400, 30, 750, eqp_xiaomaoshi_ee}},
};


/*
 * Ӣ���ӵ�һ��װ����box_idΪ��һ������
 */
int takeoff_equip(LIFE_S *self, int box_id)
{
    EQUIPMENT_S *eqp = NULL;
    HMAA_S tmp = {0, 0, 0, 0};
    unsigned long long time = 0;
    unsigned long long used_time = 0;
    unsigned short cd = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    if (box_id >= EQUIP_MAX)
        return ERR_UNKNOW_EQP;

    if (!IS_HERO(self))
        return ERR_WRONG_OBJ;

    eqp = self->equips[box_id].eqp;
    if (!eqp)
        return DOTA_OK;

    /*
     * ������ȴ״̬�µ�װ���ǲ����ӵ���
     */
    time = get_current_time();
    used_time = self->equips[box_id].used_time;
    cd = eqp->skl_attr.cold_down;
    if (used_time && (used_time + cd < time))
        return ERR_COLD_DOWN;

    ASM_TO_HMAA(&(eqp->bsc_attr), &tmp);

    /*
     * ���Է��֣����ӵ�װ���۳�Ѫ��ʱ���Ǹ��ݰٷֱ����۳��ġ�����
     * �򵶸�ĳӢ��������100��Ѫ��������Ӣ��װ���򵶺���Ѫ��Ѫ��
     * Ϊ500�㣬��ô��ĳһʱ�̣�����Ӣ�۵�Ѫ��Ϊ300�㣬��ʱ���ӵ�
     * �������Ʒ��Ӣ�۵�Ѫ������300 - 100 = 200������300*(4/5)
     * ����240�㣬HMAA��������ԱҲӦ�����ش˹�ء�
     */
    MINUS_HAMM_PER(&(self->cur_hmaa), &(self->org_hmaa), &tmp);
    MINUS_HAMM(&self->org_hmaa, &tmp);

    MINUS_ASM(&(self->org_asm), &(eqp->bsc_attr));
    MINUS_ASM(&(self->cur_asm), &(eqp->bsc_attr));

    /* �ӵ� */
    self->equips[box_id].eqp = NULL;
    self->equips[box_id].used_time = 0;
    return DOTA_OK;
}


int takeon_equip(LIFE_S *self, int box_id, EQUIPMENT_S *eqp)
{
    HMAA_S tmp = {0, 0, 0, 0};
    int ret = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    if (!eqp || box_id >= EQUIP_MAX)
        return ERR_UNKNOW_EQP;

    if (DOTA_OK != (ret = takeoff_equip(self, box_id)))
        return ret;

    ASM_TO_HMAA(&(eqp->bsc_attr), &tmp);

    /*
     * ��װ��һ����Ʒ��ʱ��ͬ��Ҳ����ѭһ���ٷֱȵķ��򡣼�����
     * �����ӵ���Ʒʱ�����������ͣ�����ʱӢ�۵�Ѫ��Ϊ240�㣬װ��
     * ���򵶺��Ѫ������240 + 100 = 340�㣬����240/(4/5)=300�㡣
     * ����ٷֱ�(4/5)�������ӵ�Ѫ��(100)�����Ӻ���Ѫ״̬��Ѫ��
     * (500)�ı�ֵ�����100%�Ĳ��롣������Ҫ�ȵ���ADD_HAMM���õ���
     * Ѫ��Ѫ�������ӵ�װ��ʱ�෴��
     */
    ADD_HAMM(&(self->org_hmaa), &tmp);
    ADD_HAMM_PER(&(self->cur_hmaa), &(self->org_hmaa), &tmp);

    ADD_ASM(&(self->org_asm), &(eqp->bsc_attr));
    ADD_ASM(&(self->cur_asm), &(eqp->bsc_attr));

    /* 
     * ��ʼ����ʱ����ʵû��Ҫ����һ����������ô����Ŀ��
     * �Ǳ�֤����ʱ����������������OK�ġ�
     */
    self->equips[box_id].eqp = eqp;
    self->equips[box_id].used_time = 0;
    return DOTA_OK;
}


int init_box(LIFE_S *self)
{
    int id = 0;
    EQUIPMENT_S * eqp = NULL;
    EQUIP_LIFE_S *equips = NULL;

    DOTA_RETURN_IF((!self), ERR_NULL_POINTER);

    for (equips = self->equips; !equips; equips++, id++) {
        eqp = equips->eqp;
        (void)takeon_equip(self, id, eqp);
    }
    return DOTA_OK;
}
