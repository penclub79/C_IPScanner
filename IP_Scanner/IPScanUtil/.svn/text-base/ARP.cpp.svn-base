// ARP.cpp: implementation of the CARP class.
//
// Name: CARP
// Description:	CARP class, to read ARP table by using SNMP query
//				reading implementeate in function: GetTable
//				and Add/Modify/Remove ARP entry
//				Add/Modify/Remove implementeate in function: EditEntry
// Version: 1.0.0.0
// ?992-2008 Eng. Usama El-Mokadem: musama@hotmail.com.
//
// CONTACT INFORMATION:
// Eng. Usama El-Mokadem
// Email: musama@hotmail.com
// Web: http://musama.tripod.com
// Mobile: 0020 10 1289308
// Egypt
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "ARPTable.h"
#include "ARP.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CARP::CARP()
{
	// Load dynamic library: inetmib1.dll
	hMIBLibrary	= LoadLibrary(TEXT("inetmib1.dll"));

	// If library loaded, get addresses of (SnmpExtensionInit, pfnSnmpExtensionQuery) functions
	if (hMIBLibrary)
	{
		pfnSnmpExtensionInit	= (PFNSNMPEXTENSIONINIT)	GetProcAddress(hMIBLibrary, "SnmpExtensionInit");
		pfnSnmpExtensionQuery	= (PFNSNMPEXTENSIONQUERY)	GetProcAddress(hMIBLibrary, "SnmpExtensionQuery");

		// If success get addresses and initialize SNMP, bInitialized = true
		if (pfnSnmpExtensionInit && pfnSnmpExtensionQuery)
		{
			HANDLE				hPollForTrapEvent;
			AsnObjectIdentifier	aoiSupportedView;

			bInitialized		= pfnSnmpExtensionInit(0, &hPollForTrapEvent, &aoiSupportedView);
		}
	}
	else
	{
		// If fail to get addresses, bInitialized = false
		bInitialized			= FALSE;
		AfxMessageBox(_T("Load library fail"));
	}
}

CARP::~CARP()
{
	// If library loaded, free it
	if (hMIBLibrary)
		FreeLibrary(hMIBLibrary);
}

BOOL CARP::ParseIPAddress(CString IPAddr, BYTE& nField0, BYTE& nField1, BYTE& nField2, BYTE& nField3)
{
	BYTE*	pField;
	TCHAR	ch;
	int		p = 0;

	pField	= &nField0;
	*pField	= 0;
	for(int i=0; i<IPAddr.GetLength(); i++)
	{
		ch		= IPAddr.GetAt(i);
		if (ch == '.')
		{
			switch(++p)
			{
			case 1:		pField	= &nField1; break;
			case 2:		pField	= &nField2; break;
			case 3:		pField	= &nField3; break;
			}

			*pField	= 0;
			continue;
		}

		ch		-= '0';
		*pField	= *pField*10 + ch;
	}

	return TRUE;
}

