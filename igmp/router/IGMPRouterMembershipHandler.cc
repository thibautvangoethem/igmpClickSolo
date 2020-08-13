#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <unistd.h>
#include "../packet_sources/HelperFunc.hh"
#include "../Packets/V3Membership.hh"
#include "../Packets/MembershipQuery.hh"
#include "IGMPRouterMembershipHandler.hh"

CLICK_DECLS
	InterfaceReceptionState::InterfaceReceptionState(IGMPRouterMembershipHandler* router) {
		owningRouter = router;
	}

	int InterfaceReceptionState::configure(Vector<String>&, ErrorHandler*) {
		return 0;
	}

	void InterfaceReceptionState::run_timer(Timer*) {
		
		//we change filtermode to include in correspondence with 6.5.
		filterMode = true;
		timerRuning=false;

		//however the previous was quite useless as we need to delete this element
		//there namely is never a source record specified as the list is always empty
		owningRouter->removeInterface(this);
	}

CLICK_ENDDECLS
EXPORT_ELEMENT(InterfaceReceptionState)


CLICK_DECLS
IGMPRouterMembershipHandler::IGMPRouterMembershipHandler(): queryIntervalTimer(this)
{
	robustnessVariable= 2;
	querierInterval = 125;
	queryResponseInterval = 100;
	startupQueryCount = robustnessVariable;
	//for values larger then 128 there is no guarantee that the exact value will be representable.
	lastMemberQueryInterval = 10; 
	lastMemberQueryCount = robustnessVariable;

	typeQueries = 0x11;
	multicastAddress.s_addr = 0;
	checksumValid = true;
}

IGMPRouterMembershipHandler::~ IGMPRouterMembershipHandler()
{
	for(Vector<InterfaceReceptionState*>::iterator interface = this->interfaces.begin(); interface != this->interfaces.end(); ){
		interfaces.erase(interface);
	}
}

in_addr IGMPRouterMembershipHandler::getn1(){
	return network1;
}
in_addr IGMPRouterMembershipHandler::getn2(){
	return network2;
}
in_addr IGMPRouterMembershipHandler::getserver(){
	return server;
}
in_addr IGMPRouterMembershipHandler::getdst(){
	return _dstIP;
}
int IGMPRouterMembershipHandler::getseq(){
	return _sequence;
}
void IGMPRouterMembershipHandler::incrseq(){
	_sequence++;
}

void IGMPRouterMembershipHandler::removeInterface(InterfaceReceptionState* interface){
	
	Vector<InterfaceReceptionState*>::iterator removedPosition = NULL;
	for(Vector<InterfaceReceptionState*>::iterator recordIndex = interfaces.begin(); recordIndex != interfaces.end(); recordIndex++){
		if(interface == (*recordIndex)){
			removedPosition = recordIndex;
		}
	}
	if(removedPosition != NULL){
		interfaces.erase(removedPosition);
	} else {
		click_chatter("No record was recognized by the router for removal.");
	}
}

int IGMPRouterMembershipHandler::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
	.read_mp("DST", _dstIP)
	.read_mp("server", server)
	.read_mp("N1", network1)
	.read_mp("N2", network2)
	.read("interval", querierInterval)
	.read("responseInterval",queryResponseInterval)
    .complete() < 0){ return -1;}
	

	queryIntervalTimer.initialize(this);


	//Setting timer first to startup query interval, after this the timer will just be query interval
	startUpQueryTimerData* data = new startUpQueryTimerData();
	data->me = this;
	data->count = startupQueryCount; 

	Timer* startupQueryTimer = new Timer(&IGMPRouterMembershipHandler::handleExpiryQT, data);
	startupQueryTimer->initialize(this);
	//Though it doesn't seem correct we can't call the sendquery in configure so we schedule it with a small time
	//If we were to not do this the first queries would be lost due to the router not finding the host it needs to send them to
	startupQueryTimer->schedule_after_msec(200);

	return 0;
}

void IGMPRouterMembershipHandler::sendGeneralQuery(){
	MembershipQuery generalQuery=makeGeneralQuery();
	sendQuery(1,generalQuery.addToPacket(0,getn1(),getdst(),getseq(), checksumValid));
	incrseq();
	sendQuery(2,generalQuery.addToPacket(0,getn2(),getdst(),getseq(), checksumValid));
	incrseq();
	sendQuery(0,generalQuery.addToPacket(0,getserver(),getdst(),getseq(), checksumValid));
	incrseq();

}

void IGMPRouterMembershipHandler::run_timer(Timer* t){
	// This timer isnt used anymore as it is also triggered by the expiring of the group membership timer
	// and removing this timer causes linking errors, so it will remain here, empty
}

void IGMPRouterMembershipHandler::handleExpiryQT(Timer* t, void * counter){
	startUpQueryTimerData* data = reinterpret_cast<startUpQueryTimerData *>(counter);

	data->me->sendGeneralQuery();
	
	if (data->count <= 1){
	// Normal general query interval
		Timer* newStartupQueryTimer = new Timer(&IGMPRouterMembershipHandler::handleExpiryQT, data);
		newStartupQueryTimer->initialize(data->me);
		newStartupQueryTimer->schedule_after_sec((int)( data->me->querierInterval));

	//We use the 1/4 querierinterval time for next general query as startup query interval 
	}else{
		//reset timer
		data->count--;
		Timer* newStartupQueryTimer = new Timer(&IGMPRouterMembershipHandler::handleExpiryQT, data);
		newStartupQueryTimer->initialize(data->me);
		newStartupQueryTimer->schedule_after_sec((int)((HelperFunc::deduceIntFromCode((uint8_t) data->me->querierInterval))/4));
	}

}

void IGMPRouterMembershipHandler::expireLMQT(InterfaceReceptionState* receptionState){
	Vector<lastMemberQueryTimerData*>::iterator toRemove = NULL;
	for(Vector<lastMemberQueryTimerData*>::iterator iter = lmqtDataVec.begin(); iter != lmqtDataVec.end(); iter++){
		if ((*iter)->receptionState == receptionState){
			toRemove= iter;
			break;
		}
	}
	delete (*toRemove)->receptionState;
	lmqtDataVec.erase(toRemove);
}

void IGMPRouterMembershipHandler::handleExpiryLastMemberQueryTimer(Timer * t, void * counter) {
	lastMemberQueryTimerData* data = reinterpret_cast<lastMemberQueryTimerData *>(counter);
	// this will send a query every lmqi as specified in rfc 6.4.2
	data->count--;
	if (data->count <= 0){
		// this is the last expire, now the lmqt is ended and interface should be removed
		if(data->final || data->alreadyQueried>data->me->lastMemberQueryInterval*100*data->me->lastMemberQueryCount){
			//though we never calculated the last member query time a simple rundown of the algorithm is enough to see that amount of time has now passed:
			//last member query count * last member query interval
			data->receptionState->filterMode = true;
			//it is not wrong to use a simple destructor on data and letting the receptionstate still exist because it gets referenced from the interfaces vector
			IGMPRouterMembershipHandler* mePtr = data->me; 
			mePtr->expireLMQT(data->receptionState);
		// query a new timer for the end of the lmqt
		}else{
			data->final=true;
			int toQuery= data->me->lastMemberQueryInterval*100*data->me->lastMemberQueryCount-data->alreadyQueried;
			data->alreadyQueried+=toQuery;
			Timer* newLastMemberQueryTimer = new Timer(&IGMPRouterMembershipHandler::handleExpiryLastMemberQueryTimer, data);
			newLastMemberQueryTimer->initialize(data->me);		
			newLastMemberQueryTimer->schedule_after_msec(toQuery);
		}
	}else{
		int alreadysend= data->me->lastMemberQueryCount-data->count;
		if(data->alreadyQueried<alreadysend*data->interval){
			data->count++;
			//query a timer till next specific query send
			int toQuery= alreadysend*data->interval-data->alreadyQueried;
			data->alreadyQueried=alreadysend*data->interval;
			Timer* newLastMemberQueryTimer = new Timer(&IGMPRouterMembershipHandler::handleExpiryLastMemberQueryTimer, data);
			newLastMemberQueryTimer->initialize(data->me);		
			newLastMemberQueryTimer->schedule_after_msec(toQuery);
		}else{
			//send query
			MembershipQuery change = data->me->makeGroupSpecificQuery(data->address.s_addr);
			change.setSFlag(!data->receptionState->recentelyGotJoin);
			data->receptionState->recentelyGotJoin=false;
			uint32_t temp=htonl(255);
			uint32_t srcint=data->srcInt;
			uint32_t test=temp|srcint;
			test-=htonl(1);
			in_addr broadcast=in_addr();
			broadcast.s_addr=test;					
			WritablePacket * changepacket=change.addToPacket(0,broadcast,data->address,0, data->me->checksumValid);

			data->me->output(data->receptionState->interface).push(changepacket);

			//reset timer
			// int interval=click_random(0,data->me->lastMemberQueryInterval*100);
			int interval=0;
			data->alreadyQueried+=interval;
			Timer* newLastMemberQueryTimer = new Timer(&IGMPRouterMembershipHandler::handleExpiryLastMemberQueryTimer, data);
			newLastMemberQueryTimer->initialize(data->me);		
			newLastMemberQueryTimer->schedule_after_msec(interval);
		}
	}

}

void IGMPRouterMembershipHandler::push(int port, Packet *p){
	//if the port is 0 it is a udp packet from the server and should be distributed correctly
	if(port==0){
		handleUDPPacket(port,p);
		return;
	}else{
		//if not it is an igmp message and you should update
		WritablePacket *q=p->uniqueify();
		click_ip* ip=(click_ip*)q->data();
		V3Membership mem=V3Membership::readPacket(q);
		if(mem.isChecksumCorrect()){
			handleMembershipReport(ip->ip_src,port,&mem);
		}
	}
}

void IGMPRouterMembershipHandler::handleUDPPacket(int interface,Packet* p){
	WritablePacket* q=p->uniqueify();
	click_ip* iphOfUdp=(click_ip*)q->data();
	igmp_udp* udp=(igmp_udp*)iphOfUdp+1;

	//check ip checksum
	int checksumBefore=iphOfUdp->ip_sum;
    uint16_t checksum= click_in_cksum((unsigned char *) iphOfUdp, sizeof(click_ip));
    if(checksum!=0){
        click_chatter("checksum of ip incorrect");
        return;
    }
    iphOfUdp->ip_sum=checksumBefore;
	

	//check udp checksum
	//as this calculation wasnt as the ip header, and was quite difficult to implement i used a click element in the router click script
	
	for(InterfaceReceptionState* i : interfaces){
		if(i->multicast==iphOfUdp->ip_dst&& !i->filterMode){
			Packet *q=p->clone();
			if(q){
				output(i->interface).push(q);	
			}
		}
		
	}
	p->kill();
	
}


void IGMPRouterMembershipHandler::sendQuery(int port, Packet *p){
    output(port).push(p);
}

MembershipQuery IGMPRouterMembershipHandler::makeGeneralQuery(){
	
	MembershipQuery query;

	query.setType(typeQueries);
	query.setGroupAddr(multicastAddress.s_addr); //normally this is 0.0.0.0
	query.setMaxResp(queryResponseInterval);
	query.setSFlag(false);
	query.setQRV(robustnessVariable);
	query.setQQIC(querierInterval);
	return query;
}

MembershipQuery IGMPRouterMembershipHandler::makeGroupSpecificQuery(uint32_t group) {
	
	MembershipQuery query;
	
	query.setType(typeQueries);
	query.setMaxResp(lastMemberQueryInterval);
	query.setSFlag(false);
	query.setGroupAddr(group);
	query.setQQIC(querierInterval);
	query.setQRV(robustnessVariable);

	return query;
}

int IGMPRouterMembershipHandler::sendQuery(const String &conf, Element *e,void * thunk, ErrorHandler * errh){
	IGMPRouterMembershipHandler * me = (IGMPRouterMembershipHandler *) e;
	MembershipQuery query= me->makeGeneralQuery();

	me->sendQuery(0,query.addToPacket(0,me->getserver(),me->getdst(),me->getseq(), me->checksumValid));
	me->incrseq();
	me->sendQuery(1,query.addToPacket(0,me->getn1(),me->getdst(),me->getseq(), me->checksumValid));
	me->incrseq();
	me->sendQuery(2,query.addToPacket(0,me->getn2(),me->getdst(),me->getseq(), me->checksumValid));
	me->incrseq();
	return 0;
} 

int IGMPRouterMembershipHandler::setRobustness(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	int newRobustness;

	if (Args(vconf, routerPtr, errh)
	.read_mp("ROBUSTNESS", newRobustness)
    .complete() < 0){ return -1;}
	click_chatter("Setting robustness to %d",newRobustness);

	routerPtr->robustnessVariable = newRobustness;
	//we also need to update the lastmemberquerycount to be up to date with the robustnessvariable
	routerPtr->lastMemberQueryCount = newRobustness;
	//same for startupquerycount, though it isn't really usefull
	routerPtr->startupQueryCount = newRobustness;
}

int IGMPRouterMembershipHandler::setQueryInterval(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	int newQueryInterval;

	if (Args(vconf, routerPtr, errh)
	.read_mp("QUERY_INTERVAL", newQueryInterval)
    .complete() < 0){ return -1;}
	click_chatter("Setting query interval to %d",newQueryInterval);
	routerPtr->querierInterval = newQueryInterval;
}

int IGMPRouterMembershipHandler::setMaxResponseTime(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	int newMaxResponseTime;

	if (Args(vconf, routerPtr, errh)
	.read_mp("MAX_RESPONSE_TIME", newMaxResponseTime)
    .complete() < 0){ return -1;}
	
	//we multiply by 10 because newMaxResponseTime is in deciseconds
	if (newMaxResponseTime > routerPtr->querierInterval*10){
		return -1;
	}
	click_chatter("Setting max response time to %d",newMaxResponseTime);
	routerPtr->queryResponseInterval = newMaxResponseTime;
}

int IGMPRouterMembershipHandler::setLastMemberQueryInterval(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	int newLastMemberQueryInterval;

	if (Args(vconf, routerPtr, errh)
	.read_mp("LAST_MEMBER_QUERY_INTERVAL", newLastMemberQueryInterval)
    .complete() < 0){ return -1;}
	click_chatter("Setting last member query interval to %d",newLastMemberQueryInterval);

	routerPtr->lastMemberQueryInterval = newLastMemberQueryInterval;
}

int IGMPRouterMembershipHandler::setLastMemberQueryCount(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	int newLastMemberQueryCount;

	if (Args(vconf, routerPtr, errh)
	.read_mp("LAST_MEMBER_QUERY_COUNT", newLastMemberQueryCount)
    .complete() < 0){ return -1;}
	click_chatter("Setting last member query count to %d",newLastMemberQueryCount);

	routerPtr->lastMemberQueryCount = newLastMemberQueryCount;
}

int IGMPRouterMembershipHandler::setIGMPType(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	int newIGMPType;

	if (Args(vconf, routerPtr, errh)
	.read_mp("QUERY", newIGMPType)
    .complete() < 0){ return -1;}
	click_chatter("Setting igmp type to %d",newIGMPType);

	routerPtr->typeQueries = newIGMPType;
}

int IGMPRouterMembershipHandler::setIGMPAddress(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	in_addr newIGMPaddress;

	if (Args(vconf, routerPtr, errh)
	.read_mp("QUERY", newIGMPaddress)
    .complete() < 0){ return -1;}
	click_chatter("Changing the igmp address for all queries");
	routerPtr->multicastAddress = newIGMPaddress;
}

