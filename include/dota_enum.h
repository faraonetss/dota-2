/***********************************************************************************************************************
*  FOR DOTA, FOR ICEFROG!
*
*  Copyright (c) xiaomaoshi, all rights reserved.
*
*  Email: mym_cocacola@163.com
*
* File name: dota_enum.h
* Describe: all enum codes in dota were described at here.
* Author: xiaomaoshi
* Action: create file
* Date: 2015/05/20
* History:
***********************************************************************************************************************/

#ifndef __DOTA_ENUM_H__
#define __DOTA_ENUM_H__

#define DOTA_OK            0
#define DOTA_FAILED        1

#define DOTA_SUCCESS_START 0x1000 // �ɹ���ʼ
#define DOTA_SUCCESS_END   0x1FFF // �ɹ�����

#define DOTA_ERROR_START   0x2000 // ����ʼ
#define DOTA_ERROR_END     0x2FFF // ����ʼ

#define DOTA_INVALID       0xFFFF // ��Чֵ

typedef enum tag_success
{
    DOTA_SUCCESS = DOTA_SUCCESS_START,   // �ͷųɹ�
    SUC_IGNORE_CD,                       // �ɹ����Һ�����ȴ

    SUC_BUTT,
}SUCCESS_E;

typedef enum tag_error
{
    ERR_NULL_POINTER = DOTA_ERROR_START, // ��ָ��
    ERR_MALLOC_FAILED,                   // �����ڴ�ʧ��
    ERR_INVALID_PARAMETER,               // ��Ч�Ĳ���

    ERR_OUT_OF_HEALTH,                   // ����ֵ����
    ERR_OUT_OF_MANA,                     // ħ��ֵ����
    ERR_COLD_DOWN,                       // ��ȴ��
    ERR_WRONG_OBJ,                       // ����Ķ���
    ERR_MAGIC_IMMUNITY,                  // ħ������
    ERR_TRIGGER_FAILED,                  // ����ʧ��

    ERR_CANNOT_BE_ATTACKED,              // Ŀ�겻�ܱ�����
    ERR_CANNOT_MAGIC_ATTACK,             // ���ܽ���ħ������
    ERR_CANNOT_PHYSICAL_ATTACK,          // ���ܽ���������
    ERR_CANNOT_USE_EQUIPMENT,            // ����ʹ����Ʒ

    ERR_UNKNOW_SKILL,                    // δ֪�ļ���
    ERR_INVALID_SKILL,                   // ��Ч�ļ���
    ERR_NO_ENEMY,                        // û���ҵ��з���λ
    ERR_UNKNOW_EQP,                      // δ֪����Ʒ
    ERR_INVALID_EQP,                     // ��Ч����Ʒ
    ERR_INVALID_BUFF,                    // ��Ч��BUFF

    ERR_SHOULD_OVER,                     // ��ʾĳ����λӦ�ý���

    ERROR_BUTT,                          // ��Ҫ�ٴ˺�����Ӵ�����
}ERROR_E;

typedef enum
{
    LIFE_BIRTH,
    LIFE_RUNNING,
    LIFE_ZOMBIE,
    LIFE_DEAD,

    STATE_BUTT,
}LIFE_STATE_E;
#endif