//-----------------------------------------------------------------------
// Function:	GetEntries: Read ARP table for specific NIC interface.
//
// Parameters:
//	pTable	 		Pointer to array of arpTable struct
//	TableLength		Length of the array
//	AdapterIndex	NIC Adapter index number
//
// Returns:
//					Number of read ARP entries
//-----------------------------------------------------------------------
int CARP::GetEntries(arpTable* pTable, int TableLength, int AdapterIndex)
{
	// Be sure initialize SNMP true
	if (!bInitialized)
		return 0;

	SnmpVarBindList		SVBList[3];
	SnmpVarBind			SVBVars[3];
	UINT				OID[3][10];
	AsnInteger32		aiErrorStatus[3], aiErrorIndex[3];
	AsnObjectIdentifier	AsnOID0 = {sizeof(OID[0])/sizeof(UINT), OID[0]};
	AsnObjectIdentifier	AsnOID1 = {sizeof(OID[1])/sizeof(UINT), OID[1]};
	AsnObjectIdentifier	AsnOID2 = {sizeof(OID[2])/sizeof(UINT), OID[2]};
	unsigned long		pIPAddress;
	unsigned long		pMACAddress;
	int					iEntries;

	
	//-----------------------------------------------------------------------
	//	Fill array of 3 OIDs
	//	
	//	OID[0]	:	"1.3.6.1.2.1.4.22.1.1", ipNetToMediaIfIndex
	//				The interface on which this entry's equivalence is effective
	//
	//	OID[1]	:	"1.3.6.1.2.1.4.22.1.2", ipNetToMediaPhysAddress
	//				The media-dependent 'physical' address
	//
	//	OID[2]	:	"1.3.6.1.2.1.4.22.1.4", ipNetToMediaType
	//				Entry type: 1:Other, 2:Invalid(Remove), 3:Dynamic, 4:Static
	//
	for (int count=0; count<3; count++)
	{
		OID[count][0]		= 1;
		OID[count][1]		= 3;
		OID[count][2]		= 6;
		OID[count][3]		= 1;
		OID[count][4]		= 2;
		OID[count][5]		= 1;
		OID[count][6]		= 4;
		OID[count][7]		= 22;
		OID[count][8]		= 1;

		switch(count)
		{
		case 0:
			// Adapter interface
			OID[count][9]	= 1;
			break;

		case 1:
			// MAC address
			OID[count][9]	= 2;
			break;

		case 2:
			// Entry Type
			OID[count][9]	= 4;
			break;
		}
	}

	ZeroMemory(pTable, sizeof(arpTable)*TableLength);

	SVBList[0].len	= 1;
	SVBList[0].list	= &SVBVars[0];
	SnmpUtilOidCpy(&SVBVars[0].name, &AsnOID0);

	SVBList[1].len	= 1;
	SVBList[1].list	= &SVBVars[1];
	SnmpUtilOidCpy(&SVBVars[1].name, &AsnOID1);

	SVBList[2].len	= 1;
	SVBList[2].list	= &SVBVars[2];
	SnmpUtilOidCpy(&SVBVars[2].name, &AsnOID2);

	iEntries		= 0;
	do
	{
		aiErrorStatus[0]	= 0;
		aiErrorIndex[0]		= 0;
		aiErrorStatus[1]	= 0;
		aiErrorIndex[1]		= 0;
		aiErrorStatus[2]	= 0;
		aiErrorIndex[2]		= 0;

		// Query information of 3 OIDs
		if (pfnSnmpExtensionQuery(SNMP_PDU_GETNEXT, &SVBList[0], &aiErrorStatus[0], &aiErrorIndex[0]))
			if (pfnSnmpExtensionQuery(SNMP_PDU_GETNEXT, &SVBList[1], &aiErrorStatus[1], &aiErrorIndex[1]))
				if (pfnSnmpExtensionQuery(SNMP_PDU_GETNEXT, &SVBList[2], &aiErrorStatus[2], &aiErrorIndex[2]))
					if (aiErrorStatus[0] == SNMP_ERRORSTATUS_NOERROR &&
						aiErrorStatus[1] == SNMP_ERRORSTATUS_NOERROR &&
						aiErrorStatus[2] == SNMP_ERRORSTATUS_NOERROR) // Check for error
					{
						//-----------------------------------------------------------------------
						// From MSDN Help: http://msdn2.microsoft.com/en-us/library/aa378021.aspx
						// 
						// If the extension agent cannot resolve the variable bindings on a Get Next request, 
						// it must change the name field of the SnmpVarBind structure to the value of the object 
						// identifier immediately following that of the currently supported MIB subtree view. 
						// For example, if the extension agent supports view ".1.3.6.1.4.1.77.1", a Get Next 
						// request on ".1.3.6.1.4.1.77.1.5.1" would result in a modified name field of ".1.3.6.1.4.1.77.2". 
						// This signals the SNMP service to continue the attempt to resolve the variable bindings 
						// with other extension agents
						//-----------------------------------------------------------------------

						if(SnmpUtilOidNCmp(&SVBVars[0].name, &AsnOID0, AsnOID0.idLength)) 
							break;
						if(SnmpUtilOidNCmp(&SVBVars[1].name, &AsnOID1, AsnOID1.idLength)) 
							break;
						if(SnmpUtilOidNCmp(&SVBVars[2].name, &AsnOID2, AsnOID2.idLength)) 
							break;

						// Verify selected Adapter interface
						if (AdapterIndex == SVBList[0].list->value.asnValue.number)
						{
							// pIPAddress get pointer ro IP Address
							pIPAddress		= (unsigned long)SVBList[1].list->name.ids;
							pTable[iEntries].IPAddress[0]	= *(unsigned char *)(pIPAddress + 44);
							pTable[iEntries].IPAddress[1]	= *(unsigned char *)(pIPAddress + 48);
							pTable[iEntries].IPAddress[2]	= *(unsigned char *)(pIPAddress + 52);
							pTable[iEntries].IPAddress[3]	= *(unsigned char *)(pIPAddress + 56);

							// pIPAddress get pointer ro MAC Address
							pMACAddress		= (unsigned long)SVBList[1].list->value.asnValue.string.stream;
							if (pMACAddress)
							{
								pTable[iEntries].MACAddress[0]	= *(unsigned char *)(pMACAddress + 0);
								pTable[iEntries].MACAddress[1]	= *(unsigned char *)(pMACAddress + 1);
								pTable[iEntries].MACAddress[2]	= *(unsigned char *)(pMACAddress + 2);
								pTable[iEntries].MACAddress[3]	= *(unsigned char *)(pMACAddress + 3);
								pTable[iEntries].MACAddress[4]	= *(unsigned char *)(pMACAddress + 4);
								pTable[iEntries].MACAddress[5]	= *(unsigned char *)(pMACAddress + 5);
							}

							// Entry Type
							pTable[iEntries].Type	= (unsigned long)SVBList[2].list->value.asnValue.number;

							// Type must be one of (1, 2, 3, 4)
							if (pTable[iEntries].Type>=1 && pTable[iEntries].Type<=4)
								iEntries++;		// Move to next array position
						}
					}
					else
						break;	// If error exit do-while
	}
	while(iEntries<TableLength);

	// Frees the memory allocated for the specified object identifiers
	SnmpUtilOidFree(&SVBVars[2].name);
	SnmpUtilOidFree(&SVBVars[1].name);
	SnmpUtilOidFree(&SVBVars[0].name);

	return iEntries;	// Return number of Entries
}

