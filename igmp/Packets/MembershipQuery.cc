#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IgmpIPEncap.hh"
#include "MembershipQuery.hh"
#include "../packet_sources/HelperFunc.hh"

CLICK_DECLS
    void MembershipQuery::setType(int type){
        queryType = (uint8_t)type;
    }

    void MembershipQuery::setGroupAddr(uint32_t group){
        groupAdd=group;
    }
    void MembershipQuery::setMaxResp(int max){
        maxResp= max;
    }
    int MembershipQuery::getQRV(){
        return QRV;
    }

    int MembershipQuery::getQQIC(){
        return QQIC;
    }

    int MembershipQuery::getMaxResp(){
        return maxResp;

    }
    void MembershipQuery::setSFlag(bool flag){
        sFlag=flag;
    }
    void MembershipQuery::setQRV(int qrv){
        if (qrv > 7){
            QRV=0;
        } else {
            QRV=qrv;
        }
    }
    void MembershipQuery::setQQIC(int qqic){
        QQIC=qqic;
    }
    void MembershipQuery::addSource(uint32_t source){
        sources.push_back(source);
    }
    
    WritablePacket* MembershipQuery::addToPacket(WritablePacket* q,in_addr ipsrc,in_addr ipdst,int seq, bool correctChecksum){
        //todo: actually collect information of memberships
        IgmpIpEncap encapper;
        unsigned int numberOfSources = sources.size();
        int hsz = sizeof(click_ip) +sizeof(routerAlert) +sizeof(MembershipQueryMessage) + (sizeof(uint32_t)*numberOfSources);
        q = Packet::make(hsz);
        if (q==0) {
            click_chatter("unable to create packet");
            return 0;
        };
        memset(q->data(),'\0', hsz);


        routerAlert* alertEnd = encapper.addIP(q, ipsrc, ipdst, seq);
        
        MembershipQueryMessage* data = (MembershipQueryMessage*)(alertEnd+1);

        data->type = (uint8_t)(queryType);
        data->max_resp_code = HelperFunc::deduceCodeFromInt(maxResp);
        data->checksum = (uint16_t)(0);
        data->group_address = (uint32_t)(groupAdd);
        data->resv = 0;
        data->s = sFlag;
        data->qrv = QRV;
        data->qqic = HelperFunc::deduceCodeFromInt(QQIC);
        data->numberOfSources = htons((uint16_t)(numberOfSources));

        uint32_t sources[numberOfSources];
        for(int i = 0; i < numberOfSources; i++)
        {
            //sources[i] = (uint32_t)(0xC0A80202);//voorbeeldadres
            sources[i] = (uint32_t)(0);
        }

        uint32_t* startSources = (uint32_t*)(data+1);
        memcpy(startSources, sources, sizeof(uint32_t)*numberOfSources);

        unsigned int data_checksum_size = sizeof(MembershipQueryMessage)+(sizeof(uint32_t)*numberOfSources);
        data->checksum=click_in_cksum((const unsigned char *)data,data_checksum_size);
        if (!correctChecksum){
            data->checksum += 1;
        }

        return q;
    }

    //TODO make wrong checksum get rid of everything
    MembershipQuery MembershipQuery::readPacket(WritablePacket* q){
        IgmpIpEncap encapper;
        MembershipQuery membership;
        MembershipQueryMessage* data=(MembershipQueryMessage*)(encapper.readIgmpIP(q)+1);
        membership.setReadIpDst(encapper.getReadDst());
        membership.setReadIpSrc(encapper.getReadSrc());
        //I assume no source addresses are present in the query
        membership.setMaxResp(data->max_resp_code);
        membership.setGroupAddr(data->group_address);
        membership.setSFlag(data->s);
        membership.setQRV(data->qrv);
        membership.setQQIC(data->qqic);

        //checking the checksum
        unsigned int data_checksum_size = sizeof(MembershipQueryMessage)+(sizeof(uint32_t)*data->numberOfSources);
        if(click_in_cksum((const unsigned char *)data,data_checksum_size)){
            click_chatter("Checksum of MemberShipQuery was incorrect");
            return MembershipQuery();
        }
        
        return membership;
        
    }

    void MembershipQuery::setReadIpSrc(in_addr src){
        readIpsrc=src;
    }

    in_addr MembershipQuery::getReadIpSrc(){
        return readIpsrc;
    }

    void MembershipQuery::setReadIpDst(in_addr dst){
        readIpdst=dst;
    }

    in_addr MembershipQuery::getReadIpDst(){
        return readIpdst;
    }


CLICK_ENDDECLS
EXPORT_ELEMENT(MembershipQuery)

