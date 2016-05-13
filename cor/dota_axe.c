/***********************************************************************************************************************
*  FOR DOTA, FOR ICEFROG!  
*
*  Copyright (c) xiaomaoshi, all rights reserved.
*
*  Email: mym_cocacola@163.com
*
* File name: dota_axe.c
* Describe: HERO AXE
* Author: xiaomaoshi
* Action: create file
* Date: 2015/05/20
* History:
***********************************************************************************************************************/

#include "dota_data.h"
#include "dota_enum.h"
#include "dota_equipment.h"
#include "dota_time.h"
#include "dota_log.h"
#include "dota_life.h"
#include "dota_operate.h"
#include "dota_fsm.h"

/* �����ĸ����ܵ�ID */
#define AXE_E 1
#define AXE_R 2
#define AXE_X 3
#define AXE_C 4

/* ������̭֮�У���Ѫ������400ֱ��նɱ */
#define AXE_C_DAMAGE_LOWER_LIMIT 300
#define AXE_C_DAMAGE_UPPER_LIMIT 400

/* �����������˺� */
#define AXE_X_DAMAGE      205
/* ������20% */
#define AXE_X_POSSIBILITY 20

#define AXE_R_DAMAGE_PER_SECOND 40

/* ������սʿ��ŭ��Ч������������40�㻤�� */
#define AXE_E_INCREASE_ARMOR    40
/* ������սʿ��ŭ��Ч����Ŀ�겻���ƶ�������ʩ��������ʹ����Ʒ */
#define AXE_E_TARGET_EFFECT \
    (MOVE_MASK | EQUIP_MASK | MAGIC_ATTACK_MASK)

/* �����Ĵ������ɸ�ɺ����쵱���� */
#define AXE_NAME "Mogul Kahn"
#define AXE_LEVEL 16

/* ������ʼ��Χ */
#define AXE_ORG_AGILITY  55
#define AXE_ORG_STRENGTH 64
#define AXE_ORG_MENTAL   44

#define AXE_ORG_HEALTH   1366
#define AXE_ORG_MANA     572
#define AXE_ORG_ARMOR    6
#define AXE_ORG_ATTACK   92

/* �����Ĺ������Ϊ1.2�룬Ϲд�ģ���Ҳ��֪������ */
#define AXE_ATTACK_INTERVAL 12


/* ����ʹ����Ʒ������ʩ�� */
int axe_e_target_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    self->state &= (~AXE_E_TARGET_EFFECT);
    self->target = owner;
    return DOTA_SUCCESS;
}

/* �������Ч����������ʹ����Ʒ������ʩ�� */
int axe_e_clean_target_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    self->state |= AXE_E_TARGET_EFFECT;
    self->target = NULL;
    return DOTA_SUCCESS;
}

/* ����40�㻤�ף�����3�� */
int axe_e_self_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    self->cur_hmaa.armor += AXE_E_INCREASE_ARMOR;
    return DOTA_SUCCESS;
}


/* ���40�㻤�� */
int axe_e_clean_self_buff(LIFE_S *self, LIFE_S *owner)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* ����40�㻤�� */
    if (self->cur_hmaa.armor < AXE_E_INCREASE_ARMOR) {
        self->cur_hmaa.armor = 0;
        return DOTA_SUCCESS;
    }
    self->cur_hmaa.armor -= AXE_E_INCREASE_ARMOR;
    return DOTA_SUCCESS;
}

