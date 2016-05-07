/***********************************************************************************************************************
*  FOR DOTA, FOR ICEFROG!
*
*  Copyright (c) xiaomaoshi, all rights reserved.
*
*  Email: mym_cocacola@163.com
*
* File name: dota_Es.c
* Describe: HERO ES
* Author: xiaomaoshi
* Action: create file
* Date: 2015/05/20
* History: 
***********************************************************************************************************************/

#include "dota_data.h"
#include "dota_enum.h"
#include "dota_equipment.h"
#include "dota_operate.h"
#include "dota_time.h"
#include "dota_log.h"
#include "dota_life.h"
#include "dota_fsm.h"

/* ��ţ�ĸ����ܵ�ID */
#define ES_F 1
#define ES_E 2
#define ES_A 3
#define ES_C 4

/* ������ţ�������ְ� */
#define ES_NAME "Earth Shaker"
#define ES_LEVEL 16

#define ES_C_DAMAGE         270
#define ES_C_JUMP_DAMAGE    70

#define ES_F_DAMAGE         275

/* ��ţ��ǿ��ͼ�����ӵĹ��������� */
#define ES_E_TIMES          4

#define ES_A_DAMAGE         125
/* ����ѣ��1.5�� */
#define ES_A_GIDDINESS_TIME 15

/* ��ţ��ʼ��Χ */
#define ES_ORG_AGILITY  35
#define ES_ORG_STRENGTH 67
#define ES_ORG_MENTAL   45

#define ES_ORG_HEALTH   1423
#define ES_ORG_MANA     585
#define ES_ORG_ARMOR    6
#define ES_ORG_ATTACK   101

/* ��ţ�Ĺ��������������1.4�� */
#define ES_ATTACK_INTERVAL 14

static int es_a(LIFE_S *self);

#if 1
/* ������ţ��ѣ��buffЧ�� */
int es_giddiness_effect(LIFE_S *self, LIFE_S *target)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    SET_GIDDINESS_EFFECT(self);
    return DOTA_SUCCESS;
}

/* ���������ţѣ��buffЧ�� */
int es_clean_giddiness(LIFE_S *self, LIFE_S *target)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    CLEAN_GIDDINESS_EFFECT(self);
    return DOTA_SUCCESS;
}

/* ������ţǿ��ͼ��Ч��������400%�Ĺ����� */
int es_e_effect(LIFE_S *self, LIFE_S *target)
{
    unsigned short attack = 0;
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* ����400%�Ĺ����� */
    attack = ES_E_TIMES * self->org_hmaa.attack;
    self->cur_hmaa.attack += attack;
    return DOTA_SUCCESS;
}

/* ���������ţǿ��ͼ��Ч�� */
int es_e_clean(LIFE_S *self, LIFE_S *target)
{
    unsigned short attack = 0;
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* �������BUFF��Ҫ�����Ĺ��� */
    attack = ES_E_TIMES * self->org_hmaa.attack;
    if (self->cur_hmaa.attack <= attack) {
         self->cur_hmaa.attack = 0;
         return DOTA_SUCCESS;
    }
    self->cur_hmaa.attack -= attack;
    return DOTA_SUCCESS;
}
#endif

