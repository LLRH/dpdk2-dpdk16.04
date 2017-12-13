#ifndef _CoLoR_H_
#define _CoLoR_H_

#define CONTENTLEN 4
#define PUBKEYLEN 4

typedef struct CoLoR_get
{
    uint8_t version_type;	//�汾4λ������4λ
    uint8_t ttl;		//����ʱ��
    uint16_t total_len;	//�ܳ���
    uint16_t port_src;	//Դ�˿ں�
    uint16_t port_dst;	//Ŀ�Ķ˿ں�
    uint16_t minmal_PID_CP; //pid�ı������
    uint8_t PIDs;		//PID����Ŀ
    uint8_t Offest_RES;     //λ����ȡOffset
    uint32_t offset;	//ƫ����
    uint32_t length;	//ƫ�Ƴ���
    uint16_t content_len;	//��Կ����
    uint16_t mtu;		//����䵥Ԫ
    uint16_t publickey_len;	//��Կ����
    uint16_t checksum;	//�����
    uint8_t nid_sid[16];	//NID part of an SID������Ϊ16�ֽ�
    uint8_t l_sid[20]; 	//SID�ĳ���Ϊ20�ֽ�
    uint8_t nid[16];	//NID�ĳ���Ϊ16�ֽ�
    uint8_t content[CONTENTLEN];	// Content characteristics
    uint8_t publickey[PUBKEYLEN];	//��Կ
} CoLoR_get_t;

/*version_type�ֶ�*/
#define TYPE_GET 0xA0
#define TYPE_DTAT 0xA1
//#define TYPE_REGISTER 0xA2
#define TYPE_CONTROL 0xA3

/*control_type�ֶ�*/
#define control_type_register 0
#define control_tpye_announce 1

#define NID_LENGTH 16
struct control_public_header {
    uint8_t version_type;
    uint8_t control_type;
    uint8_t total_length;
    uint16_t port_number_1;
    uint16_t port_number_2;
    uint16_t min_pid_change_period;
    uint8_t ack_flag_res;
    uint8_t pid_index;
    uint8_t item_number;
    uint16_t checksum;
    uint8_t nids_s[NID_LENGTH];
    uint8_t nid_r[NID_LENGTH];
    uint32_t mac;
    uint32_t offset;
    uint32_t length;
    //CONTENT OF ITEM;
}__attribute__((__packed__));
typedef struct control_public_header control_public_header_t;

//TODO:ע�����TYPE�ֶε�����
#define REGISTER_TYPE_ADD 0x01
#define REGISTER_TYPE_UPDATE 0x02
#define REGISTER_TYPE_DELETE 0x03

//TODO:ע�����ʱ��ĵ�λ
#define REISTER_TIME_UNIT_SECOND    0x1
#define REISTER_TIME_UNIT_MINITUE   0x2
#define REISTER_TIME_UNIT_HOUR      0x3
#define REISTER_TIME_UNIT_DAY       0x4
#define REISTER_TIME_UNIT_WEEK      0x5
#define REISTER_TIME_UNIT_MONTH     0x6
#define REISTER_TIME_UNIT_YEAR      0x7

#define L_SID_LENGTH 20
struct control_register{
    //SID����
    uint8_t n_sid[NID_LENGTH];
    uint8_t l_sid[L_SID_LENGTH];
    //������:ע��(0x01)������(0x02)��ɾ��(0x03)
    //����ACK:SUCCESS(0x10), DUPLICATE(0x20), INVALIDE(0x30)
    uint8_t type;
    //�ṩ�ߵ�NID
    uint8_t nid_s[NID_LENGTH];
    //ע��ķ�Χ:Ĭ���ϼ�(0x01),�ϼ��ͶԵ���(0x02)�����������(0x03)��������(0x04)
    uint8_t scope;
    //ע�����Ч��
    uint8_t time_of_validity;
    //ע��ĵ�λʱ��:��(0x01)������(0x02)��Сʱ(0x03)����(0x04)������(0x05)����(0x06)����(0x07)
    uint8_t time_unit;
    //ע�����ݵ��ֽ���
    uint32_t content_size;
    //Classification of content: txt(0x1),image(0x2),video(0x3)
    uint32_t content_classification;
};
typedef struct control_register control_register_t;

//������������·��������
#define MAX_NUM_PID 8
//����ͨ������������������·����������
#define MAX_NUM_AS_PATH  16
struct control_announce{
    //pidͨ��Э��: ǿ������(0x1)���������(0x2), ���ο�(0x3)
    uint8_t pid_agreement;
    //ͨ�淽��PIDϲ�ö��б�
    uint8_t pid_number;
    uint32_t pid[MAX_NUM_PID];
    uint8_t pid_preference[MAX_NUM_PID];
    //ͨ���·������
    uint8_t as_number;
    uint16_t as[MAX_NUM_AS_PATH];
    //ͨ������������
    uint8_t announce_ttl;
    //ע�����ݵĸ���,����ͨ��һϵ�е����ݡ���
    uint16_t register_number;
    //Ĭ�϶���һ��ע�����ݣ�����ɱ䳤����
    struct control_register register_first;
    //�䳤���֡������Ը��������ע����Ŀ
};

#endif