/***********************************************************************************************************************
*  FOR DOTA, FOR ICEFROG!
*
*  Copyright (c) xiaomaoshi, all rights reserved.
*
*  Email: mym_cocacola@163.com
*
* File name: dota_hero.c
* Describe: 
* Author: xiaomaoshi
* Action: create file
* Date: 2015/05/20
* History: 
***********************************************************************************************************************/

#include "dota_data.h"
#include "dota_equipment.h"
#include "dota_operate.h"
#include "dota_time.h"
#include "dota_log.h"

/* ���ڱ���������״̬����buff����200���� */
int physical_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    return DOTA_OK;
}

/* ����������buff */
int clean_physical_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    return DOTA_OK;
}


int default_death(LIFE_S *self)
{
    unsigned int money = 0;
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    DOTA_RETURN_IF((LIFE_ZOMBIE != self->life_state), ERR_WRONG_OBJ);
    /* �������е�BUFF */
    clean_all_buff(self);

    /* ��0Ѫ����ħ�������ԣ��Ͼ���λ������ */
    RESET_HAMM(&(self->cur_hmaa));

    /* ���Ϊ��ɱ */
    if (self == self->murderer) {
        money = HERO_LOST_MONEY(0);
        TRACE_BATTLE("%s is suicide!\n", self->name);
    } else {
        money = HERO_LOST_MONEY(self->serial_kill);
        TRACE_BATTLE("%s is dead!\n", self->name);
    }

    if (self->money > money)
        self->money -= money;
    else
        self->money = 0;

    self->life_state = LIFE_DEAD;
    return DOTA_SUCCESS;
}


int default_rebirth(LIFE_S *self)
{
    HMAA_S        *pstOrgHmaa = NULL;
    HMAA_S        *pstCurHmaa = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    pstOrgHmaa = &(self->org_hmaa);
    pstCurHmaa = &(self->cur_hmaa);

    /* ��������ɱ������Ҫ������ɱ�������� */
    if (self != self->murderer)
        self->serial_kill = 0;

    /* �ÿ����� */
    self->murderer = NULL;

    /* �������ԣ�Ӣ������ */
    COPY_HAMM(pstOrgHmaa, pstCurHmaa);
    self->state = self->org_state;

    TRACE_BATTLE("%s is rebirth!\n", self->name);
    self->life_state = LIFE_RUNNING;
    return DOTA_SUCCESS;
}


int default_kill(LIFE_S *self)
{
    LIFE_S *target = NULL;
    QUEUE_S *queue = NULL;
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    queue = self->kill_queue;
    while (!queue_empty(queue)) {
        target = (LIFE_S *)queue_out(queue);

        if (!target || self == target)
            continue;

        TRACE_BATTLE("%s killed %s!\n",
                     self->name,
                     target->name);

        if (!IS_HERO(target)) {
            self->money += target->money;
            continue;
        }

        if (self->serial_kill < 10)
            self->serial_kill += 1;

        self->money += HERO_GET_MONEY(target->serial_kill);
    }
    return DOTA_SUCCESS;
}

/* Ĭ�ϵ����������� */
int default_attack(LIFE_S *self)
{
    unsigned short     damage    = 0;
    unsigned short     cold_down  = 0;
    unsigned long long cur_time  = 0;
    unsigned long long used_time = 0;
    BUFF_NODE_S       *buff_node = NULL;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    /* �ж������Ƿ���Խ��������� */
    if (!CAN_PHYSICAL_ATTACK(self)) {
        TRACE_BATTLE(INFO_FORMAT "%s cannot physical attack!\n",
                     self->name);
        return ERR_CANNOT_PHYSICAL_ATTACK;
    }

    /* �ж�Ŀ���Ƿ���Ա����� */
    if (!IS_CAN_BE_ATTACKED(target)) {
        TRACE_BATTLE(INFO_FORMAT "%s cannot be attacked!\n",
                     target->name);
        return ERR_CANNOT_BE_ATTACKED;
    }

    cold_down = self->skills[PY_ATTK].attr.cold_down;
    used_time = self->skills[PY_ATTK].used_time;
    cur_time = get_current_time();
    /* 
     * �����ǵ�һ�������������ж��������Ƿ���ȴ����
     * Ϊ������Ҳ���м���ģ�������
     */
    if (used_time && (used_time + cold_down > cur_time)) {
        TRACE_BATTLE(INFO_FORMAT "%s use %s is cold down"
                     "(LAST USE: %04llu, interval: %hu)!\n",
                     self->name, self->skills[PY_ATTK].attr.name,
                     used_time, cold_down);
        return ERR_COLD_DOWN;
    }

    /* ����һ��BUFF�ڵ� */
    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->buff_name = self->skills[PY_ATTK].attr.name;
    buff_node->duration = self->skills[PY_ATTK].attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = 0;
    buff_node->do_buff = physical_buff;
    buff_node->clean_buff = clean_physical_buff;
    buff_node->owner = self;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_MIN;

    /* 
     * physical buff�ǵ��ӵģ������ĳ����λ���ϵĸ���buff������
     * ���жϴ�ʱ�ж��ٵ������ڹ����õ�λ����buffֻ�����200���롣
     */
    add_buff_node(buff_node, &(target->buff_list));

    /* ��¼������������ʱ�� */
    self->skills[PY_ATTK].used_time = cur_time;

    /* ���㱾����������ɵ��˺� */
    damage = HERO_PHYSICAL_DAMAGE(self->cur_hmaa.attack,
                                  target->cur_hmaa.armor);
    TRACE_BATTLE("%s physical attack %s, make %hu damage.\n",
                 self->name, target->name, damage);

    /* �ж��Ƿ������ͷ���� */
    if (target->cur_hmaa.health <= damage) {
        target->life_state = LIFE_ZOMBIE;
        /* ���ɱ���ˣ���ǵ��������� */
        target->murderer = self;
        queue_in(self->kill_queue, target);
        return DOTA_SUCCESS;
    }
    target->cur_hmaa.health -= damage;
    return DOTA_SUCCESS;
}