//-----------------------------------------------------------------------
// Function:	EditEntry: Add/Modify/Remove ARP entry for specific NIC interface.
//
// Parameters:
//	IPAddress 		Array of 4 BYTES, 4 octs of IP Address
//	MACAddress		Array of 4 BYTES, 6 octs of MAC Address
//	Type			Entry type (2:Remove, 3:Dynamic, 4:Static)
//	AdapterIndex	NIC Adapter index number
//
// Returns:
//					TRUE if set successfully, FALSE otherwise.
//-----------------------------------------------------------------------
BOOL CARP::EditEntry(unsigned char IPAddress[4], unsigned char MACAddress[6], unsigned long Type, int AdapterIndex)
{
	if (!bInitialized)
		return 0;

	SnmpVarBindList		SVBList;
	SnmpVarBind			SVBVars[4];
	UINT				OID[4][10];
	AsnInteger32		aiErrorStatus, aiErrorIndex;
	BOOL				bReturn	= FALSE;

	//-----------------------------------------------------------------------
	//	Fill array of 4 OIDs
	//	
	//	OID[0]	:	"1.3.6.1.2.1.4.22.1.1", ipNetToMediaIfIndex
	//				The interface on which this entry's equivalence is effective
	//
	//	OID[1]	:	"1.3.6.1.2.1.4.22.1.2", ipNetToMediaPhysAddress
	//				The media-dependent 'physical' address
	//
	//	OID[2]	:	"1.3.6.1.2.1.4.22.1.3", ipNetToMediaNetAddress
	//				The IpAddress corresponding to the media-dependent 'physical' address
	//
	//	OID[3]	:	"1.3.6.1.2.1.4.22.1.4", ipNetToMediaType
	//				Entry type: 1:Other, 2:Invalid(Remove), 3:Dynamic, 4:Static
	//-----------------------------------------------------------------------
	for (int count=0; count<4; count++)
	{
		OID[count][0]		= 1;
		OID[count][1]		= 3;
		OID[count][2]		= 6;
		OID[count][3]		= 1;
		OID[count][4]		= 2;
		OID[count][5]		= 1;
		OID[count][6]		= 4;
		OID[count][7]		= 22;
		OID[count][8]		= 1;
		OID[count][9]		= 1 + count;

		switch(count)
		{
		case 0:
			//	OID[0]	:	"1.3.6.1.2.1.4.22.1.1", ipNetToMediaIfIndex
			//				The interface on which this entry's equivalence is effective
			SVBVars[count].value.asnType				= ASN_INTEGER;
			SVBVars[count].value.asnValue.number		= AdapterIndex;
			break;

		case 1:
			//	OID[1]	:	"1.3.6.1.2.1.4.22.1.2", ipNetToMediaPhysAddress
			//				The media-dependent 'physical' address
			SVBVars[count].value.asnType				= ASN_OCTETSTRING;
			SVBVars[count].value.asnValue.string.stream	= MACAddress;
			SVBVars[count].value.asnValue.string.length	= 6;	// MAC Address length
			SVBVars[count].value.asnValue.string.dynamic= FALSE;
			break;

		case 2:
			//	OID[2]	:	"1.3.6.1.2.1.4.22.1.3", ipNetToMediaNetAddress
			//				The IpAddress corresponding to the media-dependent 'physical' address
			SVBVars[count].value.asnType				= ASN_IPADDRESS;
			SVBVars[count].value.asnValue.string.stream	= IPAddress;
			SVBVars[count].value.asnValue.string.length	= 4;	// IP Address length
			SVBVars[count].value.asnValue.string.dynamic= FALSE;
			break;

		case 3:
			//	OID[3]	:	"1.3.6.1.2.1.4.22.1.4", ipNetToMediaType
			//				Entry type: 2:Remove, 3:Dynamic, 4:Static
			SVBVars[count].value.asnType				= ASN_INTEGER;
			SVBVars[count].value.asnValue.number		= Type;
			break;
		}

		AsnObjectIdentifier	AsnOID = {sizeof(OID[count])/sizeof(UINT), OID[count]};
		SnmpUtilOidCpy(&SVBVars[count].name, &AsnOID);
	}

	SVBList.len		= 4;
	SVBList.list	= SVBVars;

	aiErrorStatus	= 0;
	aiErrorIndex	= 0;

	// Set information of entry (4 OIDs)
	if( pfnSnmpExtensionQuery(SNMP_PDU_SET, &SVBList, &aiErrorStatus, &aiErrorIndex) )
	{
		if (aiErrorStatus == SNMP_ERRORSTATUS_NOERROR)
			bReturn = TRUE; // If success set bReturn = true
	}

	// Frees the memory allocated for the specified object identifiers
	SnmpUtilOidFree(&SVBVars[3].name);
	SnmpUtilOidFree(&SVBVars[2].name);
	SnmpUtilOidFree(&SVBVars[1].name);
	SnmpUtilOidFree(&SVBVars[0].name);

	return bReturn;		// TRUE if set successfully, FALSE otherwise.
}
