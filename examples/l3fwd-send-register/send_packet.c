#include "send_packet.h"

//在mian.c里面把这个结构体改成全局的变量 把static去掉
extern struct rte_mempool *pktmbuf_pool[NB_SOCKETS];

#include "CoLoR.h"
#include <stdlib.h>
#include <time.h>

static int
pktgen_ctor_ether_header(struct ether_hdr *eth, struct rte_mbuf *m) {
    struct ether_hdr *ether_header = eth;
    int i;
    uint8_t addr1[6] = {00, 0x16, 0x31, 0xfe, 0xe6, 0x90};
    uint8_t addr2[6] = {00, 0x16, 0x31, 0xfe, 0xe6, 0x91};
    for (i = 0; i < 6; i++) {
        ether_header->d_addr.addr_bytes[i] = addr2[i];
    }
    for (i = 0; i < 6; i++) {
        ether_header->s_addr.addr_bytes[i] = addr1[i];
    }
    ether_header->ether_type = 0x0008;
    memcpy(rte_pktmbuf_mtod_offset(m,
    struct ether_hdr*,0),ether_header, sizeof(struct ether_hdr));
    return sizeof(struct ether_hdr);
}

static int
pktgen_ctor_ip_header(struct ipv4_hdr *ip, struct rte_mbuf *m, uint8_t next_proto_id) {
    struct ipv4_hdr *ip_header = ip;
    ip_header->version_ihl = 0x45;
    ip_header->type_of_service = 0;
    ip_header->total_length = 0;
    ip_header->packet_id = 0;
    ip_header->fragment_offset = 0;
    ip_header->time_to_live = 4;
    ip_header->next_proto_id = next_proto_id;
    ip_header->hdr_checksum = 0;
    ip_header->src_addr = htonl(IPv4(192, 168, 1, 2));
    ip_header->dst_addr = htonl(IPv4(192, 168, 18, 24));
    memcpy(rte_pktmbuf_mtod_offset(m,
    struct ipv4_hdr*,sizeof(struct ether_hdr)),ip_header, sizeof(struct ipv4_hdr));
    return sizeof(struct ipv4_hdr);
}

//TODO:控制包包头公共部分
static int
pktgen_ctor_control_public_header(struct rte_mbuf *m) {
    control_public_header_t *control_public_hdr = rte_pktmbuf_mtod_offset(m, control_public_header_t * ,
                                                                          sizeof(struct ether_hdr) +
                                                                          sizeof(struct ipv4_hdr));

    control_public_hdr->version_type = TYPE_CONTROL;
    control_public_hdr->control_type = control_type_register;

    return sizeof(control_public_header_t);
}


unsigned last_rand = 0;
uint64_t mysrand(int min, int max) {
    srand((unsigned) (time(NULL) + last_rand));
    last_rand = rand();
    return rand() % (max - min + 1) + min;
}

//TODO:让GET包读取上次注册的内容
uint8_t l_sid_last[L_SID_LENGTH];