/* Ĭ�ϵ��ͷż��ܵĲ��� */
int default_skill(LIFE_S *self, unsigned int id)
{
    unsigned short     mana  = 0;
    unsigned short     cold_down  = 0;
    unsigned long long cur_time  = 0;
    unsigned long long used_time = 0;
    int                ret        = 0;
    const char        *skl_name   = NULL;
    OPER1_PF           skill     = NULL;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    /* ����IDֻ���Ǵ�0��4������0���������� */
    if (id > SKILL_MAX) {
        TRACE_BATTLE(ERROR_FORMAT "%s cannot use the skill,"
                     "skill id is %u!\n",
                     self->name, id);
        return ERR_UNKNOW_SKILL;
    }
    
    skl_name = self->skills[id].attr.name;
    /*
     * �жϵ�ǰĿ���ܷ��Ϊ�����������类���ȴ������ˣ�
     * �Ͳ��ܳ�Ϊ��������
     */
    if (!IS_CAN_BE_ATTACKED(target)) {
        TRACE_BATTLE(INFO_FORMAT "%s cannot be attacked!\n",
                     target->name);
        return ERR_CANNOT_BE_ATTACKED;
    }

    /*
     * �жϼ����Ƿ�ֻ�ܶԵз���λ�ͷ�
     */
    if (IS_SKILL_ENEMY_ONLY(&(self->skills[id].attr))
        && (GET_CAMP(self) == GET_CAMP(target))) {
        TRACE_BATTLE(INFO_FORMAT "%s just only work to enemy!\n",
                     skl_name);
        return ERR_WRONG_OBJ;
    }
 
    /*
     * �ж��Լ��Ƿ�����ͷż��ܣ��������ʩ�����жϸü����Ƿ�
     * �Ǳ������ܣ���������ֻҪ�������㼴���ͷţ�����ʩ�����
     * λ������
     */
    if (!CAN_MAGIC_ATTACK(self)
        && !IS_SKILL_PASSIVE(&(self->skills[id].attr))) {
        TRACE_BATTLE(INFO_FORMAT "%s cannot magic attack!\n",
                     self->name);
        return ERR_CANNOT_MAGIC_ATTACK;
    }

    /*
     * �ж�Ŀ���Ƿ�ħ�����ߣ����жϱ������Ƿ�����ħ�⣬��Ŀ��
     * ������ħ������״̬�Ҹü��ܲ�������ħ�����ߣ��򷵻�
     */
    if (IS_MAIGC_IMMUNITY(target)
        && !IS_SKILL_IMI(&(self->skills[id].attr))) {
        TRACE_BATTLE(INFO_FORMAT "%s is magic immunity!\n",
                     target->name);
        return ERR_MAGIC_IMMUNITY;
    }

    /* ��ȡ�ͷŸü�����Ҫ���ĵ�ħ��ֵ */
    mana = self->skills[id].attr.mana;
    /* ���ħ��ֵ���������¼��־�󷵻�ħ��ֵ�����Ĵ��� */
    if (self->cur_hmaa.mana < mana) {
        TRACE_BATTLE(INFO_FORMAT "%s use %s is out of mana"
                     "(MUST: %hu, CUR: %hu)!\n",
                     self->name,
                     skl_name,
                     mana,
                     self->cur_hmaa.mana);
        return ERR_OUT_OF_MANA;
    }

    cold_down = self->skills[id].attr.cold_down;
    used_time = self->skills[id].used_time;
    cur_time = get_current_time();

    /*
     * �жϼ����Ƿ���CD֮�У����ʹ��ʱ��Ϊ0˵���ü����ǵ�һ
     * ��ʹ�á���������һ�ε�ʹ��ʱ����ϸü��ܵ���ȴʱ���Ѿ�
     * С�ڵ�ǰʱ�䣬˵����������ȴOK��
     */
    if (used_time && (used_time + cold_down > cur_time)) {
        TRACE_BATTLE(INFO_FORMAT "%s use %s is cold down"
                     "(LAST USE: %04llu, CD: %hu)!\n",
                     self->name,
                     skl_name,
                     used_time,
                     cold_down);
        return ERR_COLD_DOWN;
    }

    /*
     * ����������һЩ�жϣ�������֤��ʩ������Ч�ԣ�
     * �������жϼ��ܱ����Ƿ���Ч
     */
    skill = self->skills[id].attr.skill;
    if (NULL == skill) {
        TRACE_BATTLE(ERROR_FORMAT "%s use %s is invalid!\n",
                     self->name,
                     skl_name);
        return ERR_INVALID_SKILL;
    }

    /* ͨ��ע��ĺ���ָ�룬���ͷŶ�Ӧ�ļ��� */
    ret = skill(self);

    switch(ret) {
        /*
         * ĳЩ�����ͷź���ĳЩ�����²��������ȴ״̬�����۳�ħ��ֵ��
         * ����¼ʩ��ʱ�䣬��Ȼ���á�
         */
        case SUC_IGNORE_CD:
            TRACE_BATTLE("%s use %s OK.\n",
                         self->name,
                         skl_name);
            TRACE_BATTLE("ignore the cd of skill %s.\n", skl_name);
            self->cur_hmaa.mana -= mana;
        break;

        /*
         * ����ͷųɹ�������Ҫ�۳���Ӧ��ħ�������Ҽ�¼ʩ��ʱ��
         */
        case DOTA_SUCCESS:
            TRACE_BATTLE("%s use %s OK.\n",
                         self->name,
                         skl_name);
            self->cur_hmaa.mana -= mana;
            self->skills[id].used_time = cur_time;
        break;

        /*
         * �����ͷ�ʧ�ܣ���ӡʧ�ܴ�����
         */
        default:
            TRACE_BATTLE("%s use %s faild, ERROR CODE: %d.\n",
                         self->name, skl_name, ret);
        break;
    }
    return ret;
}