/***********************************************************************************************************************
* Function name: axe_e
* Skill name: ��սʿ��ŭ��
* Describe: ��������Χ���˵ĺ��⼯�е��Լ����ϣ������ǲ����κδ��۶�Ҫ��������
* Mana Cost: 110��
* Cold Down: 100��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int axe_e(LIFE_S *self)
{
    BUFF_NODE_S *buff_node = NULL;
    LIFE_S **enemy = NULL;
    LIFE_S **all_enemy = NULL;
    unsigned int dis = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* �������40�㻤�׵�BUFF */
    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->owner = self;
    buff_node->buff_name = self->skills[AXE_E].attr.name;
    buff_node->duration = self->skills[AXE_E].attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = 0;
    buff_node->do_buff = axe_e_self_buff;
    buff_node->clean_buff = axe_e_clean_self_buff;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_ONETIME;

    /* Ч�������ӣ���Ҫ����ԭ����buff�������еĻ� */
    (void)clean_buff_byname(self, buff_node->buff_name);
    add_buff_node(buff_node, &(self->buff_list));

    dis = self->skills[AXE_E].attr.dis;
    all_enemy = find_enemy_area(self, dis);
    if (!all_enemy)
        return DOTA_SUCCESS;

    enemy = all_enemy;
    while (*enemy) {
        /* �����˼�buff�������ƶ���ʩ���Լ�ʹ����Ʒ */
        buff_node = create_buff_node();
        DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

        buff_node->owner = self;
        buff_node->buff_name = self->skills[AXE_E].attr.name;
        buff_node->duration = self->skills[AXE_E].attr.duration;
        buff_node->start = get_current_time();
        buff_node->last_exec = 0;
        buff_node->do_buff = axe_e_target_buff;
        buff_node->clean_buff = axe_e_clean_target_buff;
        buff_node->exec_interval = HERO_BUFF_INTERVAL_MIN;

        /* Ч�������ӣ���Ҫ����ԭ����buff�������еĻ� */
        (void)clean_buff_byname(*enemy, buff_node->buff_name);
        add_buff_node(buff_node, &((*enemy)->buff_list));
        enemy++;
    }
    DOTA_FREE(all_enemy);
    return DOTA_SUCCESS;
}


