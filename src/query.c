/*
*	query.cpp
*	
*	script performs SNMPWALK of a target device, which feeds the results into a
*	ZMQ Worker script ( query_processors.cpp ) who places results in both
*	local database ( tcp://127.0.0.1:3306 )and 
*	remote database( tcp://10.0.13.47:3306 ). 
*
*	%> gcc -o ../bin/query query.c -l netsnmp
*
*/


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#undef DEMO_USE_SNMP_VERSION_3 // <-- Change to #define to use SNMPv3 and uncomment endif
#include "net-snmp/library/transform_oids.h"
const char *v3_pass = "demo password";
//#endif      


int main(){

	/* define struct variables */
	struct snmp_session session, *ss;
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_LEN = MAX_OID_LEN; 

	struct variable_list *vars;
	int status;
 
	/* Initialise SNMP library */
	init_snmp("snmpapp");

	/* Initialise Session and target host */
	snmp_sess_init(&session);
	session.peername = "10.0.13.47";

	/* If v3 is required, check Simple_Application */

	/* Version 1 code */ 
	session.version = SNMP_VERSION_1; 
	
	/* Set community name target host(s) use for authentication */
	session.community = "public";
	session.community_len = strlen(session.community);

	/* Open newly established session */ 
	ss = snmp_open(&session); 

	/* If session fails print an error message */ 
	if ( !ss ) {
		snmp_perror("ack");
		snmp_log(LOG_ERR, "something horrible happened!!!\n"); 
		exit(2); 
	}

	/* Create PDU to hold request data */ 
	pdu = snmp_pdu_create(SNMP_MSG_GET); 
	
	/* Fill PDU with requested OID */ 
	get_node("sysDescr.0", anOID, &anOID_LEN);

	/* Add null to PDU (requirement for gets) */ 
	snmp_add_null_var(pdu, anOID, anOID_LEN);

	/* Send request! */
	status = snmp_synch_response(ss, pdu, &response); 

/****************************
*
*	Begin to examine responses
*
****************************/

	/* Process responses */ 
	if ( status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR ){ 

		/* Success: Print the variables */ 
		for( vars = response->variables; vars; vars = vars->next_variable )
			print_variable( vars->name, vars->name_length, vars );

		/* Begin manipulation of information */
		for( vars = response->variables; vars; vars = vars->next_variable ){
			int count=1;
		
			/* var is string! horray! */
			if( vars->type == ASN_OCTET_STR ) { 
				char *sp = malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
	
			}
			else  
				printf("value #%d is NOT a string! Ack!\n", count++);
			}
	/* Process doesn't respond */
	} else { 
		if ( status == STAT_SUCCESS )
			fprintf(stderr, "Error in packet\nReason:%s\n",
					snmp_errstring(response->errstat));
		else
			snmp_sess_perror("snmpget", ss);
		}
		
/****************************
*
*	Finish examining responses
*
****************************/	

		/* clean up response */ 
		if ( response )
			snmp_free_pdu(response);
		snmp_close(ss);


	return 0;
}