/* Ĭ�ϵ�ʹ����Ʒ�Ĳ��� */
int default_equip(LIFE_S *self, unsigned int id)
{
    unsigned short     mana  = 0;
    unsigned short     cold_down  = 0;
    unsigned long long cur_time  = 0;
    unsigned long long used_time = 0;
    int                ret        = 0;
    const char        *eqp_name   = NULL;
    OPER1_PF           skill     = NULL;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    if (id >= EQUIP_MAX) {
        TRACE_BATTLE(ERROR_FORMAT "%s cannot use the equipment,"
                     "index is %u!\n", self->name, id);
        return ERR_UNKNOW_EQP;
    }
    eqp_name = self->equips[id].eqp->skl_attr.name;

    if (!IS_CAN_BE_ATTACKED(target)) {
        TRACE_BATTLE(INFO_FORMAT "%s cannot be attacked!\n",
                     self->name);
        return ERR_CANNOT_BE_ATTACKED;
    }

    if (!CAN_USE_EQUIPMENT(self)) {
        TRACE_BATTLE(INFO_FORMAT "%s cannot use equipment.\n",
                     self->name);
        return ERR_CANNOT_USE_EQUIPMENT;
    }

    /*
     * �ж���Ʒ�Ƿ�ֻ�ܶԵз���λ�ͷ�
     */
    if (IS_SKILL_ENEMY_ONLY(&(self->equips[id].eqp->skl_attr))
        && (GET_CAMP(self) == GET_CAMP(target))) {
        TRACE_BATTLE(INFO_FORMAT "%s just only work to enemy!\n",
                     eqp_name);
        return ERR_WRONG_OBJ;
    }

    if (IS_MAIGC_IMMUNITY(target) 
        && !IS_SKILL_IMI(&(self->equips[id].eqp->skl_attr))) {
        TRACE_BATTLE(INFO_FORMAT "%s is magic immunity!\n",
                     target->name);
        return ERR_MAGIC_IMMUNITY;
    }

    mana = self->equips[id].eqp->skl_attr.mana;
    if (self->cur_hmaa.mana < mana) {
        TRACE_BATTLE(INFO_FORMAT "%s use %s is out of mana"
                     "(MUST: %hu, CUR: %hu)!\n",
                     self->name, eqp_name,
                     mana, self->cur_hmaa.mana);
        return ERR_OUT_OF_MANA;
    }

    cold_down = self->equips[id].eqp->skl_attr.cold_down;
    used_time = self->equips[id].used_time;
    cur_time = get_current_time();
    if (used_time && (used_time + cold_down > cur_time)) {
        TRACE_BATTLE(INFO_FORMAT "%s use %s is cold down"
                     "(LAST USE: %04llu, CD: %hu)!\n",
                     self->name,
                     eqp_name,
                     used_time,
                     cold_down);
        return ERR_COLD_DOWN;
    }

    skill = self->equips[id].eqp->skl_attr.skill;
    if (!skill) {
        TRACE_BATTLE(ERROR_FORMAT "%s use %s is invalid!\n",
                     self->name, eqp_name);
        return ERR_INVALID_EQP;
    }

    ret = skill(self);

    switch(ret) {
        /*
         * �����Ʒ�ͷųɹ�������Ҫ�۳���Ӧ��ħ�������Ҽ�¼ʩ��ʱ��
         */
        case DOTA_SUCCESS:
            TRACE_BATTLE("%s use %s OK.\n",
                         self->name,
                         eqp_name);
            self->cur_hmaa.mana -= mana;
            self->equips[id].used_time = cur_time;
        break;

        /*
         * ��Ʒ�ͷ�ʧ�ܣ���ӡʧ�ܴ�����
         */
        default:
            TRACE_BATTLE("%s use %s faild, ERROR: %d.\n",
                         self->name,
                         eqp_name,
                         ret);
        break;
    }
    return ret;
}