/* �ܵ�30��/����˺�������16�� */
int axe_r_buff(LIFE_S *self, LIFE_S *owner)
{
    unsigned short damage = 0;
    unsigned short spell  = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);
    DOTA_RETURN_IF_NULL(owner, ERR_NULL_POINTER);

    spell = self->ms.spell_rst;
    damage = HERO_MAGIC_DAMAGE(AXE_R_DAMAGE_PER_SECOND, spell);
    TRACE_BATTLE("%s's buff AXE_R make %u damage to %s.\n",
                 owner->name, damage, self->name);

    if (self->cur_hmaa.health <= damage) {
        self->life_state = LIFE_ZOMBIE;
        self->murderer = owner;
        queue_in(owner->kill_queue, self);
        return DOTA_SUCCESS;
    }
    self->cur_hmaa.health -= damage;
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name: axe_r
* Skill name: ս������
* Describe: �ܵ�30��/����˺�������16�롣
* Mana Cost: 105��
* Cold Down: 19��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int axe_r(LIFE_S *self)
{
    BUFF_NODE_S *buff_node = NULL;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->buff_name = self->skills[AXE_R].attr.name;
    buff_node->duration = self->skills[AXE_R].attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = buff_node->start;
    buff_node->do_buff = axe_r_buff;
    buff_node->owner = self;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_DEFAULT;

    /* Ч�������ӣ���������ԭ����ս������buff�������еĻ� */
    (void)clean_buff_byname(target, buff_node->buff_name);
    add_buff_node(buff_node, &(target->buff_list));
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name: axe_x
* Skill name: ��������
* Describe: ���ܵ�����ʱ��������20%�ļ���ץס���ˣ��ֱ���˦�����Ը���300��Χ�ڵ�
*           �з���λ���205���˺����˺�����ħ�����ߡ�
* Mana Cost: 0��
* Cold Down: 0.3��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int axe_x(LIFE_S *self)
{
    int rand_num = 0;
    unsigned short damage = 0;
    unsigned short armor = 0;
    LIFE_S **enemy = NULL;
    LIFE_S **all_enemy = NULL;
    unsigned int dis = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    rand_num = (rand() % 100) + 1;
    if (rand_num > AXE_X_POSSIBILITY)
        return ERR_TRIGGER_FAILED;

    dis = self->skills[AXE_X].attr.dis;
    all_enemy = find_enemy_area(self, dis);
    if (!all_enemy)
        return DOTA_SUCCESS;

    enemy = all_enemy;
    while (*enemy) {
        armor = (*enemy)->cur_hmaa.armor;
        damage = HERO_PHYSICAL_DAMAGE(AXE_X_DAMAGE, armor);
        TRACE_BATTLE("%s use AXE_X make %u damage to %s.\n",
                     self->name, damage, (*enemy)->name);

        if ((*enemy)->cur_hmaa.health <= damage) {
            (*enemy)->life_state = LIFE_ZOMBIE;
            (*enemy)->murderer = self;
            queue_in(self->kill_queue, *enemy);
            enemy++;
            continue;
        }
        (*enemy)->cur_hmaa.health -= damage;
        enemy++;
    }
    DOTA_FREE(all_enemy);
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name: Axe_C
* Skill name: ��̭֮��
* Describe: ����������߷�Χ�ڵ����ߣ��˺������ߣ�����˲��նɱ����ֵ����ĵ��ˡ�
*           ˲��նɱ�ж�����ħ�����ߡ����300���˺���˲��նɱ����ֵ����400�ĵ��ˡ�
* Mana Cost: 180��
* Cold Down: 55��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int axe_c(LIFE_S *self)
{
    unsigned short damage = 0;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    /* Ѫ������400�Ļ���˲��նɱ�������ǲ���ħ������ */
    if (target->cur_hmaa.health <= AXE_C_DAMAGE_UPPER_LIMIT) {
        TRACE_BATTLE("%s use AXE_C make %u damage to %s.\n",
                     self->name,
                     AXE_C_DAMAGE_UPPER_LIMIT,
                     target->name);
        target->life_state = LIFE_ZOMBIE;
        target->murderer = self;
        queue_in(self->kill_queue, target);
        return SUC_IGNORE_CD;
    }

    /* 
     * ���Է�Ѫ���ܶ಻��նɱ�����ҶԷ�����ħ�����ߵ�״̬��
     * �򲻻�����˺���������Ҳ��CD���൱���Ѿ��ų�����
     */
    if (IS_MAIGC_IMMUNITY(target)) {
        TRACE_BATTLE("%s use AXE_C make %u damage to %s.\n",
                     self->name, 0, target->name);
        return DOTA_SUCCESS;
    }

    /* Ѫ������400���Ҳ�����ħ��״̬ʱ�������300���˺� */
    damage = HERO_PHYSICAL_DAMAGE(AXE_C_DAMAGE_LOWER_LIMIT,
                                  target->cur_hmaa.armor);
    TRACE_BATTLE("%s use AXE_C make %u damage to %s.\n",
                 self->name, damage, target->name);
    target->cur_hmaa.health -= damage;
    return DOTA_SUCCESS;
}


/* ��������ᰴ��һ���Ĳ����ͷ���Ʒ�ͼ��� */
int axe_policy(LIFE_S *self)
{
    int ret = DOTA_INVALID;
    int tmp = DOTA_INVALID;
    OPERATE_S *self_ops;
    LIFE_S *target;
    BUFF_NODE_S *node;

    self_ops = self->ops;

    target = self->target;
    /* Ŀ���Ѫ���Ƿ����400�����������ͷ���̭֮�У�˲��նɱ */
    if (target->cur_hmaa.health <= AXE_C_DAMAGE_UPPER_LIMIT) {
        ret = self_ops->skill(self, AXE_C);
        DOTA_RETURN_IF_SUCCESS(ret);
    }

    /* 
     * �Ƿ����������£�������ᴥ������������������������Խ�࣬
     * �����ĸ���Խ��physical attack��buff�ǲ�����������ģ�����
     * ���ݸ�buff��������֪ĳһʱ�̹���������������
     */
    while (NULL != (node = find_buff_byname(self, NAME_PHY_ATK))) {
        (void)clean_buff(self, node);
        tmp = self_ops->skill(self, AXE_X);
        if (DOTA_SUCCESS == tmp)
            ret = DOTA_SUCCESS;
    }
    DOTA_RETURN_IF_SUCCESS(ret);

    /* 
     * ��Ŀ��û����Ч�����ͷ�а�񵰵�����Ȼ���˰׷ţ�����Ǵ�����
     * AI�øɵ���
     */
    if ((NULL == find_buff_byname(target, NAME_GSV))
        && (NULL == find_buff_byname(target, "AXE_E"))) {
        ret = self_ops->equip(self, EQUIPMENT_1);
        DOTA_RETURN_IF_SUCCESS(ret);
    }

    /* �ͷ��򵶲��ԣ�û�б���ס��û��а�񵰵�Ч��ʱ���ͷ��� */
    if ((NULL == find_buff_byname(target, "AXE_E"))
        && (NULL == find_buff_byname(target, NAME_XMS))) {
        ret = self_ops->equip(self, EQUIPMENT_0);
        DOTA_RETURN_IF_SUCCESS(ret);
    }

    /* 
     * Ŀ��û�б���ס����û�б��ͷ�а�񵰵�ʱ���ͷſ�սʿŭ��
     * �������
     */
    if ((NULL == find_buff_byname(target, NAME_GSV))
        && (NULL == find_buff_byname(target, NAME_XMS))) {
        ret = self_ops->skill(self, AXE_E);
        DOTA_RETURN_IF_SUCCESS(ret);
    }

    /* ���ս�����ʵ�BUFF����ͦ���õģ����յ�����Ѫ */
    ret = self_ops->skill(self, AXE_R);
    DOTA_RETURN_IF_SUCCESS(ret);

    /* ������ */
    ret = self_ops->attack(self);
    DOTA_RETURN_IF_SUCCESS(ret);

    return ret;
}


int axe_init(LIFE_S *self)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* ��ʼ������ͷ */
    INIT_LIST_HEAD(&(self->buff_list));
    INIT_LIST_HEAD(&(self->life_list));

    self->kill_queue = queue_init(TMP_SIZE);
    if (!self->kill_queue)
        return ERR_MALLOC_FAILED;

    /* �ѵ���������װ���� */
    init_box(self);

    self->life_state = LIFE_RUNNING;
    return DOTA_OK;
}


FSM_NODE_S axe_fsm_nodes[] = {
    {RUN_START, default_run_start,
     RUN_EXEC_BUFF, RUN_OVER},

    {RUN_EXEC_BUFF, default_run_buff,
     RUN_FIND_TARGET, RUN_OVER},

    {RUN_FIND_TARGET, default_find_target,
     RUN_EXEC_POLICY, RUN_OVER},

    {RUN_EXEC_POLICY, axe_policy,
     RUN_EXIT, RUN_EXIT},

    {RUN_EXIT, NULL,
     RUN_EXIT, RUN_EXIT},

    {RUN_OVER, NULL,
     RUN_OVER, RUN_OVER},
};


FSM_S axe_fsm = {
    RUN_START,
    LIFE_PHASE,
    axe_fsm_nodes,
};


int axe_run(LIFE_S *self)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    return fsm_run(&axe_fsm, self);
}



SKILL_S axe_skill[] = {
    {{NAME_PHY_ATK, SKILL_NORMAL_IMI, 0, 0,
     AXE_ATTACK_INTERVAL, 2, 128, NULL}, 0},

    {{"AXE_E", SKILL_NORMAL_IMI, 0, 110, 100, 32, 275, axe_e}, 0},
    {{"AXE_R", SKILL_NORMAL_IMI, 0, 75, 190, 160, 900, axe_r}, 0},
    {{"AXE_X", SKILL_PASSIVE_IMI, 0, 0, 3, 0, 275, axe_x}, 0},
    {{"AXE_C", SKILL_NORMAL_IMI, 0, 200, 550, 0, 150, axe_c}, 0},
};

/* ����װ�����򵶺�Сëʩ��а�񵰵� */
EQUIP_LIFE_S axe_eqp[EQUIP_MAX] = {
    {&g_shop[EQP_GUINSOOS_SCYTHE_OF_VYSE], 0},
    {&g_shop[EQP_XIAOMAOSHI_EGGS_OF_EVIL], 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
};

OPERATE_S axe_op = {
    default_death,
    default_rebirth,
    default_kill,
    default_attack,
    default_skill,
    default_equip,
    default_execbuff,
    default_clean,

    axe_init,
    axe_run,
};

/*
 * miss designated initializer so much :(
 * �����������壬��ʼ�����鷳��
 */
LIFE_S axe_life = {
    AXE_NAME,
    AXE_LEVEL,
    HERO_STATE,
    HERO_STATE,
    SRG_HERO,

    0,
    0,
    HERO_ORGINAL_MONEY,
    {28, 28},
    NULL,

    {NULL, NULL},
    {NULL, NULL},

    {0, HERO_SPELL_RESISTANCE},
    {AXE_ORG_HEALTH, AXE_ORG_MANA, AXE_ORG_ARMOR, AXE_ORG_ATTACK},
    {AXE_ORG_HEALTH, AXE_ORG_MANA, AXE_ORG_ARMOR, AXE_ORG_ATTACK},
    {AXE_ORG_AGILITY, AXE_ORG_STRENGTH, AXE_ORG_MENTAL},
    {AXE_ORG_AGILITY, AXE_ORG_STRENGTH, AXE_ORG_MENTAL},

    /* ���ܼ� */
    axe_skill,

    /* װ���� */
    axe_eqp,

    /* ������ */
    &axe_op,

    NULL,
    NULL,
    NULL,
    LIFE_BIRTH,
};

/* ע�ắ�� */
int axe_register()
{
    return register_life(&axe_life);
}