static int
pktgen_ctor_register(struct rte_mbuf *m,uint8_t type) {
    int i = 0;
    control_register_t *control_register_hdr = \
        rte_pktmbuf_mtod_offset(m, control_register_t * ,
                                sizeof(struct ether_hdr) +
                                sizeof(struct ipv4_hdr) +
                                sizeof(control_public_header_t));

    uint8_t n_sid[NID_LENGTH] = {0x0, 0x0, 0x0, 0x0, 0x1, 0x01, 0x1, 0x1,
                                 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3};
    for (i = 0; i < NID_LENGTH; i++) {
        n_sid[i] = mysrand(0x0, 0xFF);
    }

    uint8_t l_sid[L_SID_LENGTH] = {0x0, 0x0, 0x0, 0x0, 0x1, 0x01, 0x1, 0x1, 0x2, 0x2,
                                   0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x0, 0x0, 0x0, 0x1};

    for (i = 0; i < L_SID_LENGTH; i++) {
        //删除或者更新上一次的
        if(type==REGISTER_TYPE_DELETE ||type==REGISTER_TYPE_UPDATE){
            l_sid[i]=l_sid_last[i];
        }else{
            //REGISTER_TYPE_ADD
            l_sid[i] = mysrand('!', '~');
            l_sid_last[i] = l_sid[i];
        }
    }

    memcpy(control_register_hdr->n_sid, n_sid, NID_LENGTH);
    memcpy(control_register_hdr->l_sid, l_sid, L_SID_LENGTH);

    //control_register_hdr->type = mysrand(1, 3);
    //TODO:此为注册的类型
    control_register_hdr->type = type;

    uint8_t nid_s[NID_LENGTH] = {0x0, 0x0, 0x0, 0x0, 0x1, 0x01, 0x1, 0x1,
                                 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3};
    for (i = 0; i < NID_LENGTH; i++) {
        nid_s[i] = mysrand('!', '~');
    }

    memcpy(control_register_hdr->nid_s, nid_s, NID_LENGTH);

    //The following vaules are randomly generated.
    control_register_hdr->scope = mysrand(1, 4);
    control_register_hdr->time_of_validity = mysrand(1, 365);
    control_register_hdr->time_unit = mysrand(1, 7);
    control_register_hdr->content_size = mysrand(0, 0xFFFF);
    control_register_hdr->content_classification = mysrand(1, 3);

 /*   //打印注册的SID
    switch (mysrand(1, 1)) {
        case 1:
            printf("Receving REGISTER ACK--STATE=[\033[1;31;46m SUCCESS \033[0m ]\n SID=");
            break;
        case 2:
            printf("Receving REGISTER ACK--STATE=[\033[1;32;42m DUPLICATE \033[0m]\n SID=");
            break;
        case 3:
            printf("Receving REGISTER ACK--STATE=[\033[1;33;41m INVALID \033[0m]\n SID=");
            break;
        case 4:
            printf("No Receiving REGISTER ACK--STATE=[\033[1;34;47m TIMEOUT \033[0m]\n SID=");
            break;
    }*/

    for (i = 0; i < NID_LENGTH; i++) {
        printf("%2X:", n_sid[i]);
    }

    for (i = 0; i < L_SID_LENGTH; i++) {
        printf("%2X", l_sid[i]);
        if (i != L_SID_LENGTH - 1) {
            printf(":");
        } else {
            printf("\n");
        }
    }
    return sizeof(control_register_t);
}

/*
//:TODO:数据内容结构体分装函数
static void
pkt_setup(struct rte_mbuf *m) {
    int ret = 0;

    struct ether_hdr eth_hdr;
    ret = pktgen_ctor_ether_header(&eth_hdr, m);

    struct ipv4_hdr ipv4_hdr;
    ret += pktgen_ctor_ip_header(&ipv4_hdr, m, TYPE_CONTROL);

    ret += pktgen_ctor_control_public_header(m);
    ret += pktgen_ctor_register(m);

    m->nb_segs = 1;
    m->next = NULL;
    m->pkt_len = ret;
    m->data_len = ret;
}

//TODO: Demo函数
//TODO: 发送函数 ，想法是发送的时候带了数据的内容，这个需要优化一下
void inline send_mbuf(uint8_t portid, struct rte_mbuf *mbuf, unsigned lcore_id) {

    //TODO:申请内存空间
    if (lcore_id < 0) {
        lcore_id = rte_lcore_id();
    }
    uint8_t socketid = rte_lcore_to_socket_id(lcore_id);
    struct lcore_conf *qconf = &lcore_conf[lcore_id];
    if (pktmbuf_pool[socketid] == NULL) {
        rte_exit(EXIT_FAILURE, "pktmbuf_pool[socketid]==NULL\n");
    }
    struct rte_mbuf *m = rte_pktmbuf_alloc(pktmbuf_pool[socketid]);
    if (m == NULL) {
        rte_exit(EXIT_FAILURE, "Allocate Failure\n");
    }

    //TODO:对内存数据进行修改
    pkt_setup(m);

    //TODO:发送数据包
    send_single_packet(qconf, m, portid);
    rte_pktmbuf_free(m);
}
*/

//:TODO:数据内容结构体分装函数
static void
pkt_setup_REGISTER(struct rte_mbuf *m,uint8_t type) {
    int ret = 0;

    struct ether_hdr eth_hdr;
    ret = pktgen_ctor_ether_header(&eth_hdr, m);

    struct ipv4_hdr ipv4_hdr;
    ret += pktgen_ctor_ip_header(&ipv4_hdr, m, TYPE_CONTROL);

    ret += pktgen_ctor_control_public_header(m);
    ret += pktgen_ctor_register(m,type);

    m->nb_segs = 1;
    m->next = NULL;
    m->pkt_len = ret;
    m->data_len = ret;
}


