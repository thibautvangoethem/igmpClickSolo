//#ifndef CLICK_IGMPIPENCAP_CC
//#define CLICK_IGMPIPENCAP_CC
#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IgmpIPEncap.hh"
CLICK_DECLS

routerAlert* IgmpIpEncap::addIP(WritablePacket* q,in_addr src,in_addr dst,uint32_t seq){
    click_ip *iph=(click_ip*) q->data();
    iph->ip_v=4;
    iph-> ip_hl =(sizeof(click_ip)+4)>>2;
    iph->ip_len=htons(q->length());
    uint16_t ip_id=((seq) % 0xFFFF)+1;
    iph->ip_id=htons(ip_id);
    iph->ip_p=2;
    iph->ip_ttl=1;
    iph->ip_tos=0xC0;
    iph->ip_src=src;
    iph->ip_dst=dst;
    
    routerAlert* alert = (routerAlert*)(iph+1);
    
    alert->part1=148;
    alert->part2=4;
    alert->octet2=0;
    alert->octet1=0;
    q->set_ip_header(iph, sizeof(click_ip));
    q->set_dst_ip_anno(dst);

    iph->ip_sum = click_in_cksum((unsigned char *) iph, sizeof(click_ip)+sizeof(routerAlert));
    return alert;
}

routerAlert* IgmpIpEncap::readIgmpIP(WritablePacket* q){
    click_ip* iph=(click_ip*)(q->data());
   
    int checksumBefore=iph->ip_sum;
    iph->ip_sum=0;
    routerAlert* alert = (routerAlert*)(iph+1);
    uint16_t checksum= click_in_cksum((unsigned char *) iph, sizeof(click_ip)+sizeof(routerAlert));
    if(checksumBefore!=checksum){
        click_chatter("checksum of ip incorrect");
        return 0;
    }
    readDst=iph->ip_dst;
    readSrc=iph->ip_src;
    iph->ip_sum=checksumBefore;
    return alert;
}

in_addr IgmpIpEncap::getReadDst(){
    return readDst;
}
in_addr IgmpIpEncap::getReadSrc(){
    return readSrc;
}

int IgmpIpEncap::getIgmpIPSize(){
    return sizeof(click_ip) +sizeof(routerAlert);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IgmpIpEncap)