int IGMPRouterMembershipHandler::setIGMPCheckSum(const String &conf, Element *e, void *thunk, ErrorHandler *errh){
	IGMPRouterMembershipHandler* routerPtr = reinterpret_cast<IGMPRouterMembershipHandler*>(e);
	Vector<String> vconf;
    cp_argvec(conf, vconf);

	bool validOrFalse;

	if (Args(vconf, routerPtr, errh)
	.read_mp("QUERY", validOrFalse)
    .complete() < 0){ return -1;}
	if(validOrFalse){
		click_chatter("The router will now start sending wrong checksums");
	}else{
		click_chatter("The router will now start sending correct checksums");
	}

	routerPtr->checksumValid = !validOrFalse; 
}

void IGMPRouterMembershipHandler::add_handlers(){
	add_write_handler("sendQuery", &sendQuery, (void *)0);
	add_write_handler("robustness", &setRobustness, (void *)0);
	add_write_handler("query_interval", &setQueryInterval, (void *)0);
	add_write_handler("max_response_time", &setMaxResponseTime, (void *)0);
	add_write_handler("last_member_query_interval", &setLastMemberQueryInterval, (void *)0);
	add_write_handler("last_member_query_count", &setLastMemberQueryCount, (void *)0);
	add_write_handler("set_igmp_type", &setIGMPType, (void *)0);
	add_write_handler("set_igmp_address", &setIGMPAddress,(void *)0);
	add_write_handler("invalid_igmp_checksum", &setIGMPCheckSum,(void *)0);
}


