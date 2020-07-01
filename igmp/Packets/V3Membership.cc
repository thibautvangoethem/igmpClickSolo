#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IgmpIPEncap.hh"
#include "V3Membership.hh"
CLICK_DECLS
    GrouprecordAddressStruct* GroupRecord::addToPacket(GroupRecordStruct* p){

        if (p==0) {
            click_chatter("unable to add group record");
        };
        GroupRecordStruct* gr=p;

        gr->recordType=recordType;
        gr->auxDataLen=0;
        gr->numberSources=htons(sources.size());
        gr->multicast=multicast;
        GrouprecordAddressStruct* add=(GrouprecordAddressStruct*)(gr+1);
        for(Vector<in_addr>::iterator j =sources.begin(); j!= sources.end(); j++){
           add->address=*j;
           if(j!= sources.end()-1){   
                add=(GrouprecordAddressStruct*)(add+1);
            }
        }
        return add-1;  
    };

    int GroupRecord::getSize(){
        /**int temp=8+sizeof(in_addr)*(sources.size());
        char char_arr [100];
        sprintf(char_arr, "%d", temp);
        click_chatter(char_arr);
        **/
        return 8+sizeof(in_addr)*(sources.size());
    }

    V3Membership::V3Membership(const V3Membership& other){
        grouprecords = other.grouprecords;
    }

	in_addr GroupRecord::getMulticast(){
        return multicast;
    }
    Vector<in_addr> GroupRecord::getSources(){
        return sources;
    }
    int GroupRecord::getRecordType(){
        return recordType;
    }
    bool V3Membership::isChecksumCorrect(){
        return checksumValid;
    }
    void V3Membership::setChecksumCorrect(bool correct){
        checksumValid=correct;
    }

    void V3Membership::setGroupRecords(Vector<GroupRecord> records){
        this->grouprecords=records;
    }

	void V3Membership::addGroupRecord(in_addr multicast, Vector<in_addr> sources,int recordType){
        this->grouprecords.push_back(GroupRecord{multicast,sources,recordType});
    }
    Vector<GroupRecord> V3Membership::getRecords(){
        return grouprecords;
    }
    WritablePacket* V3Membership::addToPacket(WritablePacket* q,in_addr ipsrc,in_addr ipdst,int seq,int igmpType){
        IgmpIpEncap encapper;
        int hsz=0;
        hsz+= encapper.getIgmpIPSize();
        hsz+=sizeof(V3MembershipReportPackage);
        for(Vector<GroupRecord>::iterator i =grouprecords.begin(); i!= grouprecords.end(); i++){
            hsz+=i->getSize();
        }
        q = Packet::make(hsz);
        memset(q->data(),'\0', hsz);
        if (q==0) {
            click_chatter("unable to create packet");
            return 0;
        };
        V3MembershipReportPackage* data=(V3MembershipReportPackage*)(encapper.addIP(q,ipsrc,ipdst,seq)+1);
        data->type=igmpType;
        data->reserved1=0;
        data->reserved2=0;
        data->groupRecordNumber=htons(grouprecords.size());
        data->checksum=0;
        GroupRecordStruct* gs=(GroupRecordStruct*)(data+1);
        for(Vector<GroupRecord>::iterator i =grouprecords.begin(); i!= grouprecords.end(); i++){
            gs=(GroupRecordStruct*)i->addToPacket(gs);
            if(i!=grouprecords.end()){
                gs=gs+1;
            }
        }
       

        if(checksumValid){
            data->checksum=click_in_cksum((const unsigned char *)data,hsz-encapper.getIgmpIPSize());
        }else{
            data->checksum=0;
        }

        return q;

    }

    //TODO check checksum
    V3Membership V3Membership::readPacket(WritablePacket* q){
        IgmpIpEncap encapper;
        V3Membership membership;
        V3MembershipReportPackage* data=(V3MembershipReportPackage*)(encapper.readIgmpIP(q)+1);
        
        if(htons(data->groupRecordNumber)>0){
            GroupRecordStruct* gs=(GroupRecordStruct*)(data+1);
            for(int i=0;i<htons(data->groupRecordNumber);i++){
                in_addr multicast=gs->multicast;
                int recordType=gs->recordType;
                Vector<in_addr> sources=Vector<in_addr>();
                GrouprecordAddressStruct* add=(GrouprecordAddressStruct*)gs;
                if(htons(gs->numberSources)!=0){
                    add=add+1;
                    for(int j=0;j<htons(gs->numberSources);j++){
                        sources.push_back(add->address);
                        if(j!=htons(gs->numberSources)){
                            add=add+1;
                        }
                    }
                }
                membership.addGroupRecord(multicast,sources,recordType);
                if(i!=htons(data->groupRecordNumber)-1){
                    gs=(GroupRecordStruct*)(add+1);
                }
            }
        }
        //checking checksum
        int hsz=0;
        hsz+= encapper.getIgmpIPSize();
        hsz+=sizeof(V3MembershipReportPackage);
        for(Vector<GroupRecord>::iterator i =membership.getRecords().begin(); i!= membership.getRecords().end(); i++){
            hsz+=i->getSize();
        }
        if(click_in_cksum((const unsigned char *)data,hsz-encapper.getIgmpIPSize())){
            click_chatter("Checksum of V3 membership query was incorrect");
            return V3Membership();
        }
        
        return membership;
    }
CLICK_ENDDECLS
EXPORT_ELEMENT(V3Membership)