//TODO:发送函数 ，想法是发送的时候带了数据的内容，这个需要优化一下
void inline
send_mbuf_register(uint8_t portid, struct rte_mbuf *mbuf, unsigned lcore_id,uint8_t type) {

    //TODO:申请内存空间
    if (lcore_id < 0) {
        lcore_id = rte_lcore_id();
    }
    uint8_t socketid = rte_lcore_to_socket_id(lcore_id);
    struct lcore_conf *qconf = &lcore_conf[lcore_id];
    if (pktmbuf_pool[socketid] == NULL) {
        rte_exit(EXIT_FAILURE, "pktmbuf_pool[socketid]==NULL\n");
    }
    struct rte_mbuf *m = rte_pktmbuf_alloc(pktmbuf_pool[socketid]);
    if (m == NULL) {
        rte_exit(EXIT_FAILURE, "Allocate Failure\n");
    }

    //TODO:对内存数据进行修改
    pkt_setup_REGISTER(m,type);

    //TODO:发送数据包
    send_single_packet(qconf, m, portid);
    rte_pktmbuf_free(m);
}

//TODO: 注册的类型 包类型:注册(0x01)，更新(0x02)，删除(0x03)
void inline send_register(uint8_t portid, struct rte_mbuf *mbuf, unsigned lcore_id,uint8_t type){
    send_mbuf_register(portid,mbuf,lcore_id,type);
}

/*******************get**********************************/
static int
pktgen_ctor_GET(struct rte_mbuf *m) {
    int i;

    CoLoR_get_t *get_hdr = rte_pktmbuf_mtod_offset(m, CoLoR_get_t * ,
                                                   sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr));
    get_hdr->version_type = TYPE_GET;
    get_hdr->PIDs = 0;

    for (i = 0; i < L_SID_LENGTH; i++) {
        get_hdr->l_sid[i] = l_sid_last[i];
    }
    return sizeof(CoLoR_get_t) + 4 * get_hdr->PIDs;
}

/*
static inline uint16_t 
pktgen_CoLoR_modify(struct rte_mbuf *m)
{
	int i;
	CoLoR_get_t * get_hdr=(CoLoR_get_t *)( rte_pktmbuf_mtod(m,unsigned char*)+sizeof(struct ether_hdr)+sizeof(ipHdr_t) );

	get_hdr->version_type=GET_PACKET;
	static int PIDS=0;
	//PIDS=(PIDS+1)%17;
	//This field means the number of PIDs!
	get_hdr->PIDs=PIDS;
	
	static int cycle=0;	
	cycle=cycle%8;
	for(i=0;i<16;i++){
		get_hdr->nid_sid[i]=0;
	}
	for(i=0;i<20;i++){
		get_hdr->l_sid[i]=0;
	}
	get_hdr->l_sid[19]=cycle;
	cycle++;
	return sizeof(struct ether_hdr)+sizeof(ipHdr_t)+sizeof(CoLoR_get_t)+4*get_hdr->PIDs;
	//The length of a pid is 4 byte! 
}
*/

static void
pkt_setup_GET(struct rte_mbuf *m) {
    int ret = 0;

    struct ether_hdr eth_hdr;
    ret = pktgen_ctor_ether_header(&eth_hdr, m);

    struct ipv4_hdr ipv4_hdr;
    ret += pktgen_ctor_ip_header(&ipv4_hdr, m, TYPE_GET);

    ret += pktgen_ctor_GET(m);

    m->nb_segs = 1;
    m->next = NULL;
    m->pkt_len = ret;
    m->data_len = ret;
}

void inline send_mbuf_GET(uint8_t portid, struct rte_mbuf *mbuf, unsigned lcore_id) {
    if (lcore_id < 0) {
        lcore_id = rte_lcore_id();
    }

    uint8_t socketid = rte_lcore_to_socket_id(lcore_id);
    struct lcore_conf *qconf = &lcore_conf[lcore_id];

    if (pktmbuf_pool[socketid] == NULL) {
        rte_exit(EXIT_FAILURE, "pktmbuf_pool[socketid]==NULL\n");
    } else {
        struct rte_mbuf *m = rte_pktmbuf_alloc(pktmbuf_pool[socketid]);

        if (m == NULL) {
            rte_exit(EXIT_FAILURE, "Allocate Failure\n");
        }

        pkt_setup_GET(m);

        send_single_packet(qconf, m, portid);
        rte_pktmbuf_free(m);
    }
}