void IGMPRouterMembershipHandler::handleMembershipReport(in_addr src,int interface,V3Membership* mem){
	for(int recordNumber=0;recordNumber<mem->getRecords().size();recordNumber++){
		GroupRecord rec=mem->getRecords()[recordNumber];
		bool found=false;
		//check if the interface/multicast addres combo already existed
		Vector<InterfaceReceptionState *> toErase=Vector<InterfaceReceptionState *>();
		for(InterfaceReceptionState* i : interfaces){
			if(i->interface==interface&&i->multicast==rec.getMulticast()){
				found=true;
				//change to include mode
				if(rec.getRecordType()==3 &&!i->filterMode){
					//we check if there is already a lmqtData being used.
					bool exist = false;
					for(lastMemberQueryTimerData* iter:lmqtDataVec){
						if(iter->receptionState->interface == i->interface){
							exist = true;
							break;
						}
					}
					if(!exist){
						//we do not prune the group now because we first want to be sure there are no other interested members. See rfc 6.4.2 paragraph 4

						//send groupspecific query on this interface

						//this list is always empty
						i->sourceList = rec.getSources();
						//send change report
						MembershipQuery change = makeGroupSpecificQuery(rec.getMulticast().s_addr);
				
						//see here a pretty annoying way to set the last byte of the address to 254
						uint32_t temp=htonl(255);
						uint32_t srcint=src.s_addr;
						uint32_t test=temp|srcint;
						test-=htonl(1);
						in_addr broadcast=in_addr();
						broadcast.s_addr=test;					
						WritablePacket * changepacket=change.addToPacket(0,broadcast,rec.getMulticast(),0, checksumValid);

						output(interface).push(changepacket);

						//set group
						i->groupTimer->schedule_after_msec((uint32_t)(lastMemberQueryInterval*100));
						i->timerRuning=true;
						i->recentelyGotJoin=false;

						lastMemberQueryTimerData* lmqtData = new lastMemberQueryTimerData();
						lmqtData->me = this;
						lmqtData->count = lastMemberQueryCount;
						lmqtData->interval = 100*lastMemberQueryInterval;
						lmqtData->address = rec.getMulticast();
						lmqtData->receptionState = i;
						lmqtData->srcInt = src.s_addr;
						Timer* newLastMemberQueryTimer = new Timer(&IGMPRouterMembershipHandler::handleExpiryLastMemberQueryTimer, lmqtData);
						newLastMemberQueryTimer->initialize(lmqtData->me);
						// int interval=click_random(0,lmqtData->me->lastMemberQueryInterval*100);
						int interval=0;
						newLastMemberQueryTimer->schedule_after_msec(interval);
						lmqtDataVec.push_back(lmqtData);
						break;
					}
						
				}
				//change to exclude mode
				if(rec.getRecordType()==4 &&i->filterMode){
					i->filterMode=false;


					//this list is always empty
					i->sourceList =  rec.getSources();

					//here we use the groupMembeshipInterval but it gets calculated from other values
					//we multiply the robustness variable with deduced querier interval (variable querierInterval is actually QIC), the queryResponseInterval
					//is divided by 10 because the variable queryResponseInterval is in deciseconds and we want to calculate a value that is in seconds.
					int groupMembershipInterval = ((robustnessVariable) * HelperFunc::deduceIntFromCode(querierInterval)) + (queryResponseInterval/10);
					i->groupTimer->schedule_after_sec(groupMembershipInterval);

					break;

				}
				//type is already include or exclude so these are the Current-State Records
				if(rec.getRecordType()==1||rec.getRecordType()==2){
					//this list is always empty
					i->sourceList = rec.getSources();
					i->recentelyGotJoin=true;

					//quite an ugly method to see if we are on a broadcast address as src
					uint32_t srcnormal=htonl(src.s_addr);
					uint32_t final=srcnormal%256;
        
					//if final is 1 or 2 then we have 254 or 255at the end
					if(final==1||final==2){
						// here you might think, why do I create this new interface and delete the old one and not just set the filtermode to false
						// Well the reason is because c++,click or I messed up te memory of this interfacestate somehow
						// I just spent 9 hours trying to find this bug, which manifest itself with the interfacestate memory being overwritten when a packet gets cloned later on in the handleudppacket function
						// Which causes this element to crash when it tries to send data to port 26000+ on the next udp packet
						// Again I don't know why this happens, the interface memory gets deleted nowhere and the memory locations of this interfacestate (or the cloned packet in fact) don't get changed
						// and aren't even that close to each other to be able to overwrite each other.
						// So anyway the only fix I have found is to recreate the interfacestate object here in a new memorylocation and delete the old one.
						InterfaceReceptionState* test= new InterfaceReceptionState();
						test->multicast=i->multicast;
						test->interface=i->interface;
						test->owningRouter=i->owningRouter;
						test->groupTimer=new Timer(this);
						test->groupTimer->initialize(this);
						test->filterMode=false;
						test->recentelyGotJoin=i->recentelyGotJoin;
						Vector<in_addr> temp=Vector<in_addr>();
						for(int j=0;j<temp.size();j++){
							temp.push_back(i->sourceList[j]);
						}
						test->sourceList=temp;
						interfaces.push_back(test);
						toErase.push_back(i);						
						
					}

					break;
				}
				//anything else must not be made
			
			}
		}
		for(int i=toErase.size()-1;i>=0;i--){
			removeInterface(toErase[i]);
		}
		// checking validity of the remaining interface (no duplicates this happens due to unknown reason)
		Vector<int> toremove=Vector<int>();
		for(int interface1=0;interface1<interfaces.size();interface1++){
			for(int interface2=0;interface2<interfaces.size();interface2++){
				if(interface1!=interface2){
					if(interfaces[interface1]->interface==interfaces[interface2]->interface){
						if(!interfaces[interface1]->filterMode && !interfaces[interface2]->filterMode ){
							bool alreadyToRemove=false;
							for(int i:toremove){
								if(i==interface1|| i==interface2){
									alreadyToRemove=true;
								}
							}
							if(!alreadyToRemove){
								toremove.push_front(interface1);
							}
						}
					}
				}
			}
		}
		for(int remove:toremove){
			removeInterface(interfaces[remove]);
		}
		//it didnt exist yet in the list so we need to make a new receptionState
		if(!found){
				InterfaceReceptionState* state = new InterfaceReceptionState(this);
				state->interface=interface;
				state->groupTimer=new Timer(this);
				state->groupTimer->initialize(this);
				if(rec.getRecordType()==4){
					state->filterMode=false;
					//if it is include and no sources are given then don't add anything
				}
				if(rec.getRecordType()==3){
					state->filterMode=true;
				}

				state->sourceList = rec.getSources();
				state->multicast=rec.getMulticast();
				interfaces.push_back(state);
		}
	}

}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPRouterMembershipHandler)