/***********************************************************************************************************************
* Function name: es_f
* Skill name: ����
* Describe: ������ţ��ͼ��˺�ѵر��γɾ޴�Ĺ��֣�8���ڲ���ͨ�У�
*           ���275���˺���ѣ��1.75�롣������ɵ�ѣ�θ������ѣ��
*           �����ӡ�
* Mana Cost: 175��
* Cold Down: 10��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int es_f(LIFE_S *self)
{
    BUFF_NODE_S   *buff_node = NULL;
    unsigned short damage    = 0;
    LIFE_S *target = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    target = self->target;
    DOTA_RETURN_IF_NULL(target, ERR_NULL_POINTER);

    damage = HERO_MAGIC_DAMAGE(ES_F_DAMAGE, target->ms.spell_rst);
    TRACE_BATTLE("%s use ES_F make %hu damage to %s.\n",
                 self->name, damage, target->name);

    if (target->cur_hmaa.health <= damage) {
        target->life_state = LIFE_ZOMBIE;
        target->murderer = self;
        queue_in(self->kill_queue, target);
        return DOTA_SUCCESS;
    }

    target->cur_hmaa.health -= damage;

    /* �����µ�buff�ڵ㣬����������ѣ��Ч�� */
    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->buff_name = self->skills[ES_F].attr.name;
    buff_node->duration = self->skills[ES_F].attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = 0;
    buff_node->do_buff = es_giddiness_effect;
    buff_node->clean_buff = es_clean_giddiness;
    buff_node->owner = self;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_MIN;

    /* Ч�������ӣ������Ŀ������ͬ����buff�������еĻ� */
    (void)clean_buff_byname(target, buff_node->buff_name);
    add_buff_node(buff_node, &(target->buff_list));

    /* ������ţ����ɫ��ֻҪ�ͷż��ܣ���������(A)�����Զ��ͷ� */
    (void)es_a(self);
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name: es_e
* Skill name: ǿ��ͼ��
* Describe: �ں�����ţ���ϵ�ͼ����ע��������ʹ������ţ����һ�ι��������
*           ������˺���Ч������14���ά��һ�ι�����
* Mana Cost: 50��
* Cold Down: 5��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int es_e(LIFE_S *self)
{
    BUFF_NODE_S   *buff_node = NULL;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* ���Լ����ӹ�����buff */
    buff_node = create_buff_node();
    DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

    buff_node->buff_name = self->skills[ES_E].attr.name;
    buff_node->duration = self->skills[ES_E].attr.duration;
    buff_node->start = get_current_time();
    buff_node->last_exec = 0;
    buff_node->do_buff = es_e_effect;
    buff_node->clean_buff = es_e_clean;
    buff_node->owner = self;
    buff_node->exec_interval = HERO_BUFF_INTERVAL_ONETIME;

    /* Ч�������ӣ������ԭ����ǿ��ͼ��buff�������еĻ� */
    (void)clean_buff_byname(self, buff_node->buff_name);
    add_buff_node(buff_node, &(self->buff_list));

    /* ѣ�ε��˵�buff */
    (void)es_a(self);
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name: es_a
* Skill name: ����
* Describe: ÿ��������ţʩ�ż���ʱ������ʹ��Χ300��Χ�ڵĵ�����
*           ���125������˺���ѣ��1.5�롣
* Mana Cost: 0��
* Cold Down: 0��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int es_a(LIFE_S *self)
{
    BUFF_NODE_S   *buff_node = NULL;
    unsigned short damage   = 0;
    unsigned short spell = 0;
    LIFE_S **enemy = NULL;
    LIFE_S **all_enemy = NULL;
    unsigned int dis = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    dis = self->skills[ES_A].attr.dis;
    all_enemy = find_enemy_area(self, dis);
    if (!all_enemy)
        return DOTA_SUCCESS;

    enemy = all_enemy;
    while (*enemy) {

        spell = (*enemy)->ms.spell_rst;
        damage = HERO_MAGIC_DAMAGE(ES_A_DAMAGE, spell);

        TRACE_BATTLE("%s use ES_A make %u damage to %s.\n",
                     self->name, damage, (*enemy)->name);

        /* �Ե�������˺������ù����������򲻻�Ϊ������buff�ڵ� */
        if ((*enemy)->cur_hmaa.health <= damage) {
            (*enemy)->life_state = LIFE_ZOMBIE;
            (*enemy)->murderer = self;
            queue_in(self->kill_queue, *enemy);
            enemy++;
            continue;
        }
        (*enemy)->cur_hmaa.health -= damage;

        buff_node = create_buff_node();
        DOTA_RETURN_IF_NULL(buff_node, ERR_MALLOC_FAILED);

        /* �����÷�Χ�ڵ����е���ʩ��ѣ��Ч�� */
        buff_node->buff_name = self->skills[ES_A].attr.name;
        buff_node->duration = ES_A_GIDDINESS_TIME;
        buff_node->start = get_current_time();
        buff_node->last_exec = 0;
        buff_node->do_buff = es_giddiness_effect;
        buff_node->clean_buff = es_clean_giddiness;
        buff_node->owner = self;
        buff_node->exec_interval = HERO_BUFF_INTERVAL_MIN;

        /* 
         * ����Ŀ�������Ѿ���ѣ��buff����ʵ������Ҫ��������Կ���
         * ���²�����ѣ��buff�����еıȽϣ�������ʱ���������԰�
         * ԭ�����������
         */
        (void)clean_buff_byname(*enemy, buff_node->buff_name);
        add_buff_node(buff_node, &((*enemy)->buff_list));
        enemy++;
    }
   DOTA_FREE(all_enemy);
    return DOTA_SUCCESS;
}


/***********************************************************************************************************************
* Function name: es_c
* Skill name: ������
* Describe: ������ţ�ػ����淢���𲨣�����Χ�ĵз���λ���340������˺���
*           ���໥�������ɸ����˺����˺�����ħ�����ߡ�
* Mana Cost: 265��
* Cold Down: 110��
* Author: xiaomaoshi
* Date: 2015/05/20
* History:
***********************************************************************************************************************/
int es_c(LIFE_S *self)
{
    unsigned short damage = 0, tmp_dmg;
    unsigned short spell = 0;
    LIFE_S **enemy = NULL;
    LIFE_S **all_enemy = NULL;
    unsigned int dis = 0;
    unsigned int enmey_num = 0;

    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    dis = self->skills[ES_C].attr.dis;
    all_enemy = find_enemy_area(self, dis);
    if (!all_enemy)
        return DOTA_SUCCESS;

    enemy = all_enemy;
    /* ����з���λ������Խ����ɵĵ����˺�(JUMP)Խ�� */
    while (*enemy++)
        enmey_num++;

    enemy = all_enemy;
    /*
     * ��֪��������ν��㣬�����Է��֣�ֻ��һ������������£��˺�
     * ��145�����ң���ȥ��������������˺������е��˺���50������
     * ���ҳ������ˡ������˺�340����ֻ��һ����λ���������ʵ�ǰ�
     * �϶��е���һ�ε����˺�70�����ȥ�ˣ�Ĭ�ϵ�Ӧ��ֻ��270�㡣
     */
    //tmp_dmg = ES_C_DAMAGE + (enmey_num * ES_C_JUMP_DAMAGE);
    tmp_dmg = 125 + (enmey_num * ES_C_JUMP_DAMAGE);
    while (*enemy) {
        spell = (*enemy)->ms.spell_rst;
        damage = HERO_MAGIC_DAMAGE(tmp_dmg, spell);
        TRACE_BATTLE("%s use ES_C make %hu damage to %s.\n",
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
    (void)es_a(self);
    return DOTA_SUCCESS;
}


/* ��������ᰴ��һ���Ĳ����ͷ���Ʒ�ͼ��� */
int es_policy(LIFE_S *self)
{
    int ret = DOTA_INVALID;
    OPERATE_S *self_ops;
    LIFE_S *target;

    /* ��ȡ������ */
    self_ops = self->ops;

    target = self->target;

    /* ţͷ����ʱ�Ĺ������Կ϶��Ƕ���E��һ���� */
    ret = self_ops->skill(self, ES_E);
    DOTA_RETURN_IF_SUCCESS(ret);

    /* ������ʹ�������� */
    ret = self_ops->attack(self);
    if (DOTA_SUCCESS == ret) {
        /* ������OKʱ��������ǿ��ͼ�ڵ�BUFF���������еĻ� */
        (void)clean_buff_byname(self, "ES_E");
        return DOTA_SUCCESS;
    }

    /* ���E���������ͷŲ��ɹ��Ļ����ͷŹ���F */
    ret = self_ops->skill(self, ES_F);
    DOTA_RETURN_IF_SUCCESS(ret);

    /* ǰ�漼�ܶ��ͷ����ˣ����ǷŴ� */
    ret = self_ops->skill(self, ES_C);
    DOTA_RETURN_IF_SUCCESS(ret);

    /* Ŀ�겻����ѣ��״̬���ͷŷ��� */
    if ((NULL == find_buff_byname(target, "ES_A"))
        && (NULL == find_buff_byname(target, "ES_F"))) {
        /* ʹ�÷��� */
        ret = self_ops->equip(self, EQUIPMENT_0);
        DOTA_RETURN_IF_SUCCESS(ret);
    }
    return ret;
}


int es_init(LIFE_S *self)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    /* ��ʼ������ͷ */
    INIT_LIST_HEAD(&(self->buff_list));
    INIT_LIST_HEAD(&(self->life_list));

    self->kill_queue = (QUEUE_S *)malloc(sizeof(QUEUE_S));
    if (!self->kill_queue)
        return ERR_MALLOC_FAILED;

    queue_init(self->kill_queue, TMP_SIZE);

    /* �ѵ���������װ���� */
    init_box(self);

    self->life_state = LIFE_RUNNING;
    return DOTA_OK;
}


FSM_NODE_S es_fsm_nodes[] = {
    {RUN_START, default_run_start,
     RUN_EXEC_BUFF, RUN_OVER},

    {RUN_EXEC_BUFF, default_run_buff,
     RUN_FIND_TARGET, RUN_OVER},

    {RUN_FIND_TARGET, default_find_target,
     RUN_EXEC_POLICY, RUN_OVER},

    {RUN_EXEC_POLICY, es_policy,
     RUN_EXIT, RUN_EXIT},

    {RUN_EXIT, NULL,
     RUN_EXIT, RUN_EXIT},

    {RUN_OVER, NULL,
     RUN_OVER, RUN_OVER},
};

FSM_S es_fsm = {
    RUN_START,
    LIFE_PHASE,
    es_fsm_nodes,
};


int es_run(LIFE_S *self)
{
    DOTA_RETURN_IF_NULL(self, ERR_NULL_POINTER);

    return fsm_run(&es_fsm, self);
}

SKILL_S es_skill[] = {
    {{NAME_PHY_ATK, SKILL_NORMAL_IMI, 0, 0,
     ES_ATTACK_INTERVAL, 2, 128, NULL}, 0},

    {{"ES_F", SKILL_NORMAL, 0, 170, 150, 17, 1400, es_f}, 0},
    {{"ES_E", SKILL_NORMAL, 0, 50, 50, 140, 0, es_e}, 0},
    {{"ES_A", SKILL_PASSIVE, 0, 0, 0, 0, 300, es_a}, 0},
    {{"ES_C", SKILL_NORMAL,  0, 265, 1100, 0, 575, es_c}, 0},
};

EQUIP_LIFE_S es_eqp[EQUIP_MAX] = {
    {&g_shop[EQP_EUL_SCEPTER_OF_DIVINITY], 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
    {NULL, 0},
};

OPERATE_S es_op = {
    default_death,
    default_rebirth,
    default_kill,
    default_attack,
    default_skill,
    default_equip,
    default_execbuff,
    default_clean,

    es_init,
    es_run,
};

/* ES�������� */
LIFE_S es_life = {
    ES_NAME,
    ES_LEVEL,
    HERO_STATE,
    HERO_STATE,
    SEN_HERO,

    0,
    0,
    HERO_ORGINAL_MONEY,
    {25, 25},
    NULL,

    {NULL, NULL},
    {NULL, NULL},

    {0, HERO_SPELL_RESISTANCE},
    {ES_ORG_HEALTH, ES_ORG_MANA, ES_ORG_ARMOR, ES_ORG_ATTACK},
    {ES_ORG_HEALTH, ES_ORG_MANA, ES_ORG_ARMOR, ES_ORG_ATTACK},
    {ES_ORG_AGILITY, ES_ORG_STRENGTH, ES_ORG_MENTAL},
    {ES_ORG_AGILITY, ES_ORG_STRENGTH, ES_ORG_MENTAL},

    /* ���ܼ� */
    es_skill,

    /* װ���� */
    es_eqp,

    /* ������ */
    &es_op,

    NULL,
    NULL,
    NULL,
    LIFE_BIRTH,
};


/* ע�ắ�� */
int es_register()
{
    return register_life(&es_life);
}