/* Ĭ�ϵ�����buff & debuff�Ĳ��� */
int default_execbuff(LIFE_S *self)
{
    LIST_HEAD_S       *item       = NULL;
    LIST_HEAD_S       *temp       = NULL;
    LIST_HEAD_S       *head       = NULL;
    BUFF_NODE_S       *buff_node  = NULL;
    SKILL_PF           clean_buff = NULL;
    SKILL_PF           do_buff    = NULL;
    unsigned long long cur_time   = 0;
    unsigned long long start      = 0;
    unsigned long long last       = 0;
    unsigned short     interval   = 0;
    unsigned short     duration   = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    head = &(self->buff_list);
    cur_time = get_current_time();

    /*
     * �������������е�BUFF�ڵ�
     */
    LIST_FOR_EACH_SAFE(item, temp, head) {

        buff_node = LIST_ENTRY(item, BUFF_NODE_S, buff_list);
        last = buff_node->last_exec;
        interval = buff_node->exec_interval;

        /*
         * ��һ�εĻ�������ִ��buff��do����������
         * ��ÿ�����ʱ�䵽��ִ��do����
         */
        if (!last || (last + interval <= cur_time)) {
            do_buff = buff_node->do_buff;
            if (do_buff)
                (void)do_buff(self, buff_node->owner);

            TRACE_BATTLE("%s do %s buff.\n",
                         self->name,
                         buff_node->buff_name);
            buff_node->last_exec = cur_time;
        }

        start = buff_node->start;
        duration = buff_node->duration;
        /*
         * ��ĳ��buff��ʱ����˵�ǹ����ˣ���Ҫ�����Ƴ�
         */
        if (start + duration <= cur_time) {
            list_del(item);
            clean_buff = buff_node->clean_buff;
            if (clean_buff)
                (void)clean_buff(self, buff_node->owner);

            TRACE_BATTLE("%s clean %s buff.\n",
                         self->name,
                         buff_node->buff_name);
            DOTA_FREE(buff_node);
        }
    }
    return DOTA_SUCCESS;
}


int default_clean(LIFE_S *self)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    TRACE_BATTLE("Clean %s.\n", self->name);

    /* �������е�BUFF�ڵ� */
    clean_all_buff(self);
    queue_free(self->kill_queue);
    return DOTA_SUCCESS;
}


/* Ұ�ֵ�ɱ�˲��� */
int wilders_kill(LIFE_S *self)
{
    LIFE_S *target = NULL;
    QUEUE_S *queue = NULL;
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    queue = self->kill_queue;
    while (!queue_empty(queue)) {
        target = (LIFE_S *)queue_out(queue);

        if (!target || self == target)
            continue;

        TRACE_BATTLE("%s killed %s!\n",
                     self->name,
                     target->name);
    }
    return DOTA_SUCCESS;
}


int wilders_death(LIFE_S *self)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    DOTA_RETURN_IF((LIFE_ZOMBIE != self->life_state), ERR_WRONG_OBJ);

    clean_all_buff(self);

    RESET_HAMM(&(self->cur_hmaa));

    if (self == self->murderer) {
        TRACE_BATTLE("%s is suicide!\n", self->name);
    } else {
        TRACE_BATTLE("%s is dead!\n", self->name);
    }

    self->life_state = LIFE_DEAD;
    return DOTA_SUCCESS;
}
