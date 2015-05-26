/*
*	asyncquery.cpp
*	
*	script performs SNMPWALK of a target device, which feeds the results into a
*	ZMQ Worker script ( query_processors.cpp ) who places results in both
*	local database ( tcp://127.0.0.1:3306 )and 
*	remote database( tcp://10.0.13.47:3306 ). 
*
*	%> gcc -o ../bin/asyncquery asyncquery.c -l netsnmp
*
*/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* HOST LIST - Should get this from database */
struct host { 
	char *name;
	char *community;
	} hosts[] = { 
		{"10.0.13.47",  "public" },
//		{ }, 
		{ NULL }
	};

/* List of Variables to query for */ 
struct oid {
	char *Name;
	oid Oid[MAX_OID_LEN];
	int OidLen;
	} oids[] = {
		{ "sysDescr.0" },
		{ "sysLocation.0" }, 
		{ "interfaces.ifNumber.1" },
		{ "interfaces.ifNumber.0" },
		{ NULL } 
	}; 


void synchronous (void)
{
	struct host *hp;
	
	for ( hp = hosts; hp->name; hp++ ) {
		struct snmp_session ss, *sp;
		struct oid *op;

		/* initialise session */ 
		snmp_sess_init(&ss);
		ss.version = SNMP_VERSION_2c;
		ss.peername = hp->name;
		ss.community = hp->community;
		ss.community_len = strlen(ss.community);
		snmp_synch_setup(&ss);

		if ( ! ( sp = snmp_open(&ss))){
			snmp_perror("snmp_open");
			continue;
		}

		for ( op = oids; op->Name; op++ ) {
			struct snmp_pdu *req, *resp;
			int status;
			req = snmp_pdu_create(SNMP_MSG_GET);
			snmp_add_null_var(req, op->Oid, op->OidLen);
			status = snmp_synch_response(sp, req, &resp);
			if (!print_result(status, sp, resp)) break;
			snmp_free_pdu(resp);
		}
		snmp_close(sp);
	}
}


/* define a struct session to hold our per host state */
struct session { 
	struct snmp_session *sess; 
	struct oid *current_oid;
	} sessions[sizeof(hosts)/sizeof(hosts[0])];
	
	int active_hosts; 

/* Callback function to fire whenever we get a response */
int asynch_response(int operation, struct snmp_session *sp, int reqid,
						struct snmp_pdu *pdu, void *magic)
{
	struct session* host = (struct session*)magic; 
	struct snmp_pdu *req; 

	/* upon recieving a response, we will print it out and send next req  */
	if ( operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE ) {
		if ( print_result( STAT_SUCCESS, host->sess, pdu)) {
			host->current_oid++;
			if ( host->current_oid->Name) { 

				/* build next request */ 
				req = snmp_pdu_create(SNMP_MSG_GET);
				snmp_add_null_var(req, host->current_oid->Oid, host->current_oid->OidLen);
				if (snmp_send( host->sess, req )) /* everything went well */
					return 1;
				/* Something went wrong.. */				
				else {
					snmp_perror("snmp_send");
					snmp_free_pdu(req);
					}
			}
		}
	}
	else 
		print_result(STAT_TIMEOUT, host->sess, pdu);
		active_hosts--;
		return 1; 
}

/* Code to fireup async_respose */ 
void asynchronous(void)
{
	struct session *hs;
	struct host *hp;

	/* for each host, open a session and send out first request */
	for ( hs = sessions, hp = hosts; hp->name; hs++, hp++ ) { 
		struct snmp_pdu *req;
		struct snmp_session sess;
		snmp_sess_init(&sess);
		sess.version = SNMP_VERSION_2c;
		sess.peername = hp->name; 
		sess.community = hp->community;
		sess.community_len = strlen(sess.community);
		
		sess.callback = asynch_response;
		sess.callback_magic = hs; 
		if ( ! (hs->sess = snmp_open(&sess))) {
			snmp_perror("snmp_open");
			continue;
		} 
		hs->current_oid = oids;
		req = snmp_pdu_create(SNMP_MSG_GET);
		snmp_add_null_var(req, hs->current_oid->Oid, hs->current_oid->OidLen);
		if (snmp_send(hs->sess, req))
			active_hosts++;
		else {
			snmp_perror("snmp_send");
			snmp_free_pdu(req);
		}
	}

	/* loop while any active hosts */ 
	while (active_hosts) {
		int fds = 0, block = 1;	
		fd_set fdset;
		struct timeval timeout;
		
		FD_ZERO(&fdset);
		snmp_select_info(&fds, &fdset, &timeout, &block);
		fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
		
		/* Read all sockets and process them */ 
		if (fds ) snmp_read(&fdset); 
		else snmp_timeout();
	}

	/* All active hosts processed, close sessions and finish up */ 
	for (hp = hosts, hs = sessions; hp->name; hs++, hp++){
		if (hs->sess) snmp_close(hs->sess);
	}
}

int main( int argc, char **argv)
{
	initialize();
		
	printf("-------synchronous--------\n");
	synchronous(); 
	
	printf("-------asynchronous--------\n");
	asynchronous();
 
	return 0;

}










		
	













				
